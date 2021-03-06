/*WIKI*

This algorithm retrieves the instrument names from a catalog and stores them in a vector.

*WIKI*/

#include "MantidICat/CatalogListInstruments.h"
#include "MantidAPI/CatalogManager.h"
#include "MantidKernel/ArrayProperty.h"

namespace Mantid
{
  namespace ICat
  {

    DECLARE_ALGORITHM(CatalogListInstruments)

    /// Sets documentation strings for this algorithm
    void CatalogListInstruments::initDocs()
    {
      this->setWikiSummary("Lists the name of instruments from Information catalog. ");
      this->setOptionalMessage("Lists the name of instruments from Information catalog.");
    }

    /// Init method
    void CatalogListInstruments::init()
    {
      declareProperty("Session","","The session information of the catalog to use.");
      declareProperty(new Kernel::ArrayProperty<std::string>("InstrumentList",std::vector<std::string>(),
          boost::make_shared<Kernel::NullValidator>(),Kernel::Direction::Output), "A list containing instrument names.");
    }

    /// exec method
    void CatalogListInstruments::exec()
    {
      std::vector<std::string> instruments;
      API::CatalogManager::Instance().getCatalog(getPropertyValue("Session"))->listInstruments(instruments);
      setProperty("InstrumentList",instruments);
    }

  }
}

