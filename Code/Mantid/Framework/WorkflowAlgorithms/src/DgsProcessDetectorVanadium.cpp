/*WIKI*

This algorithm is responsible for processing the detector vanadium in the form
required for the sample data normalisation in the convert to energy transfer
process. Parameters in italics are controlled by the
[[InstrumentParameterFile|instrument parameter file (IPF)]] unless provided
to the algorithm via a property manager. The mappings are given below.

{| class="wikitable"
|-
! Parameter !! IPF Mapping
|-
| DetVanIntRangeLow || wb-integr-min
|-
| DetVanIntRangeHigh || wb-integr-max
|}

Parameters in italics with dashed perimeters are only controllable by the IPF
name given. All underlined parameters are fixed and not controllable.
If the input detector vanadium is in TOF units and that is the
requested units for integration, the ''ConvertUnits'' algorithm does not run.
The range parameters feeding into ''Rebin'' are used to make a single bin.
The resulting integrated vanadium workspace can be saved to a file using the
reduction property manager with the boolean property SaveProcessedDetVan.

=== Workflow ===
[[File:DgsProcessDetectorVanadiumWorkflow.png]]

*WIKI*/

#include "MantidWorkflowAlgorithms/DgsProcessDetectorVanadium.h"
#include "MantidAPI/PropertyManagerDataService.h"
#include "MantidAPI/WorkspaceValidators.h"
#include "MantidKernel/ConfigService.h"
#include "MantidKernel/FacilityInfo.h"
#include "MantidWorkflowAlgorithms/WorkflowAlgorithmHelpers.h"

#include <boost/algorithm/string/predicate.hpp>

using namespace Mantid::Kernel;
using namespace Mantid::API;
using namespace WorkflowAlgorithmHelpers;

namespace Mantid
{
  namespace WorkflowAlgorithms
  {

    // Register the algorithm into the AlgorithmFactory
    DECLARE_ALGORITHM(DgsProcessDetectorVanadium)

    //----------------------------------------------------------------------------------------------
    /** Constructor
     */
    DgsProcessDetectorVanadium::DgsProcessDetectorVanadium()
    {
    }

    //----------------------------------------------------------------------------------------------
    /** Destructor
     */
    DgsProcessDetectorVanadium::~DgsProcessDetectorVanadium()
    {
    }

    //----------------------------------------------------------------------------------------------
    /// Algorithm's name for identification. @see Algorithm::name
    const std::string DgsProcessDetectorVanadium::name() const { return "DgsProcessDetectorVanadium"; }

    /// Algorithm's version for identification. @see Algorithm::version
    int DgsProcessDetectorVanadium::version() const { return 1; }

    /// Algorithm's category for identification. @see Algorithm::category
    const std::string DgsProcessDetectorVanadium::category() const { return "Workflow\\Inelastic\\UsesPropertyManager"; }

    //----------------------------------------------------------------------------------------------
    /// Sets documentation strings for this algorithm
    void DgsProcessDetectorVanadium::initDocs()
    {
      this->setWikiSummary("Algorithm to process detector vanadium.");
      this->setOptionalMessage("Algorithm to process detector vanadium.");
    }

    //----------------------------------------------------------------------------------------------
    /** Initialize the algorithm's properties.
     */
    void DgsProcessDetectorVanadium::init()
    {
      //auto wsValidator = boost::make_shared<CompositeValidator>();
      //wsValidator->add<WorkspaceUnitValidator>("TOF");
      this->declareProperty(new WorkspaceProperty<>("InputWorkspace", "", Direction::Input),
          "An input workspace containing the detector vanadium data in TOF units.");
      this->declareProperty(new WorkspaceProperty<>("InputMonitorWorkspace", "",
          Direction::Input, PropertyMode::Optional),
          "A monitor workspace associated with the input workspace.");
      this->declareProperty(new WorkspaceProperty<>("MaskWorkspace",
          "", Direction::Input, PropertyMode::Optional),
          "A mask workspace");
      this->declareProperty(new WorkspaceProperty<>("OutputWorkspace", "", Direction::Output),
          "The name for the output workspace.");
      this->declareProperty("ReductionProperties", "__dgs_reduction_properties",
          Direction::Output);
    }

    //----------------------------------------------------------------------------------------------
    /** Execute the algorithm.
     */
    void DgsProcessDetectorVanadium::exec()
    {
      g_log.notice() << "Starting DgsProcessDetectorVanadium" << std::endl;
      // Get the reduction property manager
      const std::string reductionManagerName = this->getProperty("ReductionProperties");
      boost::shared_ptr<PropertyManager> reductionManager;
      if (PropertyManagerDataService::Instance().doesExist(reductionManagerName))
      {
        reductionManager = PropertyManagerDataService::Instance().retrieve(reductionManagerName);
      }
      else
      {
        throw std::runtime_error("DgsProcessDetectorVanadium cannot run without a reduction PropertyManager.");
      }

      MatrixWorkspace_sptr inputWS = this->getProperty("InputWorkspace");
      MatrixWorkspace_sptr outputWS = this->getProperty("OutputWorkspace");
      MatrixWorkspace_sptr monWS = this->getProperty("InputMonitorWorkspace");

      // Normalise result workspace to incident beam parameter
      IAlgorithm_sptr norm = this->createChildAlgorithm("DgsPreprocessData");
      norm->setProperty("InputWorkspace", inputWS);
      norm->setProperty("OutputWorkspace", inputWS);
      norm->setProperty("InputMonitorWorkspace", monWS);
      norm->executeAsChildAlg();
      inputWS.reset();
      inputWS = norm->getProperty("OutputWorkspace");

      double detVanIntRangeLow = getDblPropOrParam("DetVanIntRangeLow",
          reductionManager, "wb-integr-min", inputWS);

      double detVanIntRangeHigh = getDblPropOrParam("DetVanIntRangeHigh",
          reductionManager, "wb-integr-max", inputWS);

      const std::string detVanIntRangeUnits = reductionManager->getProperty("DetVanIntRangeUnits");

      if ("TOF" != detVanIntRangeUnits)
      {
        // Convert the data to the appropriate units
        IAlgorithm_sptr cnvun = this->createChildAlgorithm("ConvertUnits");
        cnvun->setProperty("InputWorkspace", inputWS);
        cnvun->setProperty("OutputWorkspace", inputWS);
        cnvun->setProperty("Target", detVanIntRangeUnits);
        cnvun->setProperty("EMode", "Elastic");
        cnvun->executeAsChildAlg();
        inputWS = cnvun->getProperty("OutputWorkspace");
      }

      // Rebin the data (not Integration !?!?!?)
      std::vector<double> binning;
      binning.push_back(detVanIntRangeLow);
      binning.push_back(detVanIntRangeHigh - detVanIntRangeLow);
      binning.push_back(detVanIntRangeHigh);

      IAlgorithm_sptr rebin = this->createChildAlgorithm("Rebin");
      rebin->setProperty("InputWorkspace", inputWS);
      rebin->setProperty("OutputWorkspace", outputWS);
      rebin->setProperty("PreserveEvents", false);
      rebin->setProperty("Params", binning);
      rebin->executeAsChildAlg();
      outputWS = rebin->getProperty("OutputWorkspace");

      // Mask and group workspace if necessary.
      MatrixWorkspace_sptr maskWS = this->getProperty("MaskWorkspace");
//!!! I see masks here but where is the map workspace used for vanadium grouping (In ISIS)?
      IAlgorithm_sptr remap = this->createChildAlgorithm("DgsRemap");
      remap->setProperty("InputWorkspace", outputWS);
      remap->setProperty("OutputWorkspace", outputWS);
      remap->setProperty("MaskWorkspace", maskWS);
      remap->executeAsChildAlg();
      outputWS = remap->getProperty("OutputWorkspace");

      const std::string facility = ConfigService::Instance().getFacility().name();
      if ("ISIS" == facility)
      {
        // Scale results by a constant
        double wbScaleFactor = inputWS->getInstrument()->getNumberParameter("wb-scale-factor")[0];
        outputWS *= wbScaleFactor;
      }

      if (reductionManager->existsProperty("SaveProcessedDetVan"))
      {
        bool saveProc = reductionManager->getProperty("SaveProcessedDetVan");
        if (saveProc)
        {
          std::string outputFile("");
          if (reductionManager->existsProperty("SaveProcDetVanFilename"))
          {
            outputFile = reductionManager->getPropertyValue("SaveProcDetVanFilename");
          }
          if (outputFile.empty())
          {
            outputFile = this->getPropertyValue("OutputWorkspace");
            outputFile += ".nxs";
          }

          // Don't save private calculation workspaces
          if (!outputFile.empty() && !boost::starts_with(outputFile, "ChildAlgOutput") &&
              !boost::starts_with(outputFile, "__"))
          {
            IAlgorithm_sptr save = this->createChildAlgorithm("SaveNexus");
            save->setProperty("InputWorkspace", outputWS);
            save->setProperty("FileName", outputFile);
            save->execute();
          }
        }
      }

      this->setProperty("OutputWorkspace", outputWS);
    }

  } // namespace WorkflowAlgorithms
} // namespace Mantid
