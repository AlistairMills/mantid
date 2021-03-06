/*WIKI*
[[MDEventWorkspace]]s and [[MDHistoWorkspace]]s can be used with any type of coordinate system. On the other hand [[PeaksWorkspace]]s may be plotted either in QLab, QSample or HKL. There is an inherent link between a PeaksWorkspace and a MDWorkspace in that an MDWorkspace may utilise the same coordinate systems as the PeaksWorkspaces. For example, workspaces created via [[ConvertToMD]] or [[ConvertToDiffractionMDWorkspace]] may be generated in a special set of V3D coordinates, which are the same as those for the PeaksWorkspace (QLab, QSample, HKL). Amongst other usages, in order to be able to simultaneously plot MDWorkspaces and PeaksWorkspaces, it is necessary to be able to determine what (if any) special coordinates the Workspaces were created in, or are currently using.

This algorithm is for backwards compatibility. The special coordinates flags are new, and legacy workspaces will need to be corrected in order for them to work as expected with the Mantid tools.
*WIKI*/

#include "MantidCrystal/SetSpecialCoordinates.h"
#include "MantidAPI/WorkspaceValidators.h"
#include "MantidAPI/IMDWorkspace.h"
#include "MantidAPI/IPeaksWorkspace.h"
#include "MantidAPI/IMDEventWorkspace.h"
#include "MantidAPI/IMDHistoWorkspace.h"
#include "MantidKernel/ListValidator.h"
#include <boost/make_shared.hpp>

using namespace Mantid::Kernel;
using namespace Mantid::API;

namespace Mantid
{
namespace Crystal
{

    // Register the algorithm into the AlgorithmFactory
    DECLARE_ALGORITHM(SetSpecialCoordinates)

    const std::string SetSpecialCoordinates::QLabOption()
    {
      static const std::string ret("Q (lab frame)");
      return ret;
    }

    const std::string SetSpecialCoordinates::QSampleOption()
    {
      static const std::string ret("Q (sample frame)");
      return ret;
    }

    const std::string SetSpecialCoordinates::HKLOption()
    {
      static const std::string ret("HKL");
      return ret;
    }

    //----------------------------------------------------------------------------------------------
    /** Constructor
     */
    SetSpecialCoordinates::SetSpecialCoordinates()
    {
      m_specialCoordinatesNames.push_back(SetSpecialCoordinates::QLabOption());
      m_specialCoordinatesNames.push_back(SetSpecialCoordinates::QSampleOption());
      m_specialCoordinatesNames.push_back(SetSpecialCoordinates::HKLOption());

      m_specialCoordinatesMap.insert(
          std::make_pair(SetSpecialCoordinates::QLabOption(), Mantid::API::QLab));
      m_specialCoordinatesMap.insert(
          std::make_pair(SetSpecialCoordinates::QSampleOption(), Mantid::API::QSample));
      m_specialCoordinatesMap.insert(std::make_pair(SetSpecialCoordinates::HKLOption(), Mantid::API::HKL));
    }
    
  //----------------------------------------------------------------------------------------------
  /** Destructor
   */
  SetSpecialCoordinates::~SetSpecialCoordinates()
  {
  }
  

  //----------------------------------------------------------------------------------------------
  /// Algorithm's name for identification. @see Algorithm::name
  const std::string SetSpecialCoordinates::name() const { return "SetSpecialCoordinates";};
  
  /// Algorithm's version for identification. @see Algorithm::version
  int SetSpecialCoordinates::version() const { return 1;};
  
  /// Algorithm's category for identification. @see Algorithm::category
  const std::string SetSpecialCoordinates::category() const { return "Crystal";}

  //----------------------------------------------------------------------------------------------
  /// Sets documentation strings for this algorithm
  void SetSpecialCoordinates::initDocs()
  {
    this->setWikiSummary("Set or overwrite any Q3D special coordinates.");
    this->setOptionalMessage("Set or overwrite any Q3D special coordinates.");
  }

    //----------------------------------------------------------------------------------------------
    /** Initialize the algorithm's properties.
     */
    void SetSpecialCoordinates::init()
    {
      declareProperty(new WorkspaceProperty<Workspace>("InputWorkspace", "", Direction::InOut),
          "An input/output workspace. The new log will be added to it.");

      declareProperty("SpecialCoordinates", "Q (lab frame)",
          boost::make_shared<StringListValidator>(m_specialCoordinatesNames),
          "What will be the dimensions of the output workspace?\n"
              "  Q (lab frame): Wave-vector change of the lattice in the lab frame.\n"
              "  Q (sample frame): Wave-vector change of the lattice in the frame of the sample (taking out goniometer rotation).\n"
              "  HKL: Use the sample's UB matrix to convert to crystal's HKL indices.");
    }

    bool SetSpecialCoordinates::writeCoordinatesToMDEventWorkspace(Workspace_sptr inWS, SpecialCoordinateSystem coordinateSystem)
    {
      bool written = false;
      if (auto mdEventWS = boost::dynamic_pointer_cast<IMDEventWorkspace>(inWS))
      {
        mdEventWS->setCoordinateSystem(coordinateSystem);
        written = true;
      }
      return written;
    }

    bool SetSpecialCoordinates::writeCoordinatesToMDHistoWorkspace(Workspace_sptr inWS,
        SpecialCoordinateSystem coordinateSystem)
    {
      bool written = false;
      if (auto mdHistoWS = boost::dynamic_pointer_cast<IMDHistoWorkspace>(inWS))
      {
        mdHistoWS->setCoordinateSystem(coordinateSystem);
        written = true;
      }
      return written;
    }

    bool SetSpecialCoordinates::writeCoordinatesToPeaksWorkspace(Workspace_sptr inWS,
        SpecialCoordinateSystem coordinateSystem)
    {
      bool written = false;
      if (auto peaksWS = boost::dynamic_pointer_cast<IPeaksWorkspace>(inWS))
      {
        peaksWS->setCoordinateSystem(coordinateSystem);
        written = true;
      }
      return written;
    }

  //----------------------------------------------------------------------------------------------
  /** Execute the algorithm.
     */
    void SetSpecialCoordinates::exec()
    {

      Workspace_sptr inputWS = getProperty("InputWorkspace");

      std::string requestedCoordinateSystem = getProperty("SpecialCoordinates");

      SpecialCoordinateSystem coordinatesToUse = this->m_specialCoordinatesMap.find(requestedCoordinateSystem)->second;

      // Try to write the coordinates to the various allowed types of workspace.
      if(!writeCoordinatesToMDEventWorkspace(inputWS, coordinatesToUse))
      {
        if(!writeCoordinatesToMDHistoWorkspace(inputWS, coordinatesToUse))
        {
          if(!writeCoordinatesToPeaksWorkspace(inputWS, coordinatesToUse))
          {
            throw std::invalid_argument("A workspace of this type cannot be processed/");
          }
        }
      }
    }



} // namespace Crystal
} // namespace Mantid
