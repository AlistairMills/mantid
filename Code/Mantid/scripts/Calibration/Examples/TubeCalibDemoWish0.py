#
# TUBE CALIBRATION DEMONSTRATION PROGRAM - Execute this
#
# Here we run the calibration of WISH panel03
# We explicitly set the ideal tube with values corresponding to Y position in metres
#
from mantid.api import WorkspaceFactory  # For table worskspace of calibrations
from ideal_tube import * # for ideal tube
from tube_calib_fit_params import * # To handle fit parameters
from tube_calib import *  # For tube calibration functions
from tube_spec import * # For tube specification class
   
# Get calibration raw file and integrate it 
path = r"C:/Temp/" # Path name of folder containing input and output files
filename = 'WISH00017701.raw' # Name of calibration run   
rawMapWS = Load(path+filename)  #'raw' in 'rawMapWS' means unintegrated.
mapWS = Integration( rawMapWS, RangeLower=1, RangeUpper=20000 )

#Create Calibration Table
calibrationTable = CreateEmptyTableWorkspace(OutputWorkspace="CalibTable")
calibrationTable.addColumn(type="int",name="Detector ID")  # "Detector ID" column required by ApplyCalibration
calibrationTable.addColumn(type="V3D",name="Detector Position")  # "Detector Position" column required by ApplyCalibration

# Specify panel 03 of WISH instrument
thisTubeSet = TubeSpec(mapWS)
thisTubeSet.setTubeSpecByString('WISH/panel03')

# Give y-positions of slit points (gotten for converting first tube's slit point to Y)
iTube = IdealTube()
iTube.setArray([-0.41,-0.31,-0.21,-0.11,-0.02, 0.09, 0.18, 0.28, 0.39 ])

# Set fitting parameters
eP = [65.0, 113.0, 161.0, 209.0, 257.0, 305.0, 353.0, 401.0, 449.0]
fitPar = TubeCalibFitParams( eP, 2000, 32 )

# Get the calibration and put it into the calibration table
getCalibration( mapWS, thisTubeSet, calibrationTable, fitPar, iTube)
    
#Apply the calibration
ApplyCalibration( Workspace=mapWS, PositionTable=calibrationTable)