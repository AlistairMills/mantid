"""*WIKI* 

    HFIR SANS reduction workflow
    
*WIKI*"""
import mantid.simpleapi as api
from mantid.api import *
from mantid.kernel import *
from reduction_workflow.find_data import find_data
import os

class HFIRSANSReduction(PythonAlgorithm):

    def category(self):
        return "Workflow\\SANS;PythonAlgorithms"

    def name(self):
        return "HFIRSANSReduction"
    
    def PyInit(self):
        #TODO: allow for multiple files to be summed 
        #TODO: allow for input workspace instead of file
        self.declareProperty(FileProperty("Filename", "",
                                          action=FileAction.Load, extensions=['xml']))
        self.declareProperty("ReductionProperties", "__sans_reduction_properties", 
                             validator=StringMandatoryValidator(),
                             doc="Property manager name for the reduction")
        self.declareProperty("OutputWorkspace", "",
                             doc="Reduced workspace")
        self.declareProperty("OutputMessage", "", 
                             direction=Direction.Output, doc = "Output message")
        
    def _multiple_load(self, data_file, workspace, 
                       property_manager, property_manager_name):
        # Check whether we have a list of files that need merging
        #   Make sure we process a list of files written as a string
        def _load_data(filename, output_ws):
            if not property_manager.existsProperty("LoadAlgorithm"):
                raise RuntimeError, "SANS reduction not set up properly: missing load algorithm"
            p=property_manager.getProperty("LoadAlgorithm")
            alg=Algorithm.fromString(p.valueAsStr)
            alg.setProperty("Filename", filename)
            alg.setProperty("OutputWorkspace", output_ws)
            if alg.existsProperty("ReductionProperties"):
                alg.setProperty("ReductionProperties", property_manager_name)
            alg.execute()
            msg = "Loaded %s\n" % filename
            if alg.existsProperty("OutputMessage"):
                msg = alg.getProperty("OutputMessage").value
            return msg
            
        # Get instrument to use with FileFinder
        instrument = ''
        if property_manager.existsProperty("InstrumentName"):
            instrument = property_manager.getProperty("InstrumentName").value

        output_str = ''
        if type(data_file)==str:
            data_file = find_data(data_file, instrument=instrument, allow_multiple=True)
        if type(data_file)==list:
            monitor = 0.0
            timer = 0.0 
            for i in range(len(data_file)):
                output_str += "Loaded %s\n" % data_file[i]
                if i==0:
                    output_str += _load_data(data_file[i], workspace)
                else:
                    output_str += _load_data(data_file[i], '__tmp_wksp')
                    api.Plus(LHSWorkspace=workspace,
                         RHSWorkspace='__tmp_wksp',
                         OutputWorkspace=workspace)
                    # Get the monitor and timer values
                    ws = AnalysisDataService.retrieve('__tmp_wksp')
                    monitor += ws.getRun().getProperty("monitor").value
                    timer += ws.getRun().getProperty("timer").value
            
            # Get the monitor and timer of the first file, which haven't yet
            # been added to the total
            ws = AnalysisDataService.retrieve(workspace)
            monitor += ws.getRun().getProperty("monitor").value
            timer += ws.getRun().getProperty("timer").value
                    
            # Update the timer and monitor
            ws.getRun().addProperty("monitor", monitor, True)
            ws.getRun().addProperty("timer", timer, True)
            
            if AnalysisDataService.doesExist('__tmp_wksp'):
                AnalysisDataService.remove('__tmp_wksp')              
        else:
            output_str += "Loaded %s\n" % data_file
            output_str += _load_data(data_file, workspace)
        return output_str
        
    def PyExec(self):
        filename = self.getProperty("Filename").value
        output_ws = self.getPropertyValue("OutputWorkspace")
        #output_ws = '__'+output_ws+'_reduced'
        property_manager_name = self.getProperty("ReductionProperties").value
        property_manager = PropertyManagerDataService.retrieve(property_manager_name)
        
        property_list = [p.name for p in property_manager.getProperties()]
        
        output_msg = ""
        # Find the beam center
        if "SANSBeamFinderAlgorithm" in property_list:
            p=property_manager.getProperty("SANSBeamFinderAlgorithm")
            alg=Algorithm.fromString(p.valueAsStr)
            if alg.existsProperty("ReductionProperties"):
                alg.setProperty("ReductionProperties", property_manager_name)
            alg.execute()
            if alg.existsProperty("OutputMessage"):
                output_msg += alg.getProperty("OutputMessage").value+'\n'
        
        # Load the sample data
        msg = self._multiple_load(filename, output_ws, 
                                  property_manager, property_manager_name)
        output_msg += "Loaded %s\n" % filename
        output_msg += msg

        # Perform the main corrections on the sample data
        output_msg += self.process_data_file(output_ws)
        
        # Sample data transmission correction
        beam_center_x = None
        beam_center_y = None
        if "TransmissionBeamCenterAlgorithm" in property_list:
            # Execute the beam finding algorithm and set the beam
            # center for the transmission calculation
            p=property_manager.getProperty("TransmissionBeamCenterAlgorithm")
            alg=Algorithm.fromString(p.valueAsStr)
            if alg.existsProperty("ReductionProperties"):
                alg.setProperty("ReductionProperties", property_manager_name)
            alg.execute()
            beam_center_x = alg.getProperty("FoundBeamCenterX").value
            beam_center_y = alg.getProperty("FoundBeamCenterY").value

        if "TransmissionAlgorithm" in property_list:
            p=property_manager.getProperty("TransmissionAlgorithm")
            alg=Algorithm.fromString(p.valueAsStr)
            alg.setProperty("InputWorkspace", output_ws)
            alg.setProperty("OutputWorkspace", output_ws)
            
            if alg.existsProperty("BeamCenterX") \
                and alg.existsProperty("BeamCenterY") \
                and beam_center_x is not None \
                and beam_center_y is not None:
                alg.setProperty("BeamCenterX", beam_center_x)
                alg.setProperty("BeamCenterY", beam_center_y)            
            
            if alg.existsProperty("ReductionProperties"):
                alg.setProperty("ReductionProperties", property_manager_name)
            alg.execute()
            if alg.existsProperty("OutputMessage"):
                output_msg += alg.getProperty("OutputMessage").value+'\n'
        
        # Process background data
        if "BackgroundFiles" in property_list:
            background = property_manager.getProperty("BackgroundFiles").value
            background_ws = "__background_%s" % output_ws
            msg = self._multiple_load(background, background_ws, 
                                property_manager, property_manager_name)
            bck_msg = "Loaded background %s\n" % background
            bck_msg += msg
        
            # Process background like we processed the sample data
            bck_msg += self.process_data_file(background_ws)
            
            trans_beam_center_x = None
            trans_beam_center_y = None
            if "BckTransmissionBeamCenterAlgorithm" in property_list:
                # Execute the beam finding algorithm and set the beam
                # center for the transmission calculation
                p=property_manager.getProperty("BckTransmissionBeamCenterAlgorithm")
                alg=Algorithm.fromString(p.valueAsStr)
                if alg.existsProperty("ReductionProperties"):
                    alg.setProperty("ReductionProperties", property_manager_name)
                alg.execute()
                trans_beam_center_x = alg.getProperty("FoundBeamCenterX").value
                trans_beam_center_y = alg.getProperty("FoundBeamCenterY").value
            
            # Background transmission correction
            if "BckTransmissionAlgorithm" in property_list:
                p=property_manager.getProperty("BckTransmissionAlgorithm")
                alg=Algorithm.fromString(p.valueAsStr)
                alg.setProperty("InputWorkspace", background_ws)
                alg.setProperty("OutputWorkspace", '__'+background_ws+"_reduced")
                
                if alg.existsProperty("BeamCenterX") \
                    and alg.existsProperty("BeamCenterY") \
                    and trans_beam_center_x is not None \
                    and trans_beam_center_y is not None:
                    alg.setProperty("BeamCenterX", trans_beam_center_x)
                    alg.setProperty("BeamCenterY", trans_beam_center_y)
                    
                if alg.existsProperty("ReductionProperties"):
                    alg.setProperty("ReductionProperties", property_manager_name)
                alg.execute()
                if alg.existsProperty("OutputMessage"):
                    output_msg += alg.getProperty("OutputMessage").value+'\n'
                background_ws = '__'+background_ws+'_reduced'
        
            # Subtract background
            api.Minus(LHSWorkspace=output_ws,
                         RHSWorkspace=background_ws,
                         OutputWorkspace=output_ws)
            
            bck_msg = bck_msg.replace('\n','\n   |')
            output_msg += "Background subtracted [%s]%s\n" % (background_ws, bck_msg)
        
        # Absolute scale correction
        output_msg += self._simple_execution("AbsoluteScaleAlgorithm", output_ws)
        
        # Geometry correction
        output_msg += self._simple_execution("GeometryAlgorithm", output_ws)
        
        # Compute I(q)
        iq_output = None
        if "IQAlgorithm" in property_list:
            iq_output = self.getPropertyValue("OutputWorkspace")
            iq_output = iq_output+'_Iq'
            p=property_manager.getProperty("IQAlgorithm")
            alg=Algorithm.fromString(p.valueAsStr)
            alg.setProperty("InputWorkspace", output_ws)
            alg.setProperty("OutputWorkspace", iq_output)
            alg.setProperty("ReductionProperties", property_manager_name)
            alg.execute()
            if alg.existsProperty("OutputMessage"):
                output_msg += alg.getProperty("OutputMessage").value            

        # Compute I(qx,qy)
        iqxy_output = None
        if "IQXYAlgorithm" in property_list:
            iq_output_name = self.getPropertyValue("OutputWorkspace")
            iqxy_output = iq_output_name+'_Iqxy'
            p=property_manager.getProperty("IQXYAlgorithm")
            alg=Algorithm.fromString(p.valueAsStr)
            alg.setProperty("InputWorkspace", output_ws)
            alg.setProperty("OutputWorkspace", iq_output_name)
            if alg.existsProperty("ReductionProperties"):
                alg.setProperty("ReductionProperties", property_manager_name)
            alg.execute()
            if alg.existsProperty("OutputMessage"):
                output_msg += alg.getProperty("OutputMessage").value            
       
        # Verify output directory and save data
        if "OutputDirectory" in property_list:
            output_dir = property_manager.getProperty("OutputDirectory").value
            if len(output_dir)==0:
                output_dir = os.path.dirname(filename)
            if os.path.isdir(output_dir):
                output_msg += self._save_output(iq_output, iqxy_output, 
                                                output_dir, property_manager)
                Logger.get("HFIRSANSReduction").notice("Output saved in %s" % output_dir)
            elif len(output_dir)>0:
                msg = "Output directory doesn't exist: %s\n" % output_dir
                Logger.get("HFIRSANSReduction").error(msg)
    
        self.setProperty("OutputMessage", output_msg)
        
    def process_data_file(self, workspace):
        output_msg = ""
        property_manager_name = self.getProperty("ReductionProperties").value
        property_manager = PropertyManagerDataService.retrieve(property_manager_name)
        property_list = [p.name for p in property_manager.getProperties()]

        # Dark current subtraction
        output_msg += self._simple_execution("DarkCurrentAlgorithm", workspace)

        # Normalize
        output_msg += self._simple_execution("NormaliseAlgorithm", workspace)
        
        # Mask
        output_msg += self._simple_execution("MaskAlgorithm", workspace)
        
        # Solid angle correction
        output_msg += self._simple_execution("SANSSolidAngleCorrection", workspace)
        
        # Sensitivity correction
        if "SensitivityAlgorithm" in property_list:
            # Beam center for the sensitivity correction
            beam_center_x = None
            beam_center_y = None
            if "SensitivityBeamCenterAlgorithm" in property_list:
                # Execute the beam finding algorithm and set the beam
                # center for the transmission calculation
                p=property_manager.getProperty("SensitivityBeamCenterAlgorithm")
                alg=Algorithm.fromString(p.valueAsStr)
                if alg.existsProperty("ReductionProperties"):
                    alg.setProperty("ReductionProperties", property_manager_name)
                alg.execute()
                beam_center_x = alg.getProperty("FoundBeamCenterX").value
                beam_center_y = alg.getProperty("FoundBeamCenterY").value
            
            p=property_manager.getProperty("SensitivityAlgorithm")
            alg=Algorithm.fromString(p.valueAsStr)
            alg.setProperty("InputWorkspace", workspace)
            alg.setProperty("OutputWorkspace", workspace)
            
            if alg.existsProperty("BeamCenterX") \
                and alg.existsProperty("BeamCenterY") \
                and beam_center_x is not None \
                and beam_center_y is not None:
                alg.setProperty("BeamCenterX", beam_center_x)
                alg.setProperty("BeamCenterY", beam_center_y)
            
            if alg.existsProperty("ReductionProperties"):
                alg.setProperty("ReductionProperties", property_manager_name)
            alg.execute()
            if alg.existsProperty("OutputMessage"):
                output_msg += alg.getProperty("OutputMessage").value+'\n'

        return output_msg
    
    def _simple_execution(self, algorithm_name, workspace, output_workspace=None):
        """
            Simple execution of an algorithm on the given workspace
        """
        property_manager_name = self.getProperty("ReductionProperties").value
        property_manager = PropertyManagerDataService.retrieve(property_manager_name)
        
        output_msg = ""
        if output_workspace is None:
            output_workspace = workspace
            
        if property_manager.existsProperty(algorithm_name):
            p=property_manager.getProperty(algorithm_name)
            alg=Algorithm.fromString(p.valueAsStr)
            if alg.existsProperty("InputWorkspace"):
                alg.setProperty("InputWorkspace", workspace)
                alg.setProperty("OutputWorkspace", output_workspace)
            else:
                alg.setProperty("Workspace", workspace)
            if alg.existsProperty("ReductionProperties"):
                alg.setProperty("ReductionProperties", property_manager_name)
            alg.execute()
            if alg.existsProperty("OutputMessage"):
                output_msg = alg.getProperty("OutputMessage").value+'\n'
        return output_msg
        
    def _save_output(self, iq_output, iqxy_output, output_dir, property_manager):
        """
            Save the I(Q) and I(QxQy) output to file.
            
            @param iq_output: name of the I(Q) workspace
            @param iqxy_output: name of the I(QxQy) workspace
            @param output_dir: output director path
            @param property_manager: property manager object
        """
        output_msg = ""
        # Save I(Q)
        if iq_output is not None:
            if AnalysisDataService.doesExist(iq_output):
                proc_xml = ""
                if property_manager.existsProperty("ProcessInfo"):
                    process_file = property_manager.getProperty("ProcessInfo").value
                    if os.path.isfile(process_file):
                        proc = open(process_file, 'r')
                        proc_xml = proc.read()
                    elif len(process_file)>0:
                        Logger.get("HFIRSANSReduction").error("Could not read %s\n" % process_file)               
                
                filename = os.path.join(output_dir, iq_output+'.txt')
                
                alg = AlgorithmManager.create("SaveAscii")
                alg.initialize()
                alg.setChild(True)
                alg.setProperty("Filename", filename)
                alg.setProperty("InputWorkspace", iq_output)
                alg.setProperty("Separator", "Tab")
                alg.setProperty("CommentIndicator", "# ")
                alg.setProperty("WriteXError", True)
                alg.execute()
                
                filename = os.path.join(output_dir, iq_output+'.xml')
                alg = AlgorithmManager.create("SaveCanSAS1D")
                alg.initialize()
                alg.setChild(True)
                alg.setProperty("Filename", filename)
                alg.setProperty("InputWorkspace", iq_output)
                alg.setProperty("Process", proc_xml)
                alg.execute()

                output_msg += "I(Q) saved in %s\n" % (filename)
            else:
                Logger.get("HFIRSANSReduction").error("No I(Q) output found")
        
        # Save I(Qx,Qy)
        if iqxy_output is not None:
            if AnalysisDataService.doesExist(iqxy_output):
                filename = os.path.join(output_dir, iqxy_output+'.dat')
                alg = AlgorithmManager.create("SaveNISTDAT")
                alg.initialize()
                alg.setChild(True)
                alg.setProperty("Filename", filename)
                alg.setProperty("InputWorkspace", iqxy_output)
                alg.execute()
                #api.SaveNISTDAT(InputWorkspace=iqxy_output, Filename=filename)  
                output_msg += "I(Qx,Qy) saved in %s\n" % (filename)
            else:
                Logger.get("HFIRSANSReduction").error("No I(Qx,Qy) output found")

        return output_msg
#############################################################################################

registerAlgorithm(HFIRSANSReduction)