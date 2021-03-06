#include "MantidKernel/Unit.h"

#include <boost/python/class.hpp>
#include <boost/python/register_ptr_to_python.hpp>

using Mantid::Kernel::Unit;
using Mantid::Kernel::Unit_sptr;
using namespace boost::python;

namespace
{
  /**
   * Returns the full name of the unit & raises a deprecation warning
   * @param self A reference to calling object
   */
  const std::string deprecatedName(Unit & self)
  {
    PyErr_Warn(PyExc_DeprecationWarning, "'name' is deprecated, use 'caption' instead.");
    return self.caption();
  }
}

void export_Unit()
{
  register_ptr_to_python<boost::shared_ptr<Unit>>();

  class_<Unit,boost::noncopyable>("Unit", no_init)
    .def("name", &deprecatedName, "Return the full name of the unit (deprecated, use caption)")
    .def("caption", &Unit::caption, "Return the full name of the unit")
    .def("label", &Unit::label, "Returns a label to be printed on the axis")
    .def("unitID", &Unit::unitID, "Returns the string ID of the unit. This may/may not match its name")
  ;

}

