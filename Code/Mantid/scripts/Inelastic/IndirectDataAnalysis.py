from IndirectImport import import_mantidplot
mp = import_mantidplot()
from IndirectCommon import *

import math, re, os.path, numpy as np
import itertools

from mantid.simpleapi import *
from mantid.api import TextAxis
from mantid import *

##############################################################################
# Misc. Helper Functions
##############################################################################

def split(l, n):
    #Yield successive n-sized chunks from l.
    for i in xrange(0, len(l), n):
        yield l[i:i+n]

def segment(l, fromIndex, toIndex):
    for i in xrange(fromIndex, toIndex + 1):
        yield l[i]

def trimData(nSpec, vals, min, max):
    result = []
    chunkSize = len(vals) / nSpec
    assert min >= 0, 'trimData: min is less then zero'
    assert max <= chunkSize - 1, 'trimData: max is greater than the number of spectra'
    assert min <= max, 'trimData: min is greater than max'
    chunks = split(vals,chunkSize)
    for chunk in chunks:
        seg = segment(chunk,min,max)
        for val in seg:
            result.append(val)
    return result

##############################################################################
# ConvFit
##############################################################################

def search_for_fit_params(suffix, table_ws):
    """
    Find all fit parameters in a table workspace with the given suffix.
    """
    return [name for name in mtd[table_ws].getColumnNames() if name.endswith(suffix)]

##############################################################################

def convertParametersToWorkspace(params_table, x_column, param_names, output_name):
    #search for any parameters in the table with the given parameter names,
    #ignoring their function index and output them to a workspace
    workspace_names = []
    for param_name in param_names:
        column_names = search_for_fit_params(param_name, params_table)
        column_error_names = search_for_fit_params(param_name+'_Err', params_table)
        param_workspaces = []
        for name, error_name in zip(column_names, column_error_names):
            ConvertTableToMatrixWorkspace(params_table, x_column, name, error_name, OutputWorkspace=name)
            param_workspaces.append(name)
        workspace_names.append(param_workspaces)

    #transpose list of workspaces, ignoring unequal length of lists
    #this handles the case where a parameter occurs only once in the whole workspace
    workspace_names = map(list, itertools.izip_longest(*workspace_names))
    workspace_names = [filter(None, sublist) for sublist in workspace_names]

    #join all the parameters for each peak into a single workspace per peak
    temp_workspaces = []
    for peak_params in workspace_names:
        temp_peak_ws = peak_params[0]
        for param_ws in peak_params[1:]:
            ConjoinWorkspaces(temp_peak_ws, param_ws, False)
        temp_workspaces.append(temp_peak_ws)

    #join all peaks into a single workspace
    temp_workspace = temp_workspaces[0]
    for temp_ws in temp_workspaces[1:]:
        ConjoinWorkspaces(temp_workspace, temp_peak_ws, False)

    RenameWorkspace(temp_workspace, OutputWorkspace=output_name)

    #replace axis on workspaces with text axis
    axis = TextAxis.create(mtd[output_name].getNumberHistograms())
    workspace_names = [name for sublist in workspace_names for name in sublist]
    for i, name in enumerate(workspace_names):
        axis.setLabel(i, name)
    mtd[output_name].replaceAxis(1, axis)

##############################################################################

def calculateEISF(params_table):
    #get height data from parameter table
    height = search_for_fit_params('Height', params_table)[0]
    height_error = search_for_fit_params('Height_Err', params_table)[0]
    height_y = np.asarray(mtd[params_table].column(height))
    height_e = np.asarray(mtd[params_table].column(height_error))

    #get amplitude column names
    amp_names = search_for_fit_params('Amplitude', params_table)
    amp_error_names = search_for_fit_params('Amplitude_Err', params_table)

    #for each lorentzian, calculate EISF
    for amp_name, amp_error_name in zip(amp_names, amp_error_names):
        #get amplitude from column in table workspace
        amp_y = np.asarray(mtd[params_table].column(amp_name))
        amp_e = np.asarray(mtd[params_table].column(amp_error_name))

        #calculate EISF and EISF error
        total = height_y+amp_y
        EISF_y = height_y/total

        total_error = height_e**2 + np.asarray(amp_e)**2
        EISF_e = EISF_y * np.sqrt((height_e**2/height_y**2) + (total_error/total**2))

        #append the calculated values to the table workspace
        col_name = amp_name[:-len('Amplitude')] + 'EISF'
        error_col_name = amp_error_name[:-len('Amplitude_Err')] + 'EISF_Err'

        mtd[params_table].addColumn('double', col_name)
        mtd[params_table].addColumn('double', error_col_name)

        for i, (value, error) in enumerate(zip(EISF_y, EISF_e)):
            mtd[params_table].setCell(col_name, i, value)
            mtd[params_table].setCell(error_col_name, i, error)

##############################################################################

def confitSeq(inputWS, func, startX, endX, ftype, bgd, temperature=None, specMin=0, specMax=None, Verbose=False, Plot='None', Save=False):
    StartTime('ConvFit')

    bgd = bgd[:-2]

    num_spectra = mtd[inputWS].getNumberHistograms()
    if specMin < 0 or specMax >= num_spectra:
        raise ValueError("Invalid spectrum range: %d - %d" % (specMin, specMax))

    using_delta_func = ftype[:5] == 'Delta'
    lorentzians = ftype[5:6] if using_delta_func else ftype[:1]

    if Verbose:
        logger.notice('Input files : '+str(inputWS))
        logger.notice('Fit type : Delta = ' + str(using_delta_func) + ' ; Lorentzians = ' + str(lorentzians))
        logger.notice('Background type : ' + bgd)

    output_workspace = getWSprefix(inputWS) + 'conv_' + ftype + bgd + '_' + str(specMin) + "_to_" + str(specMax)

    #convert input workspace to get Q axis
    temp_fit_workspace = "__convfit_fit_ws"
    try:
        e_fixed = getEfixed(inputWS)
        ConvertSpectrumAxis(inputWS,Target='ElasticQ',EMode='Indirect',EFixed=e_fixed,OutputWorkspace=temp_fit_workspace)
    except RuntimeError:
        #try to fall back to using whatever is currently there
        axis = mtd[inputWS].getAxis(1)
        if not axis.isNumeric():
            logger.error('Input workspace must have either spectra or numeric axis.')
            sys.exit()

        if axis.getUnit().unitID() != 'MomentumTransfer':
            logger.error('Input must have axis values of Q')
            sys.exit()

        temp_fit_workspace = inputWS

    #fit all spectra in workspace
    input_params = [temp_fit_workspace+',i%d' % i
                    for i in xrange(specMin, specMax+1)]

    PlotPeakByLogValue(Input=';'.join(input_params),
                       OutputWorkspace=output_workspace, Function=func,
                       StartX=startX, EndX=endX, FitType='Sequential',
                       CreateOutput=True, OutputCompositeMembers=True,
                       ConvolveMembers=True)

    DeleteWorkspace(output_workspace + '_NormalisedCovarianceMatrices')
    DeleteWorkspace(output_workspace + '_Parameters')
    DeleteWorkspace(temp_fit_workspace)

    wsname = output_workspace + "_Result"
    parameter_names = ['Height', 'Amplitude', 'FWHM', 'EISF']
    if using_delta_func:
        calculateEISF(output_workspace)
    convertParametersToWorkspace(output_workspace, "axis-1", parameter_names, wsname)

    #set x units to be momentum transfer
    axis = mtd[wsname].getAxis(0)
    axis.setUnit("MomentumTransfer")

    CopyLogs(InputWorkspace=inputWS, OutputWorkspace=wsname)
    AddSampleLog(Workspace=wsname, LogName="fit_program", LogType="String", LogText='ConvFit')
    AddSampleLog(Workspace=wsname, LogName='background', LogType='String', LogText=str(bgd))
    AddSampleLog(Workspace=wsname, LogName='delta_function', LogType='String', LogText=str(using_delta_func))
    AddSampleLog(Workspace=wsname, LogName='lorentzians', LogType='String', LogText=str(lorentzians))

    CopyLogs(InputWorkspace=wsname, OutputWorkspace=output_workspace + "_Workspaces")

    temp_correction = temperature is not None
    AddSampleLog(Workspace=wsname, LogName='temperature_correction', LogType='String', LogText=str(temp_correction))
    if temp_correction:
        AddSampleLog(Workspace=wsname, LogName='temperature_value', LogType='String', LogText=str(temperature))

    RenameWorkspace(InputWorkspace=output_workspace, OutputWorkspace=output_workspace + "_Parameters")
    fit_workspaces = mtd[output_workspace + '_Workspaces'].getNames()
    for i, ws in enumerate(fit_workspaces):
        RenameWorkspace(ws, OutputWorkspace=output_workspace + '_' + str(i+specMin) + '_Workspace')

    if Save:
        # path name for nxs file
        workdir = getDefaultWorkingDirectory()
        o_path = os.path.join(workdir, wsname+'.nxs')
        if Verbose:
            logger.notice('Creating file : '+ o_path)
        SaveNexusProcessed(InputWorkspace=wsname, Filename=o_path)

    if Plot == 'All':
        plotParameters(wsname, *parameter_names)
    elif Plot != 'None':
        plotParameters(wsname, Plot)

    EndTime('ConvFit')

##############################################################################
# Elwin
##############################################################################

def GetTemperature(root, tempWS, log_type, Verbose):
    (instr, run_number) = getInstrRun(tempWS)

    facility = config.getFacility()
    pad_num = facility.instrument(instr).zeroPadding(int(run_number))
    zero_padding = '0' * (pad_num - len(run_number))
    
    run_name = instr + zero_padding + run_number
    log_name = run_name.upper() + '.log'

    run = mtd[tempWS].getRun()
    unit = ['Temperature', 'K']
    if log_type in run:
        # test logs in WS
        tmp = run[log_type].value
        temp = tmp[len(tmp)-1]
        
        if Verbose:
            mess = ' Run : '+run_name +' ; Temperature in log = '+str(temp)
            logger.notice(mess)
    else:                               
        # logs not in WS
        logger.warning('Log parameter not found in workspace. Searching for log file.')
        log_path = FileFinder.getFullPath(log_name)
        
        if log_path != '':            
            # get temperature from log file
            LoadLog(Workspace=tempWS, Filename=log_path)
            run_logs = mtd[tempWS].getRun()
            tmp = run_logs[log_type].value
            temp = tmp[len(tmp)-1]
            mess = ' Run : '+run_name+' ; Temperature in file = '+str(temp)
            logger.warning(mess)
        else:
            # can't find log file            
            temp = int(run_name[-3:])
            unit = ['Run-number', 'last 3 digits']
            mess = ' Run : '+run_name +' ; Temperature file not found'
            logger.warning(mess)

    return temp,unit

def elwin(inputFiles, eRange, log_type='sample', Normalise = False,
        Save=False, Verbose=False, Plot=False): 
    StartTime('ElWin')
    workdir = config['defaultsave.directory']
    CheckXrange(eRange,'Energy')
    tempWS = '__temp'
    Range2 = ( len(eRange) == 4 )
    if Verbose:
        range1 = str(eRange[0])+' to '+str(eRange[1])
        if ( len(eRange) == 4 ): 
            range2 = str(eRange[2])+' to '+str(eRange[3])
            logger.notice('Using 2 energy ranges from '+range1+' & '+range2)
        elif ( len(eRange) == 2 ):
            logger.notice('Using 1 energy range from '+range1)
    nr = 0
    inputRuns = sorted(inputFiles)
    for file in inputRuns:
        (direct, file_name) = os.path.split(file)
        (root, ext) = os.path.splitext(file_name)
        LoadNexus(Filename=file, OutputWorkspace=tempWS)
        nsam,ntc = CheckHistZero(tempWS)
        (xval, unit) = GetTemperature(root,tempWS,log_type, Verbose)
        if Verbose:
            logger.notice('Reading file : '+file)
        if ( len(eRange) == 4 ):
            ElasticWindow(InputWorkspace=tempWS, Range1Start=eRange[0], Range1End=eRange[1], 
                Range2Start=eRange[2], Range2End=eRange[3],
                OutputInQ='__eq1', OutputInQSquared='__eq2')
        elif ( len(eRange) == 2 ):
            ElasticWindow(InputWorkspace=tempWS, Range1Start=eRange[0], Range1End=eRange[1],
                OutputInQ='__eq1', OutputInQSquared='__eq2')
        (instr, last) = getInstrRun(tempWS)
        q1 = np.array(mtd['__eq1'].readX(0))
        i1 = np.array(mtd['__eq1'].readY(0))
        e1 = np.array(mtd['__eq1'].readE(0))
        Logarithm(InputWorkspace='__eq2', OutputWorkspace='__eq2')
        q2 = np.array(mtd['__eq2'].readX(0))
        i2 = np.array(mtd['__eq2'].readY(0))
        e2 = np.array(mtd['__eq2'].readE(0))
        if (nr == 0):
            CloneWorkspace(InputWorkspace='__eq1', OutputWorkspace='__elf')
            first = getWSprefix(tempWS)
            datX1 = q1
            datY1 = i1
            datE1 = e1
            datX2 = q2
            datY2 = i2
            datE2 = e2
            Tvalue = [xval]
            Terror = [0.0]
            Taxis = str(xval)
        else:
            CloneWorkspace(InputWorkspace='__eq1', OutputWorkspace='__elftmp')
            ConjoinWorkspaces(InputWorkspace1='__elf', InputWorkspace2='__elftmp', CheckOverlapping=False)
            datX1 = np.append(datX1,q1)
            datY1 = np.append(datY1,i1)
            datE1 = np.append(datE1,e1)
            datX2 = np.append(datX2,q2)
            datY2 = np.append(datY2,i2)
            datE2 = np.append(datE2,e2)
            Tvalue.append(xval)
            Terror.append(0.0)
            Taxis += ','+str(xval)
        nr += 1
    Txa = np.array(Tvalue)
    Tea = np.array(Terror)
    nQ = len(q1)
    for nq in range(0,nQ):
        iq = []
        eq = []
        for nt in range(0,len(Tvalue)):
            ii = mtd['__elf'].readY(nt)
            iq.append(ii[nq])
            ie = mtd['__elf'].readE(nt)
            eq.append(ie[nq])
        iqa = np.array(iq)
        eqa = np.array(eq)
        if (nq == 0):
            datTx = Txa
            datTy = iqa
            datTe = eqa
        else:
            datTx = np.append(datTx,Txa)
            datTy = np.append(datTy,iqa)
            datTe = np.append(datTe,eqa)

    DeleteWorkspace('__eq1')
    DeleteWorkspace('__eq2')
    DeleteWorkspace('__elf')

    if (nr == 1):
        ename = first[:-1]
    else:
        ename = first+'to_'+last

    #check if temp was increasing or decreasing
    if(datTx[0] > datTx[-1]):
        # if so reverse data to follow natural ordering
    	datTx = datTx[::-1]
        datTy = datTy[::-1]
        datTe = datTe[::-1]

    elfWS = ename+'_elf'
    e1WS = ename+'_eq1'
    e2WS = ename+'_eq2'
    #elt only created if we normalise
    eltWS = None

    wsnames = [elfWS, e1WS, e2WS]
    
    #x,y,e data for the elf, e1 and e2 workspaces
    data = [[datTx, datTy, datTe], 
            [datX1, datY1, datE1], 
            [datX2, datY2, datE2]]
    
    #x and vertical units for the elf, e1 and e2 workspaces
    xunits = ['Energy', 'MomentumTransfer', 'QSquared']
    vunits = ['MomentumTransfer', 'Energy', 'Energy']

    #vertical axis values for the elf, e1 and e2 workspaces
    vvalues = [q1, Taxis, Taxis]
    
    #number of spectra in each workspace
    nspecs =  [nQ, nr, nr]

    #x-axis units label
    label = unit[0]+' / '+unit[1]

    wsInfo = zip(wsnames,data, xunits, vunits, vvalues, nspecs)

    #Create output workspaces and add sample logs
    for wsname, wsdata, xunit, vunit, vvalue, nspec in wsInfo:
        x, y, e = wsdata

        CreateWorkspace(OutputWorkspace=wsname, DataX=x, DataY=y, DataE=e,
            Nspec=nspec, UnitX=xunit, VerticalAxisUnit=vunit, VerticalAxisValues=vvalue)

        #add sample logs to new workspace
        CopyLogs(InputWorkspace=tempWS, OutputWorkspace=wsname)
        addElwinLogs(wsname, label, eRange, Range2)

    # remove the temp workspace now we've copied the logs
    DeleteWorkspace(tempWS)

    if unit[0] == 'Temperature':

        AddSampleLog(Workspace=e1WS, LogName="temp_normalise", 
            LogType="String", LogText=str(Normalise))

        #create workspace normalized to the lowest temperature
        if Normalise:
            eltWS = ename+'_elt'
            
            #create elt workspace
            mtd[elfWS].clone(OutputWorkspace=eltWS)
            elwinNormalizeToLowestTemp(eltWS)

            #set labels and meta data
            unitx = mtd[eltWS].getAxis(0).setUnit("Label")
            unitx.setLabel(unit[0], unit[1])
            addElwinLogs(eltWS, label, eRange, Range2)

            #append workspace name to output files list
            wsnames.append(eltWS)

    #set labels on workspace axes
    unity = mtd[e1WS].getAxis(1).setUnit("Label")
    unity.setLabel(unit[0], unit[1])
    
    unity = mtd[e2WS].getAxis(1).setUnit("Label")
    unity.setLabel(unit[0], unit[1])
    
    unitx = mtd[elfWS].getAxis(0).setUnit("Label")
    unitx.setLabel(unit[0], unit[1])

    if Save:
        elwinSaveWorkspaces(wsnames, workdir, Verbose)

    if Plot:
        elwinPlot(label,e1WS,e2WS,elfWS,eltWS)

    EndTime('Elwin')
    return e1WS,e2WS

#normalize workspace to the lowest temperature
def elwinNormalizeToLowestTemp(eltWS):
    nhist = mtd[eltWS].getNumberHistograms()
    
    #normalize each spectrum in the workspace
    for n in range(0,nhist):
        y = mtd[eltWS].readY(n)
        scale = 1.0/y[0]
        yscaled = scale * y
        mtd[eltWS].setY(n, yscaled)

# Write each of the created workspaces to file
def elwinSaveWorkspaces(flist, dir, Verbose):
    for fname in flist:
        fpath = os.path.join(dir, fname+'.nxs')

        if Verbose:
            logger.notice('Creating file : '+ fpath)

        SaveNexusProcessed(InputWorkspace=fname, Filename=fpath)

# Add sample log to each of the workspaces created by Elwin
def addElwinLogs(ws, label, eRange, Range2):

    AddSampleLog(Workspace=ws, LogName="vert_axis", LogType="String", LogText=label)
    AddSampleLog(Workspace=ws, LogName="range1_start", LogType="Number", LogText=str(eRange[0]))
    AddSampleLog(Workspace=ws, LogName="range1_end", LogType="Number", LogText=str(eRange[1]))
    AddSampleLog(Workspace=ws, LogName="two_ranges", LogType="String", LogText=str(Range2))

    if Range2:
        AddSampleLog(Workspace=ws, LogName="range2_start", LogType="Number", LogText=str(eRange[2]))
        AddSampleLog(Workspace=ws, LogName="range2_end", LogType="Number", LogText=str(eRange[3]))

#Plot each of the workspace output by elwin
def elwinPlot(label,eq1,eq2,elf,elt):
    plotElwinWorkspace(eq1, yAxisTitle='Elastic Intensity', setScale=True)
    plotElwinWorkspace(eq2, yAxisTitle='log(Elastic Intensity)', setScale=True)
    plotElwinWorkspace(elf, xAxisTitle=label)

    if elt is not None:
        plotElwinWorkspace(elt, xAxisTitle=label)

#Plot a workspace generated by Elwin
def plotElwinWorkspace(ws, xAxisTitle=None, yAxisTitle=None, setScale=False):
    ws = mtd[ws]
    nBins = ws.blocksize()
    lastX = ws.readX(0)[nBins-1]

    nhist = ws.getNumberHistograms()

    try:
        graph = mp.plotSpectrum(ws, range(0,nhist))
    except RuntimeError, e:
        #User clicked cancel on plot so don't do anything
        return None
    
    layer = graph.activeLayer()

    #set the x scale of the layer
    if setScale:
        layer.setScale(mp.Layer.Bottom, 0.0, lastX)
    
    #set the title on the x-axis
    if xAxisTitle:
        layer.setAxisTitle(mp.Layer.Bottom, xAxisTitle)

    #set the title on the y-axis
    if yAxisTitle:
        layer.setAxisTitle(mp.Layer.Left, yAxisTitle)

##############################################################################
# Fury
##############################################################################

def furyPlot(inWS, spec):
    graph = mp.plotSpectrum(inWS, spec)
    layer = graph.activeLayer()
    layer.setScale(mp.Layer.Left, 0, 1.0)

def fury(samWorkspaces, res_file, rebinParam, RES=True, Save=False, Verbose=False,
        Plot=False): 
    StartTime('Fury')
    workdir = config['defaultsave.directory']
    samTemp = samWorkspaces[0]
    nsam,npt = CheckHistZero(samTemp)
    Xin = mtd[samTemp].readX(0)
    d1 = Xin[1]-Xin[0]
    if d1 < 1e-8:
        error = 'Data energy bin is zero'
        logger.notice('ERROR *** ' + error)
        sys.exit(error)
    d2 = Xin[npt-1]-Xin[npt-2]
    dmin = min(d1,d2)
    pars = rebinParam.split(',')
    if (float(pars[1]) <= dmin):
        error = 'EWidth = ' + pars[1] + ' < smallest Eincr = ' + str(dmin)
        logger.notice('ERROR *** ' + error)
        sys.exit(error)
    outWSlist = []
    # Process RES Data Only Once
    if Verbose:
        logger.notice('Reading RES file : '+res_file)
    LoadNexus(Filename=res_file, OutputWorkspace='res_data') # RES
    CheckAnalysers(samTemp,'res_data',Verbose)
    nres,nptr = CheckHistZero('res_data')
    if nres > 1:
        CheckHistSame(samTemp,'Sample','res_data','Resolution')
    Rebin(InputWorkspace='res_data', OutputWorkspace='res_data', Params=rebinParam)
    Integration(InputWorkspace='res_data', OutputWorkspace='res_int')
    ConvertToPointData(InputWorkspace='res_data', OutputWorkspace='res_data')
    ExtractFFTSpectrum(InputWorkspace='res_data', OutputWorkspace='res_fft', FFTPart=2)
    Divide(LHSWorkspace='res_fft', RHSWorkspace='res_int', OutputWorkspace='res')
    for samWs in samWorkspaces:
        (direct, filename) = os.path.split(samWs)
        (root, ext) = os.path.splitext(filename)
        Rebin(InputWorkspace=samWs, OutputWorkspace='sam_data', Params=rebinParam)
        Integration(InputWorkspace='sam_data', OutputWorkspace='sam_int')
        ConvertToPointData(InputWorkspace='sam_data', OutputWorkspace='sam_data')
        ExtractFFTSpectrum(InputWorkspace='sam_data', OutputWorkspace='sam_fft', FFTPart=2)
        Divide(LHSWorkspace='sam_fft', RHSWorkspace='sam_int', OutputWorkspace='sam')
        # Create save file name
        savefile = getWSprefix(samWs) + 'iqt'
        outWSlist.append(savefile)
        Divide(LHSWorkspace='sam', RHSWorkspace='res', OutputWorkspace=savefile)
        #Cleanup Sample Files
        DeleteWorkspace('sam_data')
        DeleteWorkspace('sam_int')
        DeleteWorkspace('sam_fft')
        DeleteWorkspace('sam')
        # Crop nonsense values off workspace
        bin = int(math.ceil(mtd[savefile].blocksize()/2.0))
        binV = mtd[savefile].dataX(0)[bin]
        CropWorkspace(InputWorkspace=savefile, OutputWorkspace=savefile, XMax=binV)
        if Save:
            opath = os.path.join(workdir, savefile+'.nxs')					# path name for nxs file
            SaveNexusProcessed(InputWorkspace=savefile, Filename=opath)
            if Verbose:
                logger.notice('Output file : '+opath)  
    # Clean Up RES files
    DeleteWorkspace('res_data')
    DeleteWorkspace('res_int')
    DeleteWorkspace('res_fft')
    DeleteWorkspace('res')
    if Plot:
        specrange = range(0,mtd[outWSlist[0]].getNumberHistograms())
        furyPlot(outWSlist, specrange)
    EndTime('Fury')
    return outWSlist

##############################################################################
# FuryFit
##############################################################################

def getFuryFitOption(option):
    nopt = len(option)
    if nopt == 2:
        npeak = option[0]
        type = option[1]
    elif nopt == 4:
        npeak = '2'
        type = 'SE'
    else:
        error = 'Bad option : ' +option
        logger.notice('ERROR *** ' + error)
        sys.exit(error)
    return npeak, type

def furyfitParsToWS(Table, Data, option):
    npeak, type = getFuryFitOption(option)   
    Q = createQaxis(Data)
    nQ = len(Q)
    ws = mtd[Table]
    rCount = ws.rowCount()
    cCount = ws.columnCount()
    cName =  ws.getColumnNames()
    Qa = np.array(Q)
    A0v = ws.column(1)     #bgd value
    A0e = ws.column(2)     #bgd error
    Iy1 = ws.column(5)      #intensity1 value
    Ie1 = ws.column(2)      #intensity1 error = bgd
    dataX = Qa
    dataY = np.array(A0v)
    dataE = np.array(A0e)
    names = cName[1]
    dataX = np.append(dataX,Qa)
    dataY = np.append(dataY,np.array(Iy1))
    dataE = np.append(dataE,np.array(Ie1))
    names += ","+cName[5]
    Ty1 = ws.column(7)      #tau1 value
    Te1 = ws.column(8)      #tau1 error
    dataX = np.append(dataX,Qa)
    dataY = np.append(dataY,np.array(Ty1))
    dataE = np.append(dataE,np.array(Te1))
    names += ","+cName[7]
    nSpec = 3
    if npeak == '1' and type == 'S':
        By1 = ws.column(9)  #beta1 value
        Be1 = ws.column(10) #beta2 error
        dataX = np.append(dataX,Qa)
        dataY = np.append(dataY,np.array(By1))
        dataE = np.append(dataE,np.array(Be1))
        names += ","+cName[9]
        nSpec += 1
    if npeak == '2':
        Iy2 = ws.column(9)  #intensity2 value
        Ie2 = ws.column(10) #intensity2 error
        dataX = np.append(dataX,Qa)
        dataY = np.append(dataY,np.array(Iy2))
        dataE = np.append(dataE,np.array(Ie2))
        names += ","+cName[9]
        nSpec += 1
        Ty2 = ws.column(11)  #tau2 value
        Te2 = ws.column(12) #tau2 error
        dataX = np.append(dataX,Qa)
        dataY = np.append(dataY,np.array(Ty2))
        dataE = np.append(dataE,np.array(Te2))
        names += ","+cName[11]
        nSpec += 1
    wsname = Table + "_Workspace"
    CreateWorkspace(OutputWorkspace=wsname, DataX=dataX, DataY=dataY, DataE=dataE, 
        Nspec=nSpec, UnitX='MomentumTransfer', VerticalAxisUnit='Text',
        VerticalAxisValues=names)
    return wsname

def createFurySeqResFun(ties, par, option):
    npeak, type = getFuryFitOption(option)   
    fun = 'name=LinearBackground,A0='+str(par[0])+',A1=0,ties=(A1=0);'
    
    npeak = int(npeak)

    if npeak >= 1 and type == 'E':
        #one exponential
        fun += 'name=UserFunction,Formula=Intensity*exp(-(x/Tau)),Intensity='+str(par[1])+',Tau='+str(par[2])

    if npeak == 2 and type == 'E':
        #two exponentials
        fun += ';name=UserFunction,Formula=Intensity*exp(-(x/Tau)),Intensity='+str(par[3])+',Tau='+str(par[4])

    if npeak == 1 and type == 'S':
        #one stretched exponential
        fun += 'name=UserFunction,Formula=Intensity*exp(-(x/Tau)^Beta),Intensity='+str(par[1])+',Tau='+str(par[2])+',Beta='+str(par[3])

    if npeak == 2 and type == 'SE':
        #one exponential, one stretched exponential
        fun += 'name=UserFunction,Formula=Intensity*exp(-(x/Tau)),Intensity='+str(par[1])+',Tau='+str(par[2])
        fun += ';name=UserFunction,Formula=Intensity*exp(-(x/Tau)^Beta),Intensity='+str(par[3])+',Tau='+str(par[4])+',Beta='+str(par[5])

    if ties:
        fun += ';ties=(f1.Intensity=1-f0.A0)'
    
    return fun

def getFurySeqResult(inputWS, outNm, option, Verbose):
    logger.notice('Option : ' +option)
    fitWS = outNm + '_Result_'
    npeak, type = getFuryFitOption(option)

    #table workspace containing parameters for fit 
    params = mtd[outNm+'_Parameters']
    
    #list of columns containing fit parameters
    #start with the background value
    paramColumnNames = ['f0.A0']

    #add fit params from both peaks
    for i in range(1,int(npeak)+1):
        paramColumnNames += ['f'+str(i)+'.Intensity', 'f'+str(i)+'.Tau']

    #add beta value if using a stretched exponetial
    if type == 'SE' or type == 'S':
        paramColumnNames.append('f'+npeak+'.Beta')

    group = []
    nHist = mtd[inputWS].getNumberHistograms()
    for i in range(nHist):
        #get all the applicable parameters for this iteration
        paramRow = params.row(i)
        paras = [paramRow[key] for key in paramColumnNames]

        #build function string with our parameters included
        func = createFurySeqResFun(True, paras, option)

        if Verbose:
            logger.notice('Fit func : '+func)
        
        fout = fitWS + str(i)
        
        #run fit function and collection generated workspace
        Fit(Function=func,InputWorkspace=inputWS,WorkspaceIndex=i,Output=fout,
            CreateOutput=True, MaxIterations=0,OutputCompositeMembers=True, ConvolveMembers=True)
        RenameWorkspace(InputWorkspace=fout+'_Workspace', OutputWorkspace=fout)
        unitx = mtd[fout].getAxis(0).setUnit("Label")
        unitx.setLabel('Time' , 'ns')
        
        #clean up fit output
        DeleteWorkspace(fout+'_NormalisedCovarianceMatrix')
        DeleteWorkspace(fout+'_Parameters')

        #add generated workspace to group
        group.append(fout)
    
    GroupWorkspaces(InputWorkspaces=group,OutputWorkspace=fitWS[:-1])

def furyfitPlotSeq(ws, plot):
    param_names = [plot]
    if plot == 'All':
        param_names = ['Intensity', 'Tau', 'Beta']
    plotParameters(ws, param_names)

def furyfitSeq(inputWS, func, ftype, startx, endx, intensities_constrained=False, Save=False, Plot='None', Verbose=False): 
    StartTime('FuryFit')
    
    workdir = config['defaultsave.directory']
    nHist = mtd[inputWS].getNumberHistograms()
   
    #name stem for generated workspace
    outNm = getWSprefix(inputWS) + 'fury_' + ftype + "0_to_" + str(nHist-1)
    
    fitType = ftype[:-2]
    if Verbose:
        logger.notice('Option: '+fitType)  
        logger.notice(func)

    #build input string for PlotPeakByLogValue
    input = [inputWS +',i' + str(i) for i in range(0,nHist)]
    input = ';'.join(input)
    
    PlotPeakByLogValue(Input=input, OutputWorkspace=outNm, Function=func, 
        StartX=startx, EndX=endx, FitType='Sequential')
    
    fitWS = furyfitParsToWS(outNm, inputWS, fitType)
    RenameWorkspace(InputWorkspace=outNm, OutputWorkspace=outNm+"_Parameters")
    CropWorkspace(InputWorkspace=inputWS, OutputWorkspace=inputWS, XMin=startx, XMax=endx)

    getFurySeqResult(inputWS, outNm, fitType, Verbose)
    
    #process generated workspaces
    wsnames = [fitWS, outNm+'_Result']
    params = [startx, endx, fitType]
    for ws in wsnames:
        furyAddSampleLogs(inputWS, ws, params, intensities_constrained=intensities_constrained)

        if Save:
            #save workspace to default directory
            fpath = os.path.join(workdir, ws+'.nxs')
            SaveNexusProcessed(InputWorkspace=ws, Filename=fpath)

            if Verbose:
                logger.notice(ws + ' output to file : '+fpath)

    if ( Plot != 'None' ):
        furyfitPlotSeq(fitWS, Plot)

    EndTime('FuryFit')

    return mtd[fitWS]

#Copy logs from sample and add some addtional ones
def furyAddSampleLogs(inputWs, ws, params, intensities_constrained=False, beta_constrained=False):
    startx, endx, fitType = params
    CopyLogs(InputWorkspace=inputWs, OutputWorkspace=ws)
    AddSampleLog(Workspace=ws, LogName="start_x", LogType="Number", LogText=str(startx))
    AddSampleLog(Workspace=ws, LogName="end_x", LogType="Number", LogText=str(endx))
    AddSampleLog(Workspace=ws, LogName="fit_type", LogType="String", LogText=fitType)
    AddSampleLog(Workspace=ws, LogName="intensities_constrained", LogType="String", LogText=str(intensities_constrained))
    AddSampleLog(Workspace=ws, LogName="beta_constrained", LogType="String", LogText=str(beta_constrained))

def furyfitMultParsToWS(Table, Data):
#   Q = createQaxis(Data)
    theta,Q = GetThetaQ(Data)
    ws = mtd[Table+'_Parameters']
    rCount = ws.rowCount()
    cCount = ws.columnCount()
    nSpec = ( rCount - 1 ) / 5
    val = ws.column(1)     #value
    err = ws.column(2)     #error
    dataX = []
    A0val = []
    A0err = []
    Ival = []
    Ierr = []
    Tval = []
    Terr = []
    Bval = []
    Berr = []
    for spec in range(0,nSpec):
        n1 = spec*5
        A0 = n1
        A1 = n1+1
        int = n1+2                   #intensity value
        tau = n1+3                   #tau value
        beta = n1+4                   #beta value
        dataX.append(Q[spec])
        A0val.append(val[A0])
        A0err.append(err[A0])
        Ival.append(val[int])
        Ierr.append(err[int])
        Tval.append(val[tau])
        Terr.append(err[tau])
        Bval.append(val[beta])
        Berr.append(err[beta])
    nQ = len(dataX)
    Qa = np.array(dataX)
    dataY = np.array(A0val)
    dataE = np.array(A0err)
    dataY = np.append(dataY,np.array(Ival))
    dataE = np.append(dataE,np.array(Ierr))
    dataY = np.append(dataY,np.array(Tval))
    dataE = np.append(dataE,np.array(Terr))
    dataY = np.append(dataY,np.array(Bval))
    dataE = np.append(dataE,np.array(Berr))
    names = 'f0.A0,f1.Intensity,f1.Tau,f1.Beta'
    suffix = 'Workspace'
    wsname = Table + '_' + suffix
    CreateWorkspace(OutputWorkspace=wsname, DataX=Qa, DataY=dataY, DataE=dataE, 
        Nspec=4, UnitX='MomentumTransfer', VerticalAxisUnit='Text',
        VerticalAxisValues=names)
    return wsname

def createFuryMultFun(ties = True, function = ''):
    fun =  '(composite=CompositeFunction,$domains=i;'
    fun += function
    if ties:
        fun += ';ties=(f1.Intensity=1-f0.A0)'
    fun += ');'
    return fun

def createFuryMultResFun(ties = True, A0 = 0.02, Intensity = 0.98 ,Tau = 0.025, Beta = 0.8):
    fun =  '(composite=CompositeFunction,$domains=i;'
    fun += 'name=LinearBackground,A0='+str(A0)+',A1=0,ties=(A1=0);'
    fun += 'name=UserFunction,Formula=Intensity*exp(-(x/Tau)^Beta),Intensity='+str(Intensity)+',Tau='+str(Tau)+',Beta='+str(Beta)
    if ties:
        fun += ';ties=(f1.Intensity=1-f0.A0)'
    fun += ');'
    return fun

def getFuryMultResult(inputWS, outNm, function, Verbose):
    params = mtd[outNm+'_Parameters']
    nHist = mtd[inputWS].getNumberHistograms()
    for i in range(nHist):
        j = 5 * i
#        assert( params.row(j)['Name'][3:] == 'f0.A0' )
        A0 = params.row(j)['Value']
        A1 = params.row(j + 1)['Value']
        Intensity = params.row(j + 2)['Value']
        Tau = params.row(j + 3)['Value']
        Beta = params.row(j + 4)['Value']
        func = createFuryMultResFun(True,  A0, Intensity ,Tau, Beta)
        if Verbose:
            logger.notice('Fit func : '+func)  	
        fitWS = outNm + '_Result_'
        fout = fitWS + str(i)
        Fit(Function=func,InputWorkspace=inputWS,WorkspaceIndex=i,Output=fout,MaxIterations=0)
        unitx = mtd[fout+'_Workspace'].getAxis(0).setUnit("Label")
        unitx.setLabel('Time' , 'ns')
        RenameWorkspace(InputWorkspace=fout+'_Workspace', OutputWorkspace=fout)
        DeleteWorkspace(fitWS+str(i)+'_NormalisedCovarianceMatrix')
        DeleteWorkspace(fitWS+str(i)+'_Parameters')
        if i == 0:
            group = fout
        else:
            group += ',' + fout
    GroupWorkspaces(InputWorkspaces=group,OutputWorkspace=fitWS[:-1])

def furyfitMult(inputWS, function, ftype, startx, endx, intensities_constrained=False, Save=False, Plot='None', Verbose=False):
    StartTime('FuryFit Mult')
    workdir = config['defaultsave.directory']
    option = ftype[:-2]
    if Verbose:
        logger.notice('Option: '+option)  
        logger.notice('Function: '+function)  
    nHist = mtd[inputWS].getNumberHistograms()
    outNm = getWSprefix(inputWS) + 'fury_1Smult_s0_to_' + str(nHist-1)
    f1 = createFuryMultFun(True, function)
    func= 'composite=MultiDomainFunction,NumDeriv=1;'
    ties='ties=('
    kwargs = {}
    for i in range(0,nHist):
        func+=f1
        if i > 0:
            ties += 'f' + str(i) + '.f1.Beta=f0.f1.Beta'
            if i < nHist-1:
                ties += ','
            kwargs['InputWorkspace_' + str(i)] = inputWS
        kwargs['WorkspaceIndex_' + str(i)] = i
    ties+=')'
    func += ties
    CropWorkspace(InputWorkspace=inputWS, OutputWorkspace=inputWS, XMin=startx, XMax=endx)
    Fit(Function=func,InputWorkspace=inputWS,WorkspaceIndex=0,Output=outNm,**kwargs)
    outWS = furyfitMultParsToWS(outNm, inputWS)
    result_workspace = outNm + '_Result'
    getFuryMultResult(inputWS, outNm, function, Verbose)

    params = [startx, endx, ftype]
    furyAddSampleLogs(inputWS, outWS, params, intensities_constrained=intensities_constrained, beta_constrained=True)
    furyAddSampleLogs(inputWS, result_workspace, params, intensities_constrained=intensities_constrained, beta_constrained=True)

    if Save:
        opath = os.path.join(workdir, outWS+'.nxs')					# path name for nxs file
        SaveNexusProcessed(InputWorkspace=outWS, Filename=opath)
        rpath = os.path.join(workdir, result_workspace+'.nxs')					# path name for nxs file
        SaveNexusProcessed(InputWorkspace=result_workspace, Filename=rpath)
        if Verbose:
            logger.notice('Output file : '+opath)  
            logger.notice('Output file : '+rpath)  
    if ( Plot != 'None' ):
        furyfitPlotSeq(outWS, Plot)
    EndTime('FuryFit')

##############################################################################
# MSDFit
##############################################################################

def msdfitParsToWS(Table, xData):
    dataX = xData
    ws = mtd[Table+'_Table']
    rCount = ws.rowCount()
    yA0 = ws.column(1)
    eA0 = ws.column(2)
    yA1 = ws.column(3)  
    dataY1 = map(lambda x : -x, yA1) 
    eA1 = ws.column(4)
    wsname = Table

    #check if temp was increasing or decreasing
    if(dataX[0] > dataX[-1]):
        # if so reverse data to follow natural ordering
        dataX = dataX[::-1]
        dataY1 = dataY1[::-1]
        eA1 = eA1[::-1]

    CreateWorkspace(OutputWorkspace=wsname+'_a0', DataX=dataX, DataY=yA0, DataE=eA0,
        Nspec=1, UnitX='')
    CreateWorkspace(OutputWorkspace=wsname+'_a1', DataX=dataX, DataY=dataY1, DataE=eA1,
        Nspec=1, UnitX='')
    group = wsname+'_a0,'+wsname+'_a1'
    GroupWorkspaces(InputWorkspaces=group,OutputWorkspace=wsname)
    return wsname

def msdfitPlotSeq(inputWS, xlabel):
    msd_plot = mp.plotSpectrum(inputWS+'_a1',0,True)
    msd_layer = msd_plot.activeLayer()
    msd_layer.setAxisTitle(mp.Layer.Bottom,xlabel)
    msd_layer.setAxisTitle(mp.Layer.Left,'<u2>')

def msdfitPlotFits(calcWS, n):
    mfit_plot = mp.plotSpectrum(calcWS+'_0',[0,1],True)
    mfit_layer = mfit_plot.activeLayer()
    mfit_layer.setAxisTitle(mp.Layer.Left,'log(Elastic Intensity)')

def msdfit(inputs, startX, endX, Save=False, Verbose=False, Plot=True): 
    StartTime('msdFit')
    workdir = config['defaultsave.directory']
    log_type = 'sample'
    
    file_name = inputs[0]
    base_name = os.path.basename(file_name)
    (root,ext) = os.path.splitext(base_name)
    
    if Verbose:
        logger.notice('Reading Run : ' + file_name)
    
    LoadNexusProcessed(FileName=file_name, OutputWorkspace=root)
    
    nHist = mtd[root].getNumberHistograms()
    file_list = []
    run_list = []
    ws = mtd[root]
    ws_run = ws.getRun()
    vertAxisValues = ws.getAxis(1).extractValues()
    x_list = vertAxisValues
    if 'vert_axis' in ws_run:
        xlabel = ws_run.getLogData('vert_axis').value
    for nr in range(0, nHist):
        nsam,ntc = CheckHistZero(root)
        lnWS = '__lnI_'+str(nr)
        file_list.append(lnWS)
        ExtractSingleSpectrum(InputWorkspace=root, OutputWorkspace=lnWS,
            WorkspaceIndex=nr)
        if (nr == 0):
            run_list = lnWS
        else:
            run_list += ';'+lnWS
    mname = root[:-4]
    msdWS = mname+'_msd'
    if Verbose:
       logger.notice('Fitting Runs '+mname)
       logger.notice('Q-range from '+str(startX)+' to '+str(endX))
    function = 'name=LinearBackground, A0=0, A1=0'
    PlotPeakByLogValue(Input=run_list, OutputWorkspace=msdWS+'_Table', Function=function,
        StartX=startX, EndX=endX, FitType = 'Sequential')
    msdfitParsToWS(msdWS, x_list)
    nr = 0
    fitWS = mname+'_Fit'
    calcWS = mname+'_msd_Result'
    a0 = mtd[msdWS+'_a0'].readY(0)
    a1 = mtd[msdWS+'_a1'].readY(0)
    for nr in range(0, nHist):
        inWS = file_list[nr]
        CropWorkspace(InputWorkspace=inWS,OutputWorkspace='__data',XMin=0.95*startX,XMax=1.05*endX)
        dataX = mtd['__data'].readX(0)
        nxd = len(dataX)
        dataX = np.append(dataX,2*dataX[nxd-1]-dataX[nxd-2])
        dataY = np.array(mtd['__data'].readY(0))
        dataE = np.array(mtd['__data'].readE(0))
        xd = []
        yd = []
        ed = []
        for n in range(0,nxd):
            line = a0[nr] - a1[nr]*dataX[n]
            xd.append(dataX[n])
            yd.append(line)
            ed.append(0.0)
        xd.append(dataX[nxd])
        dataX = np.append(dataX,np.array(xd))
        dataY = np.append(dataY,np.array(yd))
        dataE = np.append(dataE,np.array(ed))
        fout = calcWS +'_'+ str(nr)
        CreateWorkspace(OutputWorkspace=fout, DataX=dataX, DataY=dataY, DataE=dataE,
            Nspec=2, UnitX='DeltaE', VerticalAxisUnit='Text', VerticalAxisValues='Data,Calc')
        if nr == 0:
            gro = fout
        else:
            gro += ',' + fout
        DeleteWorkspace(inWS)
        DeleteWorkspace('__data')
    GroupWorkspaces(InputWorkspaces=gro,OutputWorkspace=calcWS)

    #add sample logs to output workspace
    CopyLogs(InputWorkspace=root, OutputWorkspace=msdWS)
    AddSampleLog(Workspace=msdWS, LogName="start_x", LogType="Number", LogText=str(startX))
    AddSampleLog(Workspace=msdWS, LogName="end_x", LogType="Number", LogText=str(endX))
    
    if Plot:
        msdfitPlotSeq(msdWS, xlabel)
        msdfitPlotFits(calcWS, 0)
    if Save:
        msd_path = os.path.join(workdir, msdWS+'.nxs')					# path name for nxs file
        SaveNexusProcessed(InputWorkspace=msdWS, Filename=msd_path, Title=msdWS)
        if Verbose:
            logger.notice('Output msd file : '+msd_path)  
    EndTime('msdFit')
    return msdWS

def plotInput(inputfiles,spectra=[]):
    OneSpectra = False
    if len(spectra) != 2:
        spectra = [spectra[0], spectra[0]]
        OneSpectra = True
    workspaces = []
    for file in inputfiles:
        root = LoadNexus(Filename=file)
        if not OneSpectra:
            GroupDetectors(root, root,
                DetectorList=range(spectra[0],spectra[1]+1) )
        workspaces.append(root)
    if len(workspaces) > 0:
        graph = mp.plotSpectrum(workspaces,0)
        layer = graph.activeLayer().setTitle(", ".join(workspaces))
        
##############################################################################
# Corrections
##############################################################################

def CubicFit(inputWS, spec, Verbose=False):
    '''Uses the Mantid Fit Algorithm to fit a quadratic to the inputWS
    parameter. Returns a list containing the fitted parameter values.'''
    function = 'name=Quadratic, A0=1, A1=0, A2=0'
    fit = Fit(Function=function, InputWorkspace=inputWS, WorkspaceIndex=spec,
      CreateOutput=True, Output='Fit')
    table = mtd['Fit_Parameters']
    A0 = table.cell(0,1)
    A1 = table.cell(1,1)
    A2 = table.cell(2,1)
    Abs = [A0, A1, A2]
    if Verbose:
       logger.notice('Group '+str(spec)+' of '+inputWS+' ; fit coefficients are : '+str(Abs))
    return Abs

def applyCorrections(inputWS, canWS, corr, Verbose=False):
    '''Through the PolynomialCorrection algorithm, makes corrections to the
    input workspace based on the supplied correction values.'''
    # Corrections are applied in Lambda (Wavelength)
    efixed = getEfixed(inputWS)                # Get efixed
    theta,Q = GetThetaQ(inputWS)
    sam_name = getWSprefix(inputWS)
    ConvertUnits(InputWorkspace=inputWS, OutputWorkspace=inputWS, Target='Wavelength',
        EMode='Indirect', EFixed=efixed)

    nameStem = corr[:-4]
    if canWS != '':
        (instr, can_run) = getInstrRun(canWS)
        corrections = [nameStem+'_ass', nameStem+'_assc', nameStem+'_acsc', nameStem+'_acc']
        CorrectedWS = sam_name +'Correct_'+ can_run
        ConvertUnits(InputWorkspace=canWS, OutputWorkspace=canWS, Target='Wavelength',
            EMode='Indirect', EFixed=efixed)
    else:
        corrections = [nameStem+'_ass']
        CorrectedWS = sam_name +'Corrected'
    nHist = mtd[inputWS].getNumberHistograms()
    # Check that number of histograms in each corrections workspace matches
    # that of the input (sample) workspace
    for ws in corrections:
        if ( mtd[ws].getNumberHistograms() != nHist ):
            raise ValueError('Mismatch: num of spectra in '+ws+' and inputWS')
    # Workspaces that hold intermediate results
    CorrectedSampleWS = '__csam'
    CorrectedCanWS = '__ccan'
    for i in range(0, nHist): # Loop through each spectra in the inputWS
        ExtractSingleSpectrum(InputWorkspace=inputWS, OutputWorkspace=CorrectedSampleWS,
            WorkspaceIndex=i)
        if ( len(corrections) == 1 ):
            Ass = CubicFit(corrections[0], i, Verbose)
            PolynomialCorrection(InputWorkspace=CorrectedSampleWS, OutputWorkspace=CorrectedSampleWS,
                Coefficients=Ass, Operation='Divide')
            if ( i == 0 ):
                CloneWorkspace(InputWorkspace=CorrectedSampleWS, OutputWorkspace=CorrectedWS)
            else:
                ConjoinWorkspaces(InputWorkspace1=CorrectedWS, InputWorkspace2=CorrectedSampleWS)
        else:
            ExtractSingleSpectrum(InputWorkspace=canWS, OutputWorkspace=CorrectedCanWS,
                WorkspaceIndex=i)
            Acc = CubicFit(corrections[3], i, Verbose)
            PolynomialCorrection(InputWorkspace=CorrectedCanWS, OutputWorkspace=CorrectedCanWS,
                Coefficients=Acc, Operation='Divide')
            Acsc = CubicFit(corrections[2], i, Verbose)
            PolynomialCorrection(InputWorkspace=CorrectedCanWS, OutputWorkspace=CorrectedCanWS,
                Coefficients=Acsc, Operation='Multiply')
            Minus(LHSWorkspace=CorrectedSampleWS, RHSWorkspace=CorrectedCanWS, OutputWorkspace=CorrectedSampleWS)
            Assc = CubicFit(corrections[1], i, Verbose)
            PolynomialCorrection(InputWorkspace=CorrectedSampleWS, OutputWorkspace=CorrectedSampleWS,
                Coefficients=Assc, Operation='Divide')
            if ( i == 0 ):
                CloneWorkspace(InputWorkspace=CorrectedSampleWS, OutputWorkspace=CorrectedWS)
            else:
                ConjoinWorkspaces(InputWorkspace1=CorrectedWS, InputWorkspace2=CorrectedSampleWS,
                                  CheckOverlapping=False)
    ConvertUnits(InputWorkspace=inputWS, OutputWorkspace=inputWS, Target='DeltaE',
        EMode='Indirect', EFixed=efixed)
    ConvertUnits(InputWorkspace=CorrectedWS, OutputWorkspace=CorrectedWS, Target='DeltaE',
        EMode='Indirect', EFixed=efixed)
    ConvertSpectrumAxis(InputWorkspace=CorrectedWS, OutputWorkspace=CorrectedWS+'_rqw', 
        Target='ElasticQ', EMode='Indirect', EFixed=efixed)
    RenameWorkspace(InputWorkspace=CorrectedWS, OutputWorkspace=CorrectedWS+'_red')
    if canWS != '':
        ConvertUnits(InputWorkspace=canWS, OutputWorkspace=canWS, Target='DeltaE',
            EMode='Indirect', EFixed=efixed)
    DeleteWorkspace('Fit_NormalisedCovarianceMatrix')
    DeleteWorkspace('Fit_Parameters')
    DeleteWorkspace('Fit_Workspace')
    return CorrectedWS
                
def abscorFeeder(sample, container, geom, useCor, corrections, Verbose=False, ScaleOrNotToScale=False, factor=1, Save=False,
        PlotResult='None', PlotContrib=False):
    '''Load up the necessary files and then passes them into the main
    applyCorrections routine.'''
    StartTime('ApplyCorrections')
    workdir = config['defaultsave.directory']
    s_hist,sxlen = CheckHistZero(sample)
    sam_name = getWSprefix(sample)
    efixed = getEfixed(sample)
    if container != '':
        CheckAnalysers(sample,container,Verbose)
        CheckHistSame(sample,'Sample',container,'Container')
        (instr, can_run) = getInstrRun(container)
        if ScaleOrNotToScale:
            Scale(InputWorkspace=container, OutputWorkspace=container, Factor=factor, Operation='Multiply')
            if Verbose:
                logger.notice('Container scaled by '+str(factor))
    if useCor:
        if Verbose:
            text = 'Correcting sample ' + sample
            if container != '':
                text += ' with ' + container
            logger.notice(text)
            
        cor_result = applyCorrections(sample, container, corrections, Verbose)
        rws = mtd[cor_result+'_red']
        outNm= cor_result + '_Result_'

        if Save:
            cred_path = os.path.join(workdir,cor_result+'_red.nxs')
            SaveNexusProcessed(InputWorkspace=cor_result+'_red',Filename=cred_path)
            if Verbose:
                logger.notice('Output file created : '+cred_path)
        calc_plot = [cor_result+'_red',sample]
        res_plot = cor_result+'_rqw'
    else:
        if ( container == '' ):
            sys.exit('ERROR *** Invalid options - nothing to do!')
        else:
            sub_result = sam_name +'Subtract_'+ can_run
            if Verbose:
                logger.notice('Subtracting '+container+' from '+sample)
            Minus(LHSWorkspace=sample,RHSWorkspace=container,OutputWorkspace=sub_result)
            ConvertSpectrumAxis(InputWorkspace=sub_result, OutputWorkspace=sub_result+'_rqw', 
                Target='ElasticQ', EMode='Indirect', EFixed=efixed)
            RenameWorkspace(InputWorkspace=sub_result, OutputWorkspace=sub_result+'_red')
            rws = mtd[sub_result+'_red']
            outNm= sub_result + '_Result_'
            if Save:
                sred_path = os.path.join(workdir,sub_result+'_red.nxs')
                SaveNexusProcessed(InputWorkspace=sub_result+'_red',Filename=sred_path)
                if Verbose:
                    logger.notice('Output file created : '+sred_path)
            res_plot = sub_result+'_rqw'
    
    if (PlotResult != 'None'):
        plotCorrResult(res_plot,PlotResult)

    if ( container != '' ):
        sws = mtd[sample]
        cws = mtd[container]
        names = 'Sample,Can,Calc'
        for i in range(0, s_hist): # Loop through each spectra in the inputWS
            dataX = np.array(sws.readX(i))
            dataY = np.array(sws.readY(i))
            dataE = np.array(sws.readE(i))
            dataX = np.append(dataX,np.array(cws.readX(i)))
            dataY = np.append(dataY,np.array(cws.readY(i)))
            dataE = np.append(dataE,np.array(cws.readE(i)))
            dataX = np.append(dataX,np.array(rws.readX(i)))
            dataY = np.append(dataY,np.array(rws.readY(i)))
            dataE = np.append(dataE,np.array(rws.readE(i)))
            fout = outNm + str(i)
            CreateWorkspace(OutputWorkspace=fout, DataX=dataX, DataY=dataY, DataE=dataE,
                Nspec=3, UnitX='DeltaE', VerticalAxisUnit='Text', VerticalAxisValues=names)
            if i == 0:
                group = fout
            else:
                group += ',' + fout
        GroupWorkspaces(InputWorkspaces=group,OutputWorkspace=outNm[:-1])
        if PlotContrib:
            plotCorrContrib(outNm+'0',[0,1,2])
        if Save:
            res_path = os.path.join(workdir,outNm[:-1]+'.nxs')
            SaveNexusProcessed(InputWorkspace=outNm[:-1],Filename=res_path)
            if Verbose:
                logger.notice('Output file created : '+res_path)
    EndTime('ApplyCorrections')

def plotCorrResult(inWS,PlotResult):
    nHist = mtd[inWS].getNumberHistograms()
    if (PlotResult == 'Spectrum' or PlotResult == 'Both'):
        if nHist >= 10:                       #only plot up to 10 hists
            nHist = 10
        plot_list = []
        for i in range(0, nHist):
            plot_list.append(i)
        res_plot=mp.plotSpectrum(inWS,plot_list)
    if (PlotResult == 'Contour' or PlotResult == 'Both'):
        if nHist >= 5:                        #needs at least 5 hists for a contour
            mp.importMatrixWorkspace(inWS).plotGraph2D()

def plotCorrContrib(plot_list,n):
        con_plot=mp.plotSpectrum(plot_list,n)
