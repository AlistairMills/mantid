#ifndef MANTID_PYTHONINTERFACE_UPCASTREGISTRY_H_
#define MANTID_PYTHONINTERFACE_UPCASTREGISTRY_H_
/**
    Copyright &copy; 2012 ISIS Rutherford Appleton Laboratory & NScD Oak Ridge National Laboratory

    This file is part of Mantid.

    Mantid is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    Mantid is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    File change history is stored at: <https://svn.mantidproject.org/mantid/trunk/Code/Mantid>
    Code Documentation is available at: <http://doxygen.mantidproject.org>
*/
#include "MantidKernel/System.h"
#include <string>
#include <boost/python/object.hpp>

namespace Mantid
{
  namespace PythonInterface
  {
    namespace Registry
    {
      /// Returns an upcasting converter
      DLLExport void registerIDForUpcasting(const std::string & id, const PyTypeObject * type);
      /// Get an upcasted type object for the given object
      DLLExport const PyTypeObject * getDerivedType(boost::python::object value);
      /// Overload. Get an upcasted type object for the given object
      DLLExport const PyTypeObject * getDerivedType(PyObject *value);

    }
  }
}




#endif /* MANTID_PYTHONINTERFACE_UPCASTREGISTRY_H_ */