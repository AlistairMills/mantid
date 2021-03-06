##
## Tests for PythonAlgorithms
##

set ( TEST_PY_FILES
  CheckForSampleLogsTest.py
  ConjoinSpectraTest.py
  CorrectLogTimesTest.py
  CreateLeBailFitInputTest.py
  CreateWorkspaceTest.py
  CreateTransmissionWorkspaceTest.py
  CreateTransmissionWorkspaceAutoTest.py
  DakotaChiSquaredTest.py
  DSFinterpTest.py
  FilterLogByTimeTest.py
  FindReflectometryLinesTest.py
  GetEiT0atSNSTest.py
  LoadFullprofFileTest.py
  LoadLiveDataTest.py
  LoadLogPropertyTableTest.py
  LoadMultipleGSSTest.py
  MaskAngleTest.py
  MaskBTPTest.py
  MaskWorkspaceToCalFileTest.py
  MeanTest.py
  MergeCalFilesTest.py
  PDDetermineCharacterizationsTest.py
  ReflectometryReductionOneTest.py
  ReflectometryReductionOneAutoTest.py
  RetrieveRunInfoTest.py
  SANSWideAngleCorrectionTest.py
  SortDetectorsTest.py
  SortXAxisTest.py
  Stitch1DTest.py
  Stitch1DManyTest.py
  SuggestTibCNCSTest.py
  SuggestTibHYSPECTest.py
  UpdatePeakParameterTableValueTest.py
  SANSSubtractTest.py
  ExportSampleLogsToCSVFileTest.py
  ExportExperimentLogTest.py
)

check_tests_valid ( ${CMAKE_CURRENT_SOURCE_DIR} ${TEST_PY_FILES} )

# Prefix for test name=PythonAlgorithms
pyunittest_add_test ( ${CMAKE_CURRENT_SOURCE_DIR} PythonAlgorithms ${TEST_PY_FILES} )
