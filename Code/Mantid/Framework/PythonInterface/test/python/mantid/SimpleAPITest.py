import unittest
from mantid.api import AlgorithmFactory, mtd, ITableWorkspace
import mantid.simpleapi as simpleapi

import os
import sys

#======================================================================================================================
# Helper class for test
class TemporaryPythonAlgorithm(object):
    """
    Dumps the given code to a file in the Python algorithm directory
    an removes the file in the del method
    """
    def __init__(self, name, code):
        from mantid import config
        
        plugin_dirs = config['python.plugins.directories'].split(";")
        if len(plugin_dirs) == 0:
            raise RuntimeError("No Python algorithm directories defined")
        
        self._pyfile = os.path.join(plugin_dirs[0], name + ".py")
        alg_file = open(self._pyfile, "w")
        alg_file.write(code)
        alg_file.close()
        
    def __del__(self):
        try:
            os.remove(self._pyfile)
            pycfile = self._pyfile.replace(".py",".pyc")
            os.remove(pycfile)
        except OSError:
            pass

#======================================================================================================================

class SimpleAPITest(unittest.TestCase):

    def tearDown(self):
        mtd.clear()
        
    def test_version_number_equals_2(self):
        self.assertEquals(simpleapi.apiVersion(), 2)
    
    def test_module_dict_seems_to_be_correct_size(self):
        # Check that the module has at least the same number
        # of attributes as unique algorithms
        module_dict = dir(simpleapi)
        all_algs = AlgorithmFactory.getRegisteredAlgorithms(True)
        self.assertTrue( len(module_dict) > len(all_algs) )
        
    def test_alg_has_expected_doc_string(self):
        # Test auto generated string, Load is manually written
        expected_doc = \
        """Rebins data with new X bin boundaries. For EventWorkspaces, you can very quickly rebin in-place by keeping the same output name and PreserveEvents=true.

Property descriptions: 

InputWorkspace(Input:req) *MatrixWorkspace*       Workspace containing the input data

OutputWorkspace(Output:req) *MatrixWorkspace*       The name to give the output workspace

Params(Input:req) *dbl list*       A comma separated list of first bin boundary, width, last bin boundary. Optionally this can be followed by a comma and more widths and last boundary pairs. Optionally this can also be a single number, which is the bin width. In this case, the boundary of binning will be determined by minimum and maximum TOF values among all events, or previous binning boundary, in case of event Workspace, or non-event Workspace, respectively. Negative width values indicate logarithmic binning. 

PreserveEvents(Input) *boolean*       Keep the output workspace as an EventWorkspace, if the input has events (default). If the input and output EventWorkspace names are the same, only the X bins are set, which is very quick. If false, then the workspace gets converted to a Workspace2D histogram.
"""
        doc = simpleapi.rebin.__doc__
        self.assertTrue(len(doc) > 0 )
        self.assertEquals(doc, expected_doc)
        
    def test_function_call_executes_correct_algorithm_when_passed_correct_args(self):
        wsname = 'test_function_call_executes_correct_algorithm_when_passed_correct_args'
        data = [1.0,2.0,3.0,4.0,5.0]
        simpleapi.CreateWorkspace(data,data,OutputWorkspace=wsname,NSpec=1,UnitX='Wavelength')
        self.assertTrue( wsname in mtd )
        
    def test_function_call_executes_with_output_workspace_on_lhs(self):
        data = [1.0,2.0,3.0,4.0,5.0]
        wavelength = simpleapi.CreateWorkspace(data,data,NSpec=1,UnitX='Wavelength')
        wsname = 'wavelength'
        self.assertTrue( wsname in mtd )
        
    def test_function_call_executes_when_algorithm_has_only_inout_workspace_props(self):
        data = [1.0,2.0,3.0,4.0,5.0, 6.0]
        wavelength = simpleapi.CreateWorkspace(data,data,NSpec=3,UnitX='Wavelength')
        simpleapi.MaskDetectors(wavelength,WorkspaceIndexList=[1,2])
        
    def test_function_accepts_EnableLogging_keyword(self):
        # The test here is that the algorithm runs without falling over about the EnableLogging keyword being a property
        wsname = 'test_function_call_executes_correct_algorithm_when_passed_correct_args'
        data = [1.0,2.0,3.0,4.0,5.0]
        simpleapi.CreateWorkspace(data,data,OutputWorkspace=wsname,NSpec=1,UnitX='Wavelength',EnableLogging=False)
        self.assertTrue( wsname in mtd )
    
    def test_function_call_raises_ValueError_when_passed_args_with_invalid_values(self):
        # lhs code bug means we can't do this "self.assertRaises(simpleapi.LoadNexus, 'DoesNotExist')" --> ticket #4186
        try:
            simpleapi.LoadNexus(Filename='DoesNotExist.nxs')
            self.fail("A ValueError was not thrown")
        except ValueError:
            pass
        
    def test_function_call_raises_RuntimeError_when_passed_incorrect_args(self):
        try:
            simpleapi.LoadNexus(NotAProperty=1)
            self.fail("A RuntimeError was not thrown")
        except RuntimeError:
            pass

    def test_function_call_raises_RuntimeError_if_num_of_ret_vals_doesnt_match_num_assigned_vars(self):
        try:
            ws, ws2 = simpleapi.CreateWorkspace([1.5],[1.5],NSpec=1,UnitX='Wavelength')
        except RuntimeError, exc:
            # Check the error is correct and it's not some random runtime error
            if 'CreateWorkspace is trying to return 1 output(s) but you have provided 2 variable(s). These numbers must match.' == str(exc):
                pass
            else:
                self.fail("Exception was raised but it did not have the correct message: '%s'" % str(exc))
        
    def _do_exec_time_props_test(self, runner):
        try:
            data, monitors = runner('IRS21360.raw', LoadMonitors='Separate')
        except Exception, exc:
            self.fail("An error occurred when returning outputs declared at algorithm execution: '%s'" % str(exc))
        
    def test_function_returns_correct_args_when_extra_output_props_are_added_at_execute_time(self):
        self._do_exec_time_props_test(simpleapi.LoadRaw)
        
    def test_function_uses_OutputWorkspace_keyword_over_lhs_var_name_if_provided(self):
        wsname = 'test_function_uses_OutputWorkspace_keyword_over_lhs_var_name_if_provided'
        data = [1.0,2.0,3.0,4.0,5.0]
        wkspace = simpleapi.CreateWorkspace(data,data,OutputWorkspace=wsname,NSpec=1,UnitX='Wavelength')
        self.assertTrue( wsname in mtd )
    
    def test_function_returns_only_mandatory_workspace_when_optional_output_is_not_given(self):
        _demo = simpleapi.CreateMDHistoWorkspace(SignalInput='1,2,3,4,5',ErrorInput='1,1,1,1,1',
                                                Dimensionality='1',Extents='-1,1',NumberOfBins='5',Names='A',Units='U')
        wsname = 'test_function_returns_only_mandatory_workspace_when_optional_output_is_not_given'
        query = simpleapi.QueryMDWorkspace(InputWorkspace=_demo,OutputWorkspace=wsname,MaximumRows='500',
                                           Normalisation='volume')

        self.assertTrue( isinstance(query, ITableWorkspace) )
        self.assertTrue( wsname in mtd )

    def test_function_returns_both_mandatory_and_optional_workspaces_when_optional_output_is_given(self):
        _demo = simpleapi.CreateMDWorkspace(Dimensions='2',EventType='MDEvent',Extents='1,10,1,10',Names='a,b',
                                            Units='MomentumTransfer,MomentumTransfer',SplitInto='4')
        wsname = 'test_function_returns_only_mandatory_workspace_when_optional_output_is_not_given'
        wsname_box = wsname + '_box'
        query = simpleapi.QueryMDWorkspace(InputWorkspace=_demo,OutputWorkspace=wsname,MaximumRows='500',
                                           Normalisation='volume',BoxDataTable=wsname_box)
        
        self.assertTrue( wsname in mtd )
        self.assertTrue( wsname_box in mtd )
        
        self.assertTrue( type(query) == tuple )
        self.assertEquals( 2, len(query) )
        
        self.assertTrue( isinstance(query[0], ITableWorkspace) )
        self.assertTrue( isinstance(query[1], ITableWorkspace) )
        
        
    def test_that_dialog_call_raises_runtime_error(self):
        try:
            simpleapi.LoadEventNexusDialog()
        except RuntimeError, exc:
            msg = str(exc)
            if msg != "Can only display properties dialog in gui mode":
                self.fail("Dialog function raised the correct exception type but the message was wrong")
                
    def test_call_inside_function_uses_new_variable_name(self):
        def convert(workspace):
            # Should replace the input workspace
            workspace = simpleapi.ConvertUnits(workspace, Target='Energy')
            return workspace
        
        raw = simpleapi.LoadRaw('IRS21360.raw',SpectrumMax=1)
        raw = convert(raw)
        # If this fails then the function above chose the name of the variable
        # over the actual object name
        self.assertTrue('workspace' in mtd)
        self.assertTrue('raw' in mtd)

    def test_python_alg_can_use_other_python_alg_through_simple_api(self):
        """
        Runs a test in a separate process as it requires a reload of the
        whole mantid module 
        """
        src = """
from mantid.api import PythonAlgorithm, AlgorithmFactory
import mantid.simpleapi as api
from mantid.simpleapi import *

class %(name)s(PythonAlgorithm):

    def PyInit(self):
        pass
    def PyExec(self):
        %(execline1)s
        %(execline2)s
        
AlgorithmFactory.subscribe(%(name)s)
"""
        name1 = "SimpleAPIPythonAlgorithm1"
        name2 = "SimpleAPIPythonAlgorithm2"
        src1 = src % {"name":name1,"execline1":name2+"()","execline2":"api."+name2+"()"}
        src2 = src % {"name":name2,"execline1":"pass","execline2":"pass"}
        a = TemporaryPythonAlgorithm(name1,src1)
        b = TemporaryPythonAlgorithm(name2,src2)
        import subprocess
        # Try to use algorithm 1 to run algorithm 2
        cmd = sys.executable + ' -c "from mantid.simpleapi import %(name)s;%(name)s()"' % {'name':name1}
        try:
            subprocess.check_call(cmd,shell=True)
        except subprocess.CalledProcessError, exc:
            self.fail("Error occurred running one Python algorithm from another: %s" % str(exc))
        
        # Ensure the files are removed promptly
        del a,b

if __name__ == '__main__':
    unittest.main()
