#include "MantidAPI/BoxController.h"

#include "MantidPythonInterface/kernel/SharedPtrToPythonMacro.h"

#include <boost/python/class.hpp>
#include <boost/python/copy_const_reference.hpp>

using Mantid::API::BoxController;
using Mantid::API::BoxController_sptr;
using namespace boost::python;

void export_BoxController()
{
  REGISTER_SHARED_PTR_TO_PYTHON(BoxController);

  class_< BoxController, boost::noncopyable >("BoxController", no_init)
    .def("getNDims", &BoxController::getNDims, "Get # of dimensions")
    .def("getSplitThreshold", &BoxController::getSplitThreshold, "Return the splitting threshold, in # of events")
    .def("getSplitInto", &BoxController::getSplitInto, "Return into how many to split along a dimension")
    .def("getMaxDepth", &BoxController::getMaxDepth, "Return the max recursion depth allowed for grid box splitting.")
    .def("getTotalNumMDBoxes", &BoxController::getTotalNumMDBoxes, "Return the total number of MD Boxes, irrespective of depth")
    .def("getTotalNumMDGridBoxes", &BoxController::getTotalNumMDGridBoxes, "Return the total number of MDGridBox'es, irrespective of depth")
    .def("getAverageDepth", &BoxController::getAverageDepth, "Return the average recursion depth of gridding.")
    .def("isFileBacked", &BoxController::isFileBacked, "Return True if the MDEventWorkspace is backed by a file ")
    .def("getFilename", &BoxController::getFilename, "Return  the full path to the file open as the file-based back or empty string if no file back-end is initiated")
    .def("useWriteBuffer", &BoxController::useWriteBuffer, "Return true if the MRU should be used")
  ;
}

