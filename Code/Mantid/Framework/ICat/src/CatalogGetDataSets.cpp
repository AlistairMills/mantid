/*WIKI*
This algorithm retrieves the datasets associated to the selected investigation
from the information catalog and saves the search results to mantid workspace.
*WIKI*/

#include "MantidICat/CatalogGetDataSets.h"
#include "MantidKernel/MandatoryValidator.h"
#include "MantidAPI/CatalogManager.h"
#include "MantidAPI/WorkspaceProperty.h"

namespace Mantid
{
  namespace ICat
  {
    DECLARE_ALGORITHM(CatalogGetDataSets)

    /// Sets documentation strings for this algorithm
    void CatalogGetDataSets::initDocs()
    {
      this->setWikiSummary("Gets the datasets associated to the selected investigation. ");
      this->setOptionalMessage("Gets the datasets associated to the selected investigation.");
    }

    /// Initialisation methods
    void CatalogGetDataSets::init()
    {
      declareProperty("InvestigationId","",boost::make_shared<Kernel::MandatoryValidator<std::string>>(),
          "ID of the selected investigation");
      declareProperty("Session","","The session information of the catalog to use.");
      declareProperty(new API::WorkspaceProperty<API::ITableWorkspace> ("OutputWorkspace", "", Kernel::Direction::Output),
          "The name of the workspace to store the result of datasets search ");
    }

    /// exec methods
    void CatalogGetDataSets::exec()
    {
      auto workspace = API::WorkspaceFactory::Instance().createTable("TableWorkspace");
      API::CatalogManager::Instance().getCatalog(getPropertyValue("Session"))->getDataSets(getProperty("InvestigationId"),workspace);
      setProperty("OutputWorkspace",workspace);
    }

  }
}
