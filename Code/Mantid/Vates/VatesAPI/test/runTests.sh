#!/bin/bash
# Simple script to build and run the tests.
# Will run all tests in the directory if no arguments are supplied,
#      or alternatively just the test files given as arguments.
#
# You will need to have the directories containing the Mantid
#      .so libraries in your LD_LIBRARY_PATH environment variable
#
# Author: Owen Arnold 02/12/2010
#

# Clean up any old executable
rm -rf runner.*

echo "Generating the source file from the test header files..."
# Chaining all tests together can have effects that you don't think of
#  - it's always a good idea to run your new/changed test on its own
test_files=""
if [ $# -eq 0 ]; then
    test_files=*.h
else
    test_files=$*
fi

cxxtestgen=../../../../Third_Party/src/cxxtest/cxxtestgen.py
python $cxxtestgen --runner=MantidPrinter -o runner.cpp $test_files


echo "Compiling the test executable..."

#TODO set path properly
mantid_libpath=~/workspace/MantidDebug/bin
#TODO set path properly
vtk_libpath=/usr/local/2.1.1/linux-x86_64/lib
#TODO set path properly
gmock_libpath=../../../TestingTools/lib/ubuntu-10.10

g++ -O0 -g3 -DBOOST_DATE_TIME_POSIX_TIME_STD_CONFIG  -o runner.exe runner.cpp -I../../../Framework/Kernel/inc  -I../../../Framework/MDAlgorithms/inc -I../../../Framework/API/inc -I../inc/VisitPlugins -I/usr/local/2.1.1/linux-x86_64/include/vtk/include/vtk-5.0 -I../../../Framework/Geometry/inc -I ../inc \
    -I ../../../../Third_Party/src/cxxtest -I ../../../../Third_Party/include -I ../../../TestingTools/include -L$vtk_libpath -L$mantid_libpath -L$gmock_libpath -lvtkCommon -lvtkFiltering -lMantidKernel -lMantidGeometry -lMantidAPI -lboost_date_time-mt -lgmock -lMantidMDAlgorithms -lMantidDataObjects  -lMantidVisitPresenters -lhdf5 -Wno-deprecated 

echo

echo "Running the tests..."
ln ../../../Framework/Build/Tests/*.properties .
LD_LIBRARY_PATH=$vtk_libpath:$mantid_libpath:$LD_LIBRARY_PATH  ./runner.exe
#valgrind --leak-check=full --show-reachable=yes --track-origins=yes ~/mantid/Code/Vates/VisitPresenters/test/runner.exe
echo

# Remove the generated files to ensure that they're not inadvertently run
#   when something in the chain has failed.
echo "Cleaning up..."
rm -f *.properties
rm -f *Test.log
rm -f *.so
echo "Done."
