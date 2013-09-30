#include "MantidAPI/Workspace.h"
#include "MantidAPI/WorkspaceListProperty.h"
#include "MantidAPI/IMDWorkspace.h"
#include "MantidAPI/IEventWorkspace.h"
#include "MantidAPI/IMDEventWorkspace.h"
#include "MantidAPI/ITableWorkspace.h"
#include "MantidAPI/ISplittersWorkspace.h"

namespace Mantid
{
namespace API
{
///@cond TEMPLATE
template MANTID_API_DLL class Mantid::API::WorkspaceListProperty<Mantid::API::Workspace>;
template MANTID_API_DLL class Mantid::API::WorkspaceListProperty<Mantid::API::IEventWorkspace>;
template MANTID_API_DLL class Mantid::API::WorkspaceListProperty<Mantid::API::IMDEventWorkspace>;
template MANTID_API_DLL class Mantid::API::WorkspaceListProperty<Mantid::API::IMDWorkspace>;
template MANTID_API_DLL class Mantid::API::WorkspaceListProperty<Mantid::API::MatrixWorkspace>;
template MANTID_API_DLL class Mantid::API::WorkspaceListProperty<Mantid::API::ITableWorkspace>;
template MANTID_API_DLL class Mantid::API::WorkspaceListProperty<Mantid::API::ISplittersWorkspace>;
///@endcond TEMPLATE
} // namespace API
} // namespace Mantid
