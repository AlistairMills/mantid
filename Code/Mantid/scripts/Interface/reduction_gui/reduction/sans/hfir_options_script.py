"""
    Classes for each reduction step. Those are kept separately 
    from the the interface class so that the HFIRReduction class could 
    be used independently of the interface implementation
"""
import xml.dom.minidom
import copy
import os
from reduction_gui.reduction.scripter import BaseScriptElement

class ReductionOptions(BaseScriptElement):
    instrument_name = "BIOSANS"
    nx_pixels = 192
    ny_pixels = 192
    pixel_size = 5.1
    
    # Absoulte scale
    scaling_factor = 1.0
    calculate_scale = False
    scaling_direct_file = ''
    scaling_att_trans = 1.0
    scaling_beam_diam = 25.0 
    manual_beam_diam = False
    
    # Sample-detector distance to force on the data set [mm]
    sample_detector_distance = 0.0
    # Detector distance offset [mm]
    detector_offset = 837.9
    # Wavelength value to force on the data set [Angstrom]
    wavelength = 0.0
    wavelength_spread = 0.1
    
    # Flag to perform the solid angle correction
    solid_angle_corr = True
    # Dark current
    dark_current_corr = False
    dark_current_data = ''
    
    # Q range
    n_q_bins = 100
    n_sub_pix = 1
    log_binning = False
    
    # Masking
    class RectangleMask(object):
        def __init__(self, x_min=0, x_max=0, y_min=0, y_max=0):
            self.x_min = x_min
            self.x_max = x_max
            self.y_min = y_min
            self.y_max = y_max
        
    # Masked edges
    top = 0
    bottom = 0
    left = 0
    right = 0
    
    # Masked shapes
    shapes = []
    
    # Masked detector IDs
    detector_ids = []
    
    # Mask file
    mask_file = ''
    use_mask_file = False
    
    # Output directory
    use_data_directory = True
    output_directory = ''
    
    NORMALIZATION_NONE = 0
    NORMALIZATION_TIME = 1
    NORMALIZATION_MONITOR = 2
    normalization = NORMALIZATION_MONITOR
        
    def __init__(self):
        super(ReductionOptions, self).__init__()
        self.reset()
        
    def to_script(self):
        """
            Generate reduction script
        """
        script  = "%s()\n" % self.instrument_name
        
        if self.sample_detector_distance != 0:
            script += "SetSampleDetectorDistance(%g)\n" % self.sample_detector_distance 
        if self.detector_offset != 0:
            script += "SetSampleDetectorOffset(%g)\n" % self.detector_offset 
        if self.wavelength != 0:
            script += "SetWavelength(%g, %g)\n" % (self.wavelength, self.wavelength_spread)
        
        if self.solid_angle_corr:
            script += "SolidAngle(detector_tubes=True)\n"
        else:
            script += "NoSolidAngle()\n"
        
        if self.dark_current_corr:
            if len(str(self.dark_current_data).strip())==0:
                raise RuntimeError, "Dark current subtraction was selected but no sensitivity data file was entered." 
            script += "DarkCurrent(\"%s\")\n" % self.dark_current_data
            
        script += self._normalization_options()
        
        if self.calculate_scale:
            scaling_params = ""
            if self.manual_beam_diam:
                scaling_params += ", beamstop_diameter=%g" % self.scaling_beam_diam
            
            script += "SetDirectBeamAbsoluteScale(\"%s\", attenuator_trans=%g%s)\n" % \
             (self.scaling_direct_file, self.scaling_att_trans, scaling_params)
        else:
            script += "SetAbsoluteScale(%g)\n" % self.scaling_factor
                
        # Q binning
        script += "AzimuthalAverage(n_bins=%g, n_subpix=%g, log_binning=%s)\n" % (self.n_q_bins, self.n_sub_pix, str(self.log_binning))        
        script += "IQxQy(nbins=%g)\n" % self.n_q_bins
        
        # Mask 
        #   Edges
        if (self.top != 0 or self.bottom != 0 or self.left != 0 or self.right != 0):
            script += "Mask(nx_low=%d, nx_high=%d, ny_low=%d, ny_high=%d)\n" % (self.left, self.right, self.bottom, self.top)
        #   Rectangles
        for item in self.shapes:
            script += "MaskRectangle(x_min=%g, x_max=%g, y_min=%g, y_max=%g)\n" % (item.x_min, item.x_max, item.y_min, item.y_max)
        #   Detector IDs
        if len(self.detector_ids)>0 and self.use_mask_file:
            script += "MaskDetectors(%s)\n" % str(self.detector_ids)
        # Output directory
        if not self.use_data_directory:
            script += "OutputPath(\"%s\")\n" % self.output_directory
        
        return script           
    
    def _normalization_options(self):
        """
            Generate the normalization portion of the reduction script
        """
        if self.normalization==ReductionOptions.NORMALIZATION_NONE:
            return "NoNormalization()\n"
        elif self.normalization==ReductionOptions.NORMALIZATION_TIME:
            return "TimeNormalization()\n"
        elif self.normalization==ReductionOptions.NORMALIZATION_MONITOR:
            return "MonitorNormalization()\n"
        return ""        
        
    def to_xml(self):
        """
            Create XML from the current data.
        """
        xml  = "<Instrument>\n"
        xml += "  <name>%s</name>\n" % self.instrument_name
        if self.nx_pixels is not None:
            xml += "  <nx_pixels>%g</nx_pixels>\n" % self.nx_pixels
        if self.ny_pixels is not None:
            xml += "  <ny_pixels>%g</ny_pixels>\n" % self.ny_pixels
        if self.pixel_size is not None:
            xml += "  <pixel_size>%g</pixel_size>\n" % self.pixel_size
        
        xml += "  <sample_det_dist>%g</sample_det_dist>\n" % self.sample_detector_distance
        xml += "  <detector_offset>%g</detector_offset>\n" % self.detector_offset
        xml += "  <wavelength>%g</wavelength>\n" % self.wavelength
        xml += "  <wavelength_spread>%g</wavelength_spread>\n" % self.wavelength_spread
        
        xml += "  <solid_angle_corr>%s</solid_angle_corr>\n" % str(self.solid_angle_corr)
        xml += "  <dark_current_corr>%s</dark_current_corr>\n" % str(self.dark_current_corr)
        xml += "  <dark_current_data>%s</dark_current_data>\n" % self.dark_current_data

        xml += "  <n_q_bins>%g</n_q_bins>\n" % self.n_q_bins
        xml += "  <n_sub_pix>%g</n_sub_pix>\n" % self.n_sub_pix
        xml += "  <log_binning>%s</log_binning>\n" % str(self.log_binning)

        xml += "  <normalization>%d</normalization>\n" % self.normalization
 
        # Output directory
        xml += "  <UseDataDirectory>%s</UseDataDirectory>\n" % str(self.use_data_directory)
        xml += "  <OutputDirectory>%s</OutputDirectory>\n" % self.output_directory
        
        xml += "</Instrument>\n"
        
        xml += "<Mask>\n"
        xml += "  <mask_top>%g</mask_top>\n" % self.top
        xml += "  <mask_bottom>%g</mask_bottom>\n" % self.bottom
        xml += "  <mask_left>%g</mask_left>\n" % self.left
        xml += "  <mask_right>%g</mask_right>\n" % self.right
        
        xml += "  <Shapes>\n"
        for item in self.shapes:
            xml += "    <rect x_min='%g' x_max='%g' y_min='%g' y_max='%g' />\n" % (item.x_min, item.x_max, item.y_min, item.y_max)
        xml += "  </Shapes>\n"
        
        ids_str = ','.join(map(str, self.detector_ids))
        xml += "  <DetectorIDs>%s</DetectorIDs>\n" % ids_str
        xml += "  <mask_file>%s</mask_file>\n" % self.mask_file
        xml += "  <use_mask_file>%s</use_mask_file>\n" % self.use_mask_file
        
        xml += "</Mask>\n"
        
        xml += "<AbsScale>\n"
        xml += "  <manual_beam_diam>%s</manual_beam_diam>\n" % str(self.manual_beam_diam)
        xml += "  <scaling_factor>%g</scaling_factor>\n" % self.scaling_factor
        xml += "  <calculate_scale>%s</calculate_scale>\n" % str(self.calculate_scale)
        xml += "  <scaling_direct_file>%s</scaling_direct_file>\n" % self.scaling_direct_file
        xml += "  <scaling_att_trans>%g</scaling_att_trans>\n" % self.scaling_att_trans
        xml += "  <scaling_beam_diam>%g</scaling_beam_diam>\n" % self.scaling_beam_diam
        xml += "</AbsScale>\n"

        return xml
        
    def from_xml(self, xml_str):
        """
            Read in data from XML
            @param xml_str: text to read the data from
        """    
        self.reset()   
        dom = xml.dom.minidom.parseString(xml_str)
        
        # Get Mantid version
        mtd_version = BaseScriptElement.getMantidBuildVersion(dom)
        
        instrument_dom = dom.getElementsByTagName("Instrument")[0]
        self.nx_pixels = BaseScriptElement.getIntElement(instrument_dom, "nx_pixels",
                                                         default=ReductionOptions.nx_pixels) 
        self.ny_pixels = BaseScriptElement.getIntElement(instrument_dom, "ny_pixels",
                                                         default=ReductionOptions.ny_pixels) 
        self.instrument_name = BaseScriptElement.getStringElement(instrument_dom, "name")
        self.pixel_size = BaseScriptElement.getFloatElement(instrument_dom, "pixel_size",
                                                            default=ReductionOptions.pixel_size)
        
        self.sample_detector_distance = BaseScriptElement.getFloatElement(instrument_dom, "sample_det_dist", 
                                                                          default=ReductionOptions.sample_detector_distance)
        self.detector_offset = BaseScriptElement.getFloatElement(instrument_dom, "detector_offset",
                                                                 default=ReductionOptions.detector_offset)
        self.wavelength = BaseScriptElement.getFloatElement(instrument_dom, "wavelength",
                                                            default=ReductionOptions.wavelength)
        self.wavelength_spread = BaseScriptElement.getFloatElement(instrument_dom, "wavelength_spread",
                                                            default=ReductionOptions.wavelength_spread)
        
        self.solid_angle_corr = BaseScriptElement.getBoolElement(instrument_dom, "solid_angle_corr",
                                                                 default = ReductionOptions.solid_angle_corr)
        
        # Output directory
        self.use_data_directory = BaseScriptElement.getBoolElement(instrument_dom, "UseDataDirectory",
                                                                   default = ReductionOptions.use_data_directory) 
        self.output_directory = BaseScriptElement.getStringElement(instrument_dom, "OutputDirectory",
                                                                   default = ReductionOptions.output_directory)
        
        # Dark current
        self.dark_current_corr = BaseScriptElement.getBoolElement(instrument_dom, "dark_current_corr",
                                                                  default = ReductionOptions.dark_current_corr)
        self.dark_current_data = BaseScriptElement.getStringElement(instrument_dom, "dark_current_data")
                
        self.n_q_bins = BaseScriptElement.getIntElement(instrument_dom, "n_q_bins",
                                                       default=ReductionOptions.n_q_bins)
        self.n_sub_pix = BaseScriptElement.getIntElement(instrument_dom, "n_sub_pix",
                                                       default=ReductionOptions.n_sub_pix)
        self.log_binning = BaseScriptElement.getBoolElement(instrument_dom, "log_binning",
                                                                 default = ReductionOptions.log_binning)
        
        self.normalization = BaseScriptElement.getIntElement(instrument_dom, "normalization",
                                                             default=ReductionOptions.normalization)

        # Mask
        element_list = dom.getElementsByTagName("Mask")
        if len(element_list)>0: 
            mask_dom = element_list[0]
            self.top = BaseScriptElement.getIntElement(mask_dom, "mask_top", default=ReductionOptions.top)
            self.bottom = BaseScriptElement.getIntElement(mask_dom, "mask_bottom", default=ReductionOptions.bottom)
            self.right = BaseScriptElement.getIntElement(mask_dom, "mask_right", default=ReductionOptions.right)
            self.left = BaseScriptElement.getIntElement(mask_dom, "mask_left", default=ReductionOptions.left)
            
            self.shapes = []
            shapes_dom_list = mask_dom.getElementsByTagName("Shapes")
            if len(shapes_dom_list)>0:
                shapes_dom = shapes_dom_list[0]
                for item in shapes_dom.getElementsByTagName("rect"):
                    x_min =  float(item.getAttribute("x_min"))
                    x_max =  float(item.getAttribute("x_max"))
                    y_min =  float(item.getAttribute("y_min"))
                    y_max =  float(item.getAttribute("y_max"))
                    self.shapes.append(ReductionOptions.RectangleMask(x_min, x_max, y_min, y_max))
                            
            self.detector_ids = BaseScriptElement.getIntList(mask_dom, "DetectorIDs", default=[])
            self.mask_file = BaseScriptElement.getStringElement(mask_dom, "mask_file")
            self.use_mask_file = BaseScriptElement.getBoolElement(mask_dom, "use_mask_file",
                                                                  default = ReductionOptions.use_mask_file)

        # Absolute scaling
        element_list = dom.getElementsByTagName("AbsScale")
        if len(element_list)>0: 
            scale_dom = element_list[0]

            self.manual_beam_diam = BaseScriptElement.getBoolElement(scale_dom, "manual_beam_diam",
                                                                     default = ReductionOptions.manual_beam_diam)
            self.scaling_factor = BaseScriptElement.getFloatElement(scale_dom, "scaling_factor", 
                                                                    default=ReductionOptions.scaling_factor)
            self.calculate_scale = BaseScriptElement.getBoolElement(scale_dom, "calculate_scale",
                                                                    default = ReductionOptions.calculate_scale)
            self.scaling_direct_file = BaseScriptElement.getStringElement(scale_dom, "scaling_direct_file")
            self.scaling_att_trans = BaseScriptElement.getFloatElement(scale_dom, "scaling_att_trans",
                                                                       default=ReductionOptions.scaling_att_trans)
            self.scaling_beam_diam = BaseScriptElement.getFloatElement(scale_dom, "scaling_beam_diam",
                                                                       default=ReductionOptions.scaling_beam_diam)

    def reset(self):
        """
            Reset state
        """
        self.nx_pixels = ReductionOptions.nx_pixels
        self.ny_pixels = ReductionOptions.ny_pixels
        #self.instrument_name = ''
        self.pixel_size = ReductionOptions.pixel_size
        
        self.scaling_factor = ReductionOptions.scaling_factor
        self.scaling_direct_file = ''
        self.calculate_scale = ReductionOptions.calculate_scale
        self.scaling_att_trans = ReductionOptions.scaling_att_trans
        self.scaling_beam_diam = ReductionOptions.scaling_beam_diam
        self.manual_beam_diam = ReductionOptions.manual_beam_diam
        
        self.sample_detector_distance = ReductionOptions.sample_detector_distance
        self.detector_offset = ReductionOptions.detector_offset
        self.wavelength = ReductionOptions.wavelength
        self.wavelength_spread = ReductionOptions.wavelength_spread
        
        self.solid_angle_corr = ReductionOptions.solid_angle_corr
        self.dark_current_corr = ReductionOptions.dark_current_corr
        self.dark_current_data = ''
        
        self.n_q_bins = ReductionOptions.n_q_bins
        self.n_sub_pix = ReductionOptions.n_sub_pix
        self.log_binning = ReductionOptions.log_binning
        
        self.normalization = ReductionOptions.normalization
        
        self.top = ReductionOptions.top
        self.bottom = ReductionOptions.bottom
        self.left = ReductionOptions.left
        self.right = ReductionOptions.right
        
        self.shapes = []
        self.detector_ids = []
        self.mask_file = ''
        self.use_mask_file = ReductionOptions.use_mask_file

        self.use_data_directory = ReductionOptions.use_data_directory
        self.output_directory = ReductionOptions.output_directory

