#ifndef MANTID_MDEVENTS_REFLECTOMETRYMDTRANFORM_H_
#define MANTID_MDEVENTS_REFLECTOMETRYMDTRANFORM_H_

#include "MantidKernel/System.h"
#include "MantidAPI/IEventWorkspace.h"
#include "MantidAPI/IMDEventWorkspace.h"

namespace Mantid
{
namespace MDEvents
{

  /** ReflectometryMDTransform : Abstract type for reflectometry transforms to MDWorkspaces
    
    @date 2012-05-29

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
  class DLLExport ReflectometryMDTransform 
  {
  public:
    virtual Mantid::API::IMDEventWorkspace_sptr execute(Mantid::API::MatrixWorkspace_const_sptr inputWs) const = 0;
  };
}
}
#endif