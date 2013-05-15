/*WIKI* 


*WIKI*/
//----------------------------------------------------------------------
// Includes
//----------------------------------------------------------------------
#include "MantidDataHandling/GetMaskedDetectors.h"
#include "MantidKernel/ArrayProperty.h"
#include "MantidKernel/System.h"
#include <map>

namespace Mantid
{
namespace DataHandling
{

// Register the algorithm into the algorithm factory
DECLARE_ALGORITHM(GetMaskedDetectors)

/// Sets documentation strings for this algorithm
void GetMaskedDetectors::initDocs()
{
  this->setWikiSummary("This algorithm returns a std::vector<int> containing the detector IDs of detectors that have been masked with [[MaskDetectors]] or its like.");
  this->setOptionalMessage("This algorithm returns a std::vector<int> containing the detector IDs of detectors that have been masked with [[MaskDetectors]] or its like.");
}


using namespace Kernel;
using namespace API;

/// (Empty) Constructor
GetMaskedDetectors::GetMaskedDetectors() {
  useAlgorithm("ExtractMasking");
}

/// Destructor
GetMaskedDetectors::~GetMaskedDetectors() {}

void GetMaskedDetectors::init()
{
  declareProperty(
    new WorkspaceProperty<>("InputWorkspace","", Direction::Input),
    "The name of the workspace that will be used as input for the algorithm" );
  declareProperty(new ArrayProperty<detid_t>("DetectorList", Direction::Output),
    "A comma separated list or array containing a list of masked detector ID's" );
}

void GetMaskedDetectors::exec()
{
  // Get the input workspace
  const MatrixWorkspace_sptr WS = getProperty("InputWorkspace");

  // List masked of detector IDs
  std::vector<detid_t> detectorList;

  detid2det_map det_map;
  WS->getInstrument()->getDetectors(det_map);

  for (detid2det_map::const_iterator iter = det_map.begin();
      iter != det_map.end(); ++iter )
  {
    if ( iter->second->isMasked() )
    {
      detectorList.push_back(iter->first);
     }
  }

  setProperty("DetectorList", detectorList);
}



} // namespace DataHandling
} // namespace Mantid
