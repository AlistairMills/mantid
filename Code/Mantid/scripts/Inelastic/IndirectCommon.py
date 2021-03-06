from mantid.simpleapi import *
from mantid import config, logger
from IndirectImport import import_mantidplot
import sys, os.path, math, datetime, re

def StartTime(prog):
    logger.notice('----------')
    message = 'Program ' + prog +' started @ ' + str(datetime.datetime.now())
    logger.notice(message)

def EndTime(prog):
    message = 'Program ' + prog +' ended @ ' + str(datetime.datetime.now())
    logger.notice(message)
    logger.notice('----------')

def loadInst(instrument):    
    ws = '__empty_' + instrument
    if not mtd.doesExist(ws):
        idf_dir = config['instrumentDefinition.directory']
        idf = idf_dir + instrument + '_Definition.xml'
        LoadEmptyInstrument(Filename=idf, OutputWorkspace=ws)

def loadNexus(filename):
    '''
    Loads a Nexus file into a workspace with the name based on the
    filename. Convenience function for not having to play around with paths
    in every function.
    '''
    name = os.path.splitext( os.path.split(filename)[1] )[0]
    LoadNexus(Filename=filename, OutputWorkspace=name)
    return name

def getInstrRun(ws_name):
    '''
    Get the instrument name and run number from a workspace.

    @param ws_name - name of the workspace
    @return tuple of form (instrument, run number) 
    '''
    ws = mtd[ws_name]
    run_number = str(ws.getRunNumber())
    if run_number == '0':
        #attempt to parse run number off of name
        match = re.match('([a-zA-Z]+)([0-9]+)', ws_name)
        if match:
            run_number = match.group(2)
        else:
            raise RuntimeError("Could not find run number associated with workspace.")

    instrument = ws.getInstrument().getName()
    facility = config.getFacility()
    instrument = facility.instrument(instrument).filePrefix(int(run_number))
    instrument = instrument.lower()
    return instrument, run_number

def getWSprefix(wsname):
    '''
    Returns a string of the form '<ins><run>_<analyser><refl>_' on which
    all of our other naming conventions are built. The workspace is used to get the
    instrument parameters.
    '''
    if wsname == '':
        return ''

    ws = mtd[wsname]
    facility = config['default.facility']

    ws_run = ws.getRun()
    if 'facility' in ws_run:
        facility = ws_run.getLogData('facility').value

    (instrument, run_number) = getInstrRun(wsname)
    if facility == 'ILL':
        run_name = instrument + '_'+ run_number
    else:
        run_name = instrument + run_number

    try:
        analyser = ws.getInstrument().getStringParameter('analyser')[0]
        reflection = ws.getInstrument().getStringParameter('reflection')[0]
    except IndexError:
        analyser = ''
        reflection = ''

    prefix = run_name + '_' + analyser + reflection + '_'
    return prefix

def getEfixed(workspace, detIndex=0):
    inst = mtd[workspace].getInstrument()
    return inst.getNumberParameter("efixed-val")[0]

# Get the default save directory and check it's valid
def getDefaultWorkingDirectory():
    workdir = config['defaultsave.directory']
    
    if not os.path.isdir(workdir):
        raise IOError("Default save directory is not a valid path!")

    return workdir

def getRunTitle(workspace):
    ws = mtd[workspace]
    title = ws.getRun()['run_title'].value.strip()
    runNo = ws.getRun()['run_number'].value
    inst = ws.getInstrument().getName()
    ins = config.getFacility().instrument(ins).shortName().lower()
    valid = "-_.() %s%s" % (string.ascii_letters, string.digits)
    title = ''.join(ch for ch in title if ch in valid)
    title = ins + runNo + '-' + title
    return title

def createQaxis(inputWS):
    result = []
    ws = mtd[inputWS]
    nHist = ws.getNumberHistograms()
    if ws.getAxis(1).isSpectra():
        inst = ws.getInstrument()
        samplePos = inst.getSample().getPos()
        beamPos = samplePos - inst.getSource().getPos()
        for i in range(0,nHist):
            efixed = getEfixed(inputWS, i)
            detector = ws.getDetector(i)
            theta = detector.getTwoTheta(samplePos, beamPos) / 2
            lamda = math.sqrt(81.787/efixed)
            q = 4 * math.pi * math.sin(theta) / lamda
            result.append(q)
    else:
        axis = ws.getAxis(1)
        msg = 'Creating Axis based on Detector Q value: '
        if not axis.isNumeric():
            msg += 'Input workspace must have either spectra or numeric axis.'
            raise ValueError(msg)
        if ( axis.getUnit().unitID() != 'MomentumTransfer' ):
            msg += 'Input must have axis values of Q'
            raise ValueError(msg)
        for i in range(0, nHist):
            result.append(float(axis.label(i)))
    return result

def GetWSangles(inWS,verbose=False):
    nhist = mtd[inWS].getNumberHistograms()						# get no. of histograms/groups
    sourcePos = mtd[inWS].getInstrument().getSource().getPos()
    samplePos = mtd[inWS].getInstrument().getSample().getPos() 
    beamPos = samplePos - sourcePos
    angles = []										# will be list of angles
    for index in range(0, nhist):
        detector = mtd[inWS].getDetector(index)					# get index
        twoTheta = detector.getTwoTheta(samplePos, beamPos)*180.0/math.pi		# calc angle
        angles.append(twoTheta)						# add angle
    return angles

def GetThetaQ(inWS):
    nhist = mtd[inWS].getNumberHistograms()						# get no. of histograms/groups
    efixed = getEfixed(inWS)
    wavelas = math.sqrt(81.787/efixed)					   # elastic wavelength
    k0 = 4.0*math.pi/wavelas
    d2r = math.pi/180.0
    sourcePos = mtd[inWS].getInstrument().getSource().getPos()
    samplePos = mtd[inWS].getInstrument().getSample().getPos() 
    beamPos = samplePos - sourcePos
    theta = []
    Q = []
    for index in range(0,nhist):
        detector = mtd[inWS].getDetector(index)					# get index
        twoTheta = detector.getTwoTheta(samplePos, beamPos)*180.0/math.pi		# calc angle
        theta.append(twoTheta)						# add angle
        Q.append(k0*math.sin(0.5*twoTheta*d2r))
    return theta,Q

def ExtractFloat(a):                              #extract values from line of ascii
    extracted = []
    elements = a.split()							#split line on spaces
    for n in elements:
        extracted.append(float(n))
    return extracted                                 #values as list

def ExtractInt(a):                              #extract values from line of ascii
    extracted = []
    elements = a.split()							#split line on spaces
    for n in elements:
        extracted.append(int(n))
    return extracted                                 #values as list

def PadArray(inarray,nfixed):                   #pad a list to specified size
	npt=len(inarray)
	padding = nfixed-npt
	outarray=[]
	outarray.extend(inarray)
	outarray +=[0]*padding
	return outarray

def CheckAnalysers(in1WS,in2WS,Verbose):
    ws1 = mtd[in1WS]
    a1 = ws1.getInstrument().getStringParameter('analyser')[0]
    r1 = ws1.getInstrument().getStringParameter('reflection')[0]
    ws2 = mtd[in2WS]
    a2 = ws2.getInstrument().getStringParameter('analyser')[0]
    r2 = ws2.getInstrument().getStringParameter('reflection')[0]
    if a1 != a2:
        raise ValueError('Workspace '+in1WS+' and '+in2WS+' have different analysers')
    elif r1 != r2:
        raise ValueError('Workspace '+in1WS+' and '+in2WS+' have different reflections')
    else:
        if Verbose:
            logger.notice('Analyser is '+a1+r1)

def CheckHistZero(inWS):
    nhist = mtd[inWS].getNumberHistograms()       # no. of hist/groups in WS
    if nhist == 0:
        raise ValueError('Workspace '+inWS+' has NO histograms')
    Xin = mtd[inWS].readX(0)
    ntc = len(Xin)-1						# no. points from length of x array
    if ntc == 0:
        raise ValueError('Workspace '+inWS+' has NO points')
    return nhist,ntc

def CheckHistSame(in1WS,name1,in2WS,name2):
    nhist1 = mtd[in1WS].getNumberHistograms()       # no. of hist/groups in WS1
    X1 = mtd[in1WS].readX(0)
    xlen1 = len(X1)
    nhist2 = mtd[in2WS].getNumberHistograms()       # no. of hist/groups in WS2
    X2 = mtd[in2WS].readX(0)
    xlen2 = len(X2)
    if nhist1 != nhist2:				# check that no. groups are the same
        e1 = name1+' ('+in1WS+') histograms (' +str(nhist1) + ')'
        e2 = name2+' ('+in2WS+') histograms (' +str(nhist2) + ')'
        error = e1 + ' not = ' + e2
        raise ValueError(error)
    elif xlen1 != xlen2:
        e1 = name1+' ('+in1WS+') array length (' +str(xlen1) + ')'
        e2 = name2+' ('+in2WS+') array length (' +str(xlen2) + ')'
        error = e1 + ' not = ' + e2
        raise ValueError(error)

def CheckXrange(x_range,type):
    if  not ( ( len(x_range) == 2 ) or ( len(x_range) == 4 ) ):
        raise ValueError(type + ' - Range must contain either 2 or 4 numbers')
    
    for lower, upper in zip(x_range[::2], x_range[1::2]):
        if math.fabs(lower) < 1e-5:
            raise ValueError(type + ' - input minimum ('+str(lower)+') is Zero')
        if math.fabs(upper) < 1e-5:
            raise ValueError(type + ' - input maximum ('+str(upper)+') is Zero')
        if upper < lower:
            raise ValueError(type + ' - input max ('+str(upper)+') < min ('+lower+')')

def CheckElimits(erange,Xin):
    nx = len(Xin)-1
    
    if math.fabs(erange[0]) < 1e-5:
        raise ValueError('Elimits - input emin ( '+str(erange[0])+' ) is Zero')
    if erange[0] < Xin[0]:
        raise ValueError('Elimits - input emin ( '+str(erange[0])+' ) < data emin ( '+str(Xin[0])+' )')
    if math.fabs(erange[1]) < 1e-5:
        raise ValueError('Elimits - input emax ( '+str(erange[1])+' ) is Zero')
    if erange[1] > Xin[nx]:
        raise ValueError('Elimits - input emax ( '+str(erange[1])+' ) > data emax ( '+str(Xin[nx])+' )')
    if erange[1] < erange[0]:
        raise ValueError('Elimits - input emax ( '+str(erange[1])+' ) < emin ( '+erange[0]+' )')

def plotSpectra(ws, y_axis_title, indicies=[]):
    """
    Plot a selection of spectra given a list of indicies

    @param ws - the workspace to plot
    @param y_axis_title - label for the y axis
    @param indicies - list of spectrum indicies to plot
    """
    if len(indicies) == 0:
        num_spectra = mtd[ws].getNumberHistograms()
        indicies = range(num_spectra)

    try:
        mp = import_mantidplot()
        plot = mp.plotSpectrum(ws, indicies, True)
        layer = plot.activeLayer()
        layer.setAxisTitle(mp.Layer.Left, y_axis_title)
    except RuntimeError:
        #User clicked cancel on plot so don't do anything
        return

def plotParameters(ws, *param_names):
    """
    Plot a number of spectra given a list of parameter names
    This searchs for relevent spectra using the text axis label.

    @param ws - the workspace to plot from
    @param param_names - list of names to search for
    """
    axis = mtd[ws].getAxis(1)
    if axis.isText() and len(param_names) > 0:
        num_spectra = mtd[ws].getNumberHistograms()

        for name in param_names:
            indicies = [i for i in range(num_spectra) if name in axis.label(i)]
            if len(indicies) > 0:
                plotSpectra(ws, name, indicies)
