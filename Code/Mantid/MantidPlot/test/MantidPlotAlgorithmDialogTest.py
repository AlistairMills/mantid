""" 
Test generation of Algorithm Dialogs
"""
import mantidplottests
from mantidplottests import *
import mantidqtpython
import sys
import tempfile

class MantidPlotAlgorithmDialogTest(unittest.TestCase):
    
    __target_algorithm__ = "CreateMDWorkspace"
    __clean_properties__ = True
        
    def test_OpenDialog(self):
        interface_manager = mantidqtpython.MantidQt.API.InterfaceManager()
        dialog = threadsafe_call( interface_manager.createDialogFromName, self.__target_algorithm__, self.__clean_properties__)
        is_instance_of_alg_dialog = isinstance(dialog, mantidqtpython.MantidQt.API.AlgorithmDialog)
        self.assertEqual(is_instance_of_alg_dialog, True, "Did not get an AlgorithmDialog instance back.")
        threadsafe_call( dialog.close )
        
    def test_ScreenShotDialog(self):
        interface_manager = mantidqtpython.MantidQt.API.InterfaceManager()
        dialog = threadsafe_call( interface_manager.createDialogFromName, self.__target_algorithm__, self.__clean_properties__)
        screenshotdir = tempfile.gettempdir();
        file = "CreateMDWorkspace_screenshot"
        screenshot_to_dir(widget=dialog, filename=file, screenshot_dir=screenshotdir)
        threadsafe_call(dialog.close)
        file_abs = os.path.join(screenshotdir, file + ".png")
        file_exists = os.path.isfile(file_abs)
        self.assertEquals(file_exists, True, "Screenshot was not written out as expected.")
        if file_exists:
            os.remove(file_abs)
        
# Run the unit tests
mantidplottests.runTests(MantidPlotAlgorithmDialogTest) 
    
    
    