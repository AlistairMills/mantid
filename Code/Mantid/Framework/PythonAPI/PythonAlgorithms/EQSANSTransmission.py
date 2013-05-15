"""*WIKI* 

Transmission counts is the signal going through a pinhole in the middle of the beam stop (after hitting the sample). The number of counts is equal to the neutron flux times the transmission N_trans = T*Phi. The transmission T is obtained by normalizing the N_trans by the monitor or accelerator current. T = N_trans/current. Dividing the pixel counts by T we account for transmission.         

*WIKI*"""

from MantidFramework import *
from mantidsimple import *
import math

# Keep track of whether we have numpy since readX() may
# return a numpy array or a list depending on whether it
# is available.
try:
    import numpy
    HAS_NUMPY = True
except:
    HAS_NUMPY = False    

class EQSANSTransmission(PythonAlgorithm):
    """
        Calculate transmission for EQSANS
        
        Transmission counts is the signal going through a pinhole in the middle of the 
        beam stop (after hitting the sample). The number of counts is equal to the neutron
        flux times the transmission N_trans = T*Phi. The transmission T is obtained by normalizing
        the N_trans by the monitor or accelerator current. T = N_trans/current. 
        Dividing the pixel counts by T we account for transmission.         
    """
    
    # Size of the box around the beam center
    NX_TRANS_PIX = 10
    NY_TRANS_PIX = 10
    
    # Number of Y pixels
    NX_PIX = 192
    NY_PIX = 256
    
    # To define transmission peak
    transmission_peak_to_bg_ratio = 1000
    
    # To define transmission peak mask
    min_transmission_peak_to_bg_ratio = 5
    
    def category(self):
        return "SANS;PythonAlgorithms"

    def name(self):
        return "EQSANSTransmission"

    def PyInit(self):
        # Input workspace
        self.declareWorkspaceProperty("InputWorkspace", "", Direction.Input, Description="Name the workspace to calculate the transmission from")
        # Output workspace to put the transmission histo into
        self.declareProperty("OutputWorkspace", "", Description="Name of the workspace that will contain the transmission histogram")
        # X position of the beam center
        self.declareProperty("XCenter", 96.0, Description="Position of the beam center in X [pixel]. Default: 96")
        # Y position of the beam center
        self.declareProperty("YCenter", 128.0, Description="Position of the beam center in Y [pixel]. Default: 128")
        # Transmission will be normalized to 1 if True
        self.declareProperty("NormalizeToUnity", False, Description="If True, the transmission will be normalized to unity. Default: False")
        # Set the wiki summary
        self.setWikiSummary("Computes the transmission for EQSANS data. The algorithm looks for a peak within a 10x10 pixel area around the provided beam center.")


    def PyExec(self):
        input_ws = self.getProperty("InputWorkspace")
        output_ws = self.getProperty("OutputWorkspace")
        xcenter = int(math.floor(self.getProperty("XCenter")))
        ycenter = int(math.floor(self.getProperty("YCenter")))
        normalize = self.getProperty("NormalizeToUnity")

        # Check that the given beam center is within the detector boundaries
        if xcenter<0 or xcenter>=self.NX_PIX or ycenter<0 or ycenter>=self.NY_PIX:
            self.log().error("Beam center is not within the detector boundaries: (%6.1f, %-6.1f)" % (xcenter, ycenter))
            if xcenter<0: xcenter = 0
            if xcenter>=self.NX_PIX: xcenter = self.NX_PIX-1
            if ycenter<0: ycenter = 0
            if ycenter>=self.NY_PIX: ycenter = self.NY_PIX-1
        
        nx_min = xcenter - self.NX_TRANS_PIX
        nx_max = xcenter + self.NX_TRANS_PIX
        ny_min = ycenter - self.NY_TRANS_PIX
        ny_max = ycenter + self.NY_TRANS_PIX

        # Check that pixels are on the detector
        if nx_min<0: nx_min = 0
        if nx_min>=self.NX_PIX: nx_min = self.NX_PIX-1
        if nx_max<0: nx_max = 0
        if nx_max>=self.NX_PIX: nx_max = self.NX_PIX-1
        
        if ny_min<0: ny_min = 0
        if ny_min>=self.NY_PIX: ny_min = self.NY_PIX-1
        if ny_max<0: ny_max = 0
        if ny_max>=self.NY_PIX: ny_max = self.NY_PIX-1
        

        # Sum up all TOF bins
        a = self.executeChildAlg(Integration, input_ws, OutputWorkspace=input_ws.getName()+'_int')
        integrated_ws = a._getWorkspaceProperty("OutputWorkspace")
        
        # Find pixel with highest and lowest signal
        counts_2D = numpy.zeros([2*self.NY_TRANS_PIX+1, 2*self.NX_TRANS_PIX+1])
        
        # Smallest count in a given pixel in the region around the center
        signal_min = None
        # Highest count in a given pixel in the region around the center
        signal_max = None
        # Position of the pixel with the highest count
        xmax = xcenter
        ymax = ycenter
        
        for ix in range(nx_min, nx_max+1):
            for iy in range(ny_min, ny_max+1):
                i_pixel = self.NY_PIX*ix+iy
                signal = integrated_ws.readY(i_pixel)[0]
                
                if signal_min is None or signal < signal_min:
                    signal_min = signal
                if signal_max is None or signal > signal_max:
                    signal_max = signal
                    xmax = ix
                    ymax = iy
                        
        self.log().information("Transmission search [%d, %d] [%d, %d]" % (nx_min, nx_max, ny_min, ny_max))
                
        self.log().information("Max transmission counts at [%d, %d] = %g" % (xmax, ymax,signal_max))
        self.log().information("Min transmission counts = %g" % (signal_min))
        
        # Find transmission peak boundaries
        # The peak is defined by the area around the pixel with the highest count
        # that is above the background level.
        xpeakmax = nx_max
        for i in range(xmax+1, nx_max+1):
            i_pixel = self.NY_PIX*i+ymax
            signal = integrated_ws.readY(i_pixel)[0]
            if signal*self.transmission_peak_to_bg_ratio < signal_max:
                xpeakmax = i-1
                break
            
            i_pixel_low = self.NY_PIX*(i-1)+ymax
            signal_low = integrated_ws.readY(i_pixel_low)[0]
            if signal > signal_low:
                xpeakmax = i-1
                break
            
        xpeakmin = nx_min
        for i in range(xmax-1, nx_min+1, -1):
            i_pixel = self.NY_PIX*i+ymax
            signal = integrated_ws.readY(i_pixel)[0]
            if signal*self.transmission_peak_to_bg_ratio < signal_max:
                xpeakmin = i+1
                break
            
            i_pixel_high = self.NY_PIX*(i+1)+ymax
            signal_high = integrated_ws.readY(i_pixel_high)[0]
            if signal > signal_high:
                xpeakmin = i+1
                break
            
        ypeakmax = ny_max
        for i in range(ymax+1, ny_max+1):
            i_pixel = self.NY_PIX*xmax+i
            signal = integrated_ws.readY(i_pixel)[0]
            if signal*self.transmission_peak_to_bg_ratio < signal_max:
                ypeakmax = i-1
                break
            
            i_pixel_low = self.NY_PIX*xmax+(i-1)
            signal_low = integrated_ws.readY(i_pixel_low)[0]
            if signal > signal_low:
                ypeakmax = i-1
                break

        ypeakmin = ny_min
        for i in range(ymax-1, ny_min+1, -1):
            i_pixel = self.NY_PIX*xmax+i
            signal = integrated_ws.readY(i_pixel)[0]
            if signal*self.transmission_peak_to_bg_ratio < signal_max:
                ypeakmin = i+1
                break
            
            i_pixel_high = self.NY_PIX*xmax+(i-1)
            signal_high = integrated_ws.readY(i_pixel_high)[0]
            if signal > signal_high:
                ypeakmin = i+1
                break

        self.log().information("Peak range in X [%d, %d]" % (xpeakmin, xpeakmax))
        self.log().information("Peak range in Y [%d, %d]" % (ypeakmin, ypeakmax))

        # Mask monitor peak around the beam center
        # Question for JK: why do we do this?
        mask = []
        for ix in range(xpeakmin, xpeakmax+1):
            for iy in range(ypeakmin, ypeakmax+1):
                i_pixel = self.NY_PIX*ix+iy
                mask.append(i_pixel)
                
        for iy in range(ypeakmin, ypeakmax+1):
            for ix in range(xpeakmin, xcenter):
                i_pixel = self.NY_PIX*ix+iy
                signal = integrated_ws.readY(i_pixel)[0]
                i_pixel_high = self.NY_PIX*(ix+1)+iy
                signal_high = integrated_ws.readY(i_pixel_high)[0]
                
                if signal > signal_high and signal*self.min_transmission_peak_to_bg_ratio<signal_max: 
                    if i_pixel in mask:
                        mask.remove(i_pixel)
                else:
                    break
                
        for iy in range(ypeakmin, ypeakmax+1):
            for ix in range(xpeakmax, xcenter, -1):
                i_pixel = self.NY_PIX*ix+iy
                signal = integrated_ws.readY(i_pixel)[0]
                i_pixel_high = self.NY_PIX*(ix-1)+iy
                signal_high = integrated_ws.readY(i_pixel_high)[0]
                
                if signal > signal_high and signal*self.min_transmission_peak_to_bg_ratio<signal_max: 
                    if i_pixel in mask:
                        mask.remove(i_pixel)
                else:
                    break
                
        for iy in range(xpeakmin, xpeakmax+1):
            for ix in range(ypeakmin, ycenter):
                i_pixel = self.NY_PIX*ix+iy
                signal = integrated_ws.readY(i_pixel)[0]
                i_pixel_high = self.NY_PIX*ix+iy+1
                signal_high = integrated_ws.readY(i_pixel_high)[0]
                
                if signal > signal_high and signal*self.min_transmission_peak_to_bg_ratio<signal_max: 
                    if i_pixel in mask:
                        mask.remove(i_pixel)
                else:
                    break
                
        for iy in range(xpeakmin, xpeakmax+1):
            for ix in range(ypeakmax, ycenter, -1):
                i_pixel = self.NY_PIX*ix+iy
                signal = integrated_ws.readY(i_pixel)[0]
                i_pixel_high = self.NY_PIX*ix+iy-1
                signal_high = integrated_ws.readY(i_pixel_high)[0]
                
                if signal > signal_high and signal*self.min_transmission_peak_to_bg_ratio<signal_max: 
                    if i_pixel in mask:
                        mask.remove(i_pixel)
                else:
                    break
        
        # Sum transmission peak counts as a function of TOF
        dataX = input_ws.readX(0)
        if HAS_NUMPY and type(dataX) == numpy.ndarray:
            dataX = dataX.tolist()
        nTOF = len(dataX)
        dataY = (nTOF-1)*[0]
        dataE = (nTOF-1)*[0]
        
        transmission_counts = 0
        total_pixels = 0

        # We will normalize the transmission by the accelerator current  
        proton_charge = input_ws.getRun()["proton_charge"].getStatistics().mean
        duration = input_ws.getRun()["proton_charge"].getStatistics().duration
        frequency = input_ws.getRun()["frequency"].getStatistics().mean
        acc_current = 1.0e-12 * proton_charge * duration * frequency
        
        for itof in range(nTOF-1):
            for ix in range(xpeakmin, xpeakmax+1):
                for iy in range(ypeakmin, ypeakmax+1):
                    i_pixel = self.NY_PIX*ix+iy
                    signal = input_ws.readY(i_pixel)[itof]
                    error = input_ws.readE(i_pixel)[itof]
                    if i_pixel in mask:
                        total_pixels += 1
                        dataY[itof] += signal
                        dataE[itof] += error*error
                        transmission_counts += signal
            dataE[itof] = math.sqrt(dataE[itof])/acc_current
            dataY[itof] /= acc_current    
        
        if normalize:
            self.log().information("Normalizing the translation to average 1")
            factor = transmission_counts/acc_current/(nTOF-1)
            
            if factor > 0:
                for itof in range(nTOF-1):
                    dataY[itof] /= factor
                    dataE[itof] /= factor
            else:
                self.log().error("No count near the beam center! Could not compute transmission.")
        
        unitX = input_ws.getAxis(0).getUnit().name()
        CreateWorkspace(output_ws, dataX, dataY, dataE, NSpec=1, UnitX=unitX)
        
        total_pixels /= (nTOF-1)
        self.log().information("Total transmission counts (%d) = %g" % (total_pixels, transmission_counts))


mtd.registerPyAlgorithm(EQSANSTransmission())
