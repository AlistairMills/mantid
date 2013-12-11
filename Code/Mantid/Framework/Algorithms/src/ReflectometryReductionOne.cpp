/*WIKI*
 Reduces a single TOF reflectometry run into a mod Q vs I/I0 workspace. Performs transmission corrections. Handles both point detector and multidetector cases.
 The algorithm can correct detector locations based on an input theta value.

 Historically the work performed by this algorithm was known as the Quick script.

 === Analysis Modes ===

 The default analysis mode is ''PointDetectorAnalysis''. Only this mode supports Transmission corrections (see below). For PointAnalysisMode
 the analysis can be roughly reduced to IvsLam = DetectorWS / sum(I0) / TransmissionWS / sum(I0). The normalization by tranmission run(s) is optional.
 Input workspaces are converted to ''Wavelength'' first via [[ConvertUnits]].

 IvsQ is calculated via [[ConvertUnits]] into units of ''MomentumTransfer''. Corrections may be applied prior to the transformation to ensure that the detectors are in the correct location according to the input Theta value.
 Corrections are only enabled when a Theta input value has been provided.

 === Transmission Runs ===
 Transmission correction is a normalization step, which may be applied to ''PointDetectorAnalysis'' reduction.

 Transmission runs are expected to be in TOF. The spectra numbers in the Transmission run workspaces must be the same as those in the Input Run workspace. If two Transmission
 runs are provided then the Stitching parameters associated with the transmission runs will also be required. If a single Transmission run is provided, then no stitching
 parameters will be needed.

 *WIKI*/

#include "MantidAlgorithms/ReflectometryReductionOne.h"
#include "MantidGeometry/Instrument/ReferenceFrame.h"
#include "MantidAPI/MatrixWorkspace.h"
#include "MantidAPI/WorkspaceValidators.h"
#include "MantidAPI/WorkspaceFactory.h"
#include "MantidKernel/ListValidator.h"
#include "MantidKernel/MandatoryValidator.h"
#include "MantidKernel/BoundedValidator.h"
#include "MantidKernel/ArrayProperty.h"
#include "MantidKernel/EnabledWhenProperty.h"
#include "MantidKernel/RebinParamsValidator.h"
#include <boost/make_shared.hpp>
#include <boost/assign/list_of.hpp>

using namespace Mantid::Kernel;
using namespace Mantid::API;
using namespace Mantid::Geometry;

namespace Mantid
{
  namespace Algorithms
  {

    /*Anonomous namespace */
    namespace
    {
      /**
       * Helper non-member function for translating all the workspace indexes in an origin workspace into workspace indexes
       * of a host end-point workspace. This is done using spectrum numbers as the intermediate.
       *
       * This function will throw a runtime error if the specId are not found to exist on the host end-point workspace.
       *
       * @param originWS : Origin workspace, which provides the original workspace index to spectrum id mapping.
       * @param hostWS : Workspace onto which the resulting workspace indexes will be hosted
       * @return Remapped wokspace indexes applicable for the host workspace.
       */
      ReflectometryReductionOne::WorkspaceIndexList createWorkspaceIndexListFromDetectorWorkspace(
          MatrixWorkspace_const_sptr originWS, MatrixWorkspace_const_sptr hostWS)
      {
        auto spectrumMap = originWS->getSpectrumToWorkspaceIndexMap();
        ReflectometryReductionOne::WorkspaceIndexList translatedIndexList;
        for (auto it = spectrumMap.begin(); it != spectrumMap.end(); ++it)
        {
          specid_t specId = (*it).first;
          translatedIndexList.push_back(static_cast<int>(hostWS->getIndexFromSpectrumNumber(specId))); // Could be slow to do it like this.
        }
        return translatedIndexList;
      }

      /**
       * Helper non-member function
       * Get indexes in terms of an end-point host workspace.
       *
       * Throws if the spectrum id cannot be found.
       * @param hostWS : The host end point workspace, which the return values will be generated with reference to.
       * @param originWS : The origin workspace
       * @param originIndexes : Indexes in terms of the origin workspace
       * @return WorkspaceIndexes in terms of the host workspace
       */
      ReflectometryReductionOne::WorkspaceIndexList getIndexesInTermsOf(MatrixWorkspace_const_sptr hostWS,
          MatrixWorkspace_sptr originWS,
          const ReflectometryReductionOne::WorkspaceIndexList& originIndexes)
      {
        auto spectrumMap = hostWS->getSpectrumToWorkspaceIndexMap();
        ReflectometryReductionOne::WorkspaceIndexList translatedIndexList;
        for (size_t i = 0; i < originIndexes.size(); ++i)
        {
          const specid_t specNumber = originWS->getSpectrum(i)->getSpectrumNo();
          translatedIndexList.push_back(static_cast<int>(spectrumMap[specNumber]));
        }
        return translatedIndexList;
      }

      const std::string multiDetectorAnalysis = "MultiDetectorAnalysis";
      const std::string pointDetectorAnalysis = "PointDetectorAnalysis";
    }
    /* End of ananomous namespace */

    // Register the algorithm into the AlgorithmFactory
    DECLARE_ALGORITHM(ReflectometryReductionOne)

    //----------------------------------------------------------------------------------------------
    /** Constructor
     */
    ReflectometryReductionOne::ReflectometryReductionOne()
    {
    }

    //----------------------------------------------------------------------------------------------
    /** Destructor
     */
    ReflectometryReductionOne::~ReflectometryReductionOne()
    {
    }

    //----------------------------------------------------------------------------------------------
    /// Algorithm's name for identification. @see Algorithm::name
    const std::string ReflectometryReductionOne::name() const
    {
      return "ReflectometryReductionOne";
    }
    ;

    /// Algorithm's version for identification. @see Algorithm::version
    int ReflectometryReductionOne::version() const
    {
      return 1;
    }
    ;

    /// Algorithm's category for identification. @see Algorithm::category
    const std::string ReflectometryReductionOne::category() const
    {
      return "Reflectometry\\ISIS";
    }

    //----------------------------------------------------------------------------------------------
    /// Sets documentation strings for this algorithm
    void ReflectometryReductionOne::initDocs()
    {
      this->setWikiSummary(
          "Reduces a single TOF reflectometry run into a mod Q vs I/I0 workspace. Performs transmission corrections.");
      this->setOptionalMessage(this->getWikiSummary());
    }

    //----------------------------------------------------------------------------------------------
    /** Initialize the algorithm's properties.
     */
    void ReflectometryReductionOne::init()
    {
      boost::shared_ptr<CompositeValidator> inputValidator = boost::make_shared<CompositeValidator>();
      inputValidator->add(boost::make_shared<WorkspaceUnitValidator>("TOF"));

      declareProperty(
          new WorkspaceProperty<MatrixWorkspace>("InputWorkspace", "", Direction::Input, inputValidator),
          "Run to reduce.");

      std::vector<std::string> propOptions;
      propOptions.push_back(pointDetectorAnalysis);
      propOptions.push_back(multiDetectorAnalysis);

      declareProperty("AnalysisMode", "PointDetectorAnalysis",
          boost::make_shared<StringListValidator>(propOptions),
          "The type of analysis to perform. Point detector or multi detector.");

      declareProperty(new ArrayProperty<int>("RegionOfInterest"),
          "Indices of the spectra a pair (lower, upper) that mark the ranges that correspond to the region of interest (reflected beam) in multi-detector mode.");
      declareProperty(new ArrayProperty<int>("RegionOfDirectBeam"),
          "Indices of the spectra a pair (lower, upper) that mark the ranges that correspond to the direct beam in multi-detector mode.");

      declareProperty(
          new PropertyWithValue<double>("WavelengthMin", Mantid::EMPTY_DBL(),
              boost::make_shared<MandatoryValidator<double> >(), Direction::Input),
          "Wavelength minimum in angstroms");
      declareProperty(
          new PropertyWithValue<double>("WavelengthMax", Mantid::EMPTY_DBL(),
              boost::make_shared<MandatoryValidator<double> >(), Direction::Input),
          "Wavelength maximum in angstroms");

      declareProperty(
                new PropertyWithValue<double>("WavelengthStep", 0.05,
                    boost::make_shared<MandatoryValidator<double> >(), Direction::Input),
                "Wavelength rebinning step in angstroms. Defaults to 0.05. Used for rebinning intermediate workspaces converted into wavelength.");

      boost::shared_ptr<CompositeValidator> mandatoryWorkspaceIndex = boost::make_shared<
          CompositeValidator>();
      mandatoryWorkspaceIndex->add(boost::make_shared<MandatoryValidator<int> >());
      auto boundedIndex = boost::make_shared<BoundedValidator<int> >();
      boundedIndex->setLower(0);
      mandatoryWorkspaceIndex->add(boundedIndex);

      declareProperty(
          new PropertyWithValue<int>("I0MonitorIndex", Mantid::EMPTY_INT(), mandatoryWorkspaceIndex),
          "I0 monitor index");

      declareProperty(
          new PropertyWithValue<double>("MonitorBackgroundWavelengthMin", Mantid::EMPTY_DBL(),
              boost::make_shared<MandatoryValidator<double> >(), Direction::Input),
          "Wavelength minimum for monitor background in angstroms. Taken to be WavelengthMin if not provided.");

      declareProperty(
          new PropertyWithValue<double>("MonitorBackgroundWavelengthMax", Mantid::EMPTY_DBL(),
              boost::make_shared<MandatoryValidator<double> >(), Direction::Input),
          "Wavelength maximum for monitor background in angstroms. Taken to be WavelengthMax if not provided.");

      declareProperty(
          new PropertyWithValue<double>("MonitorIntegrationWavelengthMin", Mantid::EMPTY_DBL(),
              boost::make_shared<MandatoryValidator<double> >(), Direction::Input),
          "Wavelength minimum for integration in angstroms. Taken to be WavelengthMin if not provided.");
      declareProperty(
          new PropertyWithValue<double>("MonitorIntegrationWavelengthMax", Mantid::EMPTY_DBL(),
              boost::make_shared<MandatoryValidator<double> >(), Direction::Input),
          "Wavelength maximum for integration in angstroms. Taken to be WavelengthMax if not provided.");

      declareProperty(new ArrayProperty<int>("WorkspaceIndexList"),
               "Indices of the spectra in pairs (lower, upper) that mark the ranges that correspond to detectors of interest.");

      declareProperty(new PropertyWithValue<std::string>("DetectorComponentName", "", Direction::Input),
          "Name of the detector component i.e. point-detector. If these are not specified, the algorithm will attempt lookup using a standard naming convention.");

      declareProperty(new PropertyWithValue<std::string>("SampleComponentName", "", Direction::Input),
                "Name of the sample component i.e. some-surface-holder. If these are not specified, the algorithm will attempt lookup using a standard naming convention.");

      declareProperty(new WorkspaceProperty<>("OutputWorkspace", "", Direction::Output), "Output Workspace IvsQ.");

      declareProperty(new WorkspaceProperty<>("OutputWorkspaceWavelength", "", Direction::Output, PropertyMode::Optional), "Output Workspace IvsLam. Intermediate workspace.");

      declareProperty(new PropertyWithValue<double>("ThetaIn", Mantid::EMPTY_DBL(), Direction::Input),
                "Final theta value in degrees. Optional, this value will be calculated internally and provided as ThetaOut if not provided.");

      declareProperty(new PropertyWithValue<double>("ThetaOut", Mantid::EMPTY_DBL(), Direction::Output), "Calculated final theta in degrees.");

      declareProperty(new PropertyWithValue<bool>("CorrectDetectorPositions", true, Direction::Input), "Correct detector positions using ThetaIn (if given)");

      declareProperty(
          new WorkspaceProperty<MatrixWorkspace>("FirstTransmissionRun", "", Direction::Input,
              PropertyMode::Optional, inputValidator->clone()),
          "First transmission run, or the low wavelength transmision run if SecondTransmissionRun is also provided.");
      declareProperty(
          new WorkspaceProperty<MatrixWorkspace>("SecondTransmissionRun", "", Direction::Input,
              PropertyMode::Optional, inputValidator->clone()),
          "Second, high wavelength transmission run. Optional. Causes the FirstTransmissionRun to be treated as the low wavelength transmission run.");

      declareProperty(
          new ArrayProperty<double>("Params", boost::make_shared<RebinParamsValidator>(true)),
          "A comma separated list of first bin boundary, width, last bin boundary. "
              "These parameters are used for stitching together transmission runs. "
              "Values are in q. This input is only needed if a SecondTransmission run is provided.");

      declareProperty(
          new PropertyWithValue<double>("StartOverlapQ", Mantid::EMPTY_DBL(), Direction::Input),
          "Start Q for stitching transmission runs together");
      declareProperty(
          new PropertyWithValue<double>("EndOverlapQ", Mantid::EMPTY_DBL(), Direction::Input),
          "End Q for stitching transmission runs together");

      setPropertyGroup("FirstTransmissionRun", "Transmission");
      setPropertyGroup("SecondTransmissionRun", "Transmission");
      setPropertyGroup("Params", "Transmission");
      setPropertyGroup("StartOverlapQ", "Transmission");
      setPropertyGroup("EndOverlapQ", "Transmission");

      // Only do transmission corrections when point detector.
      setPropertySettings("FirstTransmissionRun", new Kernel::EnabledWhenProperty("AnalysisMode", IS_EQUAL_TO, "PointDetectorAnalysis"));
      setPropertySettings("SecondTransmissionRun", new Kernel::EnabledWhenProperty("AnalysisMode", IS_EQUAL_TO, "PointDetectorAnalysis"));
      setPropertySettings("Params", new Kernel::EnabledWhenProperty("AnalysisMode", IS_EQUAL_TO, "PointDetectorAnalysis"));
      setPropertySettings("StartOverlapQ", new Kernel::EnabledWhenProperty("AnalysisMode", IS_EQUAL_TO, "PointDetectorAnalysis"));
      setPropertySettings("EndOverlapQ", new Kernel::EnabledWhenProperty("AnalysisMode", IS_EQUAL_TO, "PointDetectorAnalysis"));

      // Only use region of interest when in multi-detector analysis mode
      setPropertySettings("RegionOfInterest",
          new Kernel::EnabledWhenProperty("AnalysisMode", IS_EQUAL_TO, "MultiDetectorAnalysis"));

      // Only use region of direct beam when in multi-detector analysis mode.
      setPropertySettings("RegionOfDirectBeam",
          new Kernel::EnabledWhenProperty("AnalysisMode", IS_EQUAL_TO, "MultiDetectorAnalysis"));
    }

    /**
     * Determine if the property value is the same as the default value.
     * This can be used to determine if the property has not been set.
     * @param propertyName : Name of property to query
     * @return: True only if the property has it's default value.
     */
    bool ReflectometryReductionOne::isPropertyDefault(const std::string& propertyName) const
    {
      Property* property = this->getProperty(propertyName);
      return property->isDefault();
    }

    /**
     *  Helper method used with the stl to determine whether values are negative
     * @param value : Value to check
     * @return : True if negative.
     */
    bool checkNotPositive(const int value)
    {
      return value < 0;
    }

    /**
     * Get the workspace index list
     * @return Workspace index list.
     */
    ReflectometryReductionOne::WorkspaceIndexList ReflectometryReductionOne::getWorkspaceIndexList() const
    {
      WorkspaceIndexList indexList = getProperty("WorkspaceIndexList");
      if (indexList.size() % 2 != 0 || indexList.size() == 0)
      {
        throw std::invalid_argument(
            "WorkspaceIndex list must be composed of pairs of min index, max index.");
      }

      if (std::find_if(indexList.begin(), indexList.end(), checkNotPositive) != indexList.end())
      {
        throw std::invalid_argument("WorkspaceIndexList contains negative indexes");
      }

      for (size_t i = 0; (i + 1) < indexList.size(); i += 2)
      {
        if (indexList[i] > indexList[i + 1])
          throw std::invalid_argument("WorkspaceIndexList pairs must be in min, max order");
      }
      return indexList;
    }

    /**
     * Fetch min, max inputs as a vector (int) if they are non-default and set them to the optionalUpperLower object.
     * Performs checks to verify that invalid indexes have not been passed in.
     * @param propertyName : Property name to fetch
     * @param isPointDetector : Flag indicates that the execution is in point detector mode.
     * @param optionalUpperLower : Object to set min and max on.
     */
    void ReflectometryReductionOne::fetchOptionalLowerUpperPropertyValue(const std::string& propertyName,
        bool isPointDetector, OptionalWorkspaceIndexes& optionalUpperLower) const
    {
      if (!isPropertyDefault(propertyName))
      {
        // Validation of property inputs.
        if (isPointDetector)
        {
          throw std::invalid_argument(
              "Cannot have a region of interest property in point detector mode.");
        }
        std::vector<int> temp = this->getProperty(propertyName);
        if (temp.size() != 2)
        {
          const std::string message = propertyName + " requires a lower and upper boundary";
          throw std::invalid_argument(message);
        }
        if (temp[0] > temp[1])
        {
          throw std::invalid_argument("Min must be <= Max index");
        }
        if (std::find_if(temp.begin(), temp.end(), checkNotPositive) != temp.end())
        {
          const std::string message = propertyName + " contains negative indexes";
          throw std::invalid_argument(message);
        }
        // Assignment
        optionalUpperLower = temp;
      }
    }

    /**
     * Get min max pairs as a tuple.
     * @param minProperty : Property name for the min property
     * @param maxProperty : Property name for the max property
     * @return A tuple consisting of min, max
     */
    ReflectometryReductionOne::MinMax ReflectometryReductionOne::getMinMax(
        const std::string& minProperty, const std::string& maxProperty) const
    {
      const double min = getProperty(minProperty);
      const double max = getProperty(maxProperty);
      if (min > max)
      {
        throw std::invalid_argument("Cannot have any WavelengthMin > WavelengthMax");
      }
      return MinMax(min, max);
    }

    /**
     * Validate the transmission workspace inputs when a second transmission run is provided.
     * Throws if any of the property values do not make sense.
     */
    void ReflectometryReductionOne::validateTransmissionInputs() const
    {
      // Verify that all the required inputs for the second transmission run are now given.
      if (isPropertyDefault("FirstTransmissionRun"))
      {
        throw std::invalid_argument(
            "A SecondTransmissionRun is only valid if a FirstTransmissionRun is provided.");
      }
      if (isPropertyDefault("Params"))
      {
        throw std::invalid_argument(
            "If a SecondTransmissionRun has been given, then stitching Params for the transmission runs are also required.");
      }
      if (isPropertyDefault("StartOverlapQ"))
      {
        throw std::invalid_argument(
            "If a SecondTransmissionRun has been given, then a stitching StartOverlapQ for the transmission runs is also required.");
      }
      if (isPropertyDefault("EndOverlapQ"))
      {
        throw std::invalid_argument(
            "If a SecondTransmissionRun has been given, then a stitching EndOverlapQ for the transmission runs is also required.");
      }
      const double startOverlapQ = this->getProperty("StartOverlapQ");
      const double endOverlapQ = this->getProperty("EndOverlapQ");
      if (startOverlapQ >= endOverlapQ)
      {
        throw std::invalid_argument("EndOverlapQ must be > StartOverlapQ");
      }

    }

    /**
     * Get the transmission run information.
     *
     * Transmission runs are optional, but you cannot have the second without the first. Also, stitching
     * parameters are required if the second is present. This getter fetches and assigns to the optional reference arguments
     *
     * @param firstTransmissionRun
     * @param secondTransmissionRun
     * @param stitchingStartQ
     * @param stitchingDeltaQ
     * @param stitchingEndQ
     */
    void ReflectometryReductionOne::getTransmissionRunInfo(
        OptionalMatrixWorkspace_sptr& firstTransmissionRun,
        OptionalMatrixWorkspace_sptr& secondTransmissionRun, OptionalDouble& stitchingStartQ,
        OptionalDouble& stitchingDeltaQ, OptionalDouble& stitchingEndQ,
        OptionalDouble& stitchingStartOverlapQ, OptionalDouble& stitchingEndOverlapQ) const
    {
      if (!isPropertyDefault("FirstTransmissionRun"))
      {
        MatrixWorkspace_sptr temp = this->getProperty("FirstTransmissionRun");
        /*
         if(temp->getNumberHistograms() > 1)
         {
         throw std::invalid_argument("Error with FirstTransmissionRun. Only one histogram is permitted for a transmission run.");
         }
         */
        firstTransmissionRun = temp;
      }

      if (!isPropertyDefault("SecondTransmissionRun"))
      {
        // Check that the property values provided make sense together.
        validateTransmissionInputs();

        // Set the values.
        {
          MatrixWorkspace_sptr temp = this->getProperty("SecondTransmissionRun");
          secondTransmissionRun = temp;
        }
        {
          std::vector<double> params = getProperty("Params");
          stitchingStartQ = params[0];
          stitchingDeltaQ = params[1];
          stitchingEndQ = params[2];
        }
        {
          double temp = this->getProperty("StartOverlapQ");
          stitchingStartOverlapQ = temp;
          temp = this->getProperty("EndOverlapQ");
          stitchingEndOverlapQ = temp;
        }
      }

    }


    /**
     * Convert the TOF workspace into a monitor workspace. Crops to the monitorIndex and applying flat background correction as part of the process.
     * @param toConvert : TOF wavlength to convert.
     * @param monitorIndex : Monitor index to crop to
     * @param backgroundMinMax : Min and Max Lambda range for Flat background correction.
     * @return The cropped and corrected monitor workspace.
     */
    MatrixWorkspace_sptr ReflectometryReductionOne::toLamMonitor(const MatrixWorkspace_sptr& toConvert, const int monitorIndex, const MinMax& backgroundMinMax)
    {
      // Convert Units.
      auto convertUnitsAlg = this->createChildAlgorithm("ConvertUnits");
      convertUnitsAlg->initialize();
      convertUnitsAlg->setProperty("InputWorkspace", toConvert);
      convertUnitsAlg->setProperty("Target", "Wavelength");
      convertUnitsAlg->setProperty("AlignBins", true);
      convertUnitsAlg->execute();

      // Crop the to the monitor index.
      MatrixWorkspace_sptr monitorWS = convertUnitsAlg->getProperty("OutputWorkspace");
      auto cropWorkspaceAlg = this->createChildAlgorithm("CropWorkspace");
      cropWorkspaceAlg->initialize();
      cropWorkspaceAlg->setProperty("InputWorkspace", monitorWS);
      cropWorkspaceAlg->setProperty("StartWorkspaceIndex", monitorIndex);
      cropWorkspaceAlg->setProperty("EndWorkspaceIndex", monitorIndex);
      cropWorkspaceAlg->execute();
      monitorWS = cropWorkspaceAlg->getProperty("OutputWorkspace");

      // Flat background correction
      auto correctMonitorsAlg = this->createChildAlgorithm("CalculateFlatBackground");
      correctMonitorsAlg->initialize();
      correctMonitorsAlg->setProperty("InputWorkspace", monitorWS);
      correctMonitorsAlg->setProperty("WorkspaceIndexList",
          boost::assign::list_of(0).convert_to_container<std::vector<int> >());
      correctMonitorsAlg->setProperty("StartX", backgroundMinMax.get<0>());
      correctMonitorsAlg->setProperty("EndX", backgroundMinMax.get<1>());
      correctMonitorsAlg->execute();
      monitorWS = correctMonitorsAlg->getProperty("OutputWorkspace");

      return monitorWS;
    }

    /**
     * Convert to a detector workspace in lambda.
     * @param detectorIndexRange : Workspace index ranges to keep
     * @param toConvert : TOF wavelength to convert.
     * @param wavelengthMinMax : Wavelength minmax to keep. Crop out the rest.
     * @param wavelengthStep : Wavelength step for rebinning
     * @return Detector workspace in wavelength
     */
    MatrixWorkspace_sptr ReflectometryReductionOne::toLamDetector(
        const WorkspaceIndexList& detectorIndexRange, const MatrixWorkspace_sptr& toConvert,
        const MinMax& wavelengthMinMax, const double& wavelengthStep)
    {
      // Detector Workspace Processing
      MatrixWorkspace_sptr detectorWS;

      // Loop over pairs of detector index ranges. Peform the cropping and then conjoin the results into a single workspace.
      for (size_t i = 0; i < detectorIndexRange.size(); i += 2)
      {
        auto cropWorkspaceAlg = this->createChildAlgorithm("CropWorkspace");
        cropWorkspaceAlg->initialize();
        cropWorkspaceAlg->setProperty("InputWorkspace", toConvert);
        cropWorkspaceAlg->setProperty("StartWorkspaceIndex", detectorIndexRange[i]);
        cropWorkspaceAlg->setProperty("EndWorkspaceIndex", detectorIndexRange[i + 1]);
        cropWorkspaceAlg->execute();
        MatrixWorkspace_sptr subRange = cropWorkspaceAlg->getProperty("OutputWorkspace");
        if (i == 0)
        {
          detectorWS = subRange;
        }
        else
        {
          auto conjoinWorkspaceAlg = this->createChildAlgorithm("ConjoinWorkspaces");
          conjoinWorkspaceAlg->initialize();
          conjoinWorkspaceAlg->setProperty("InputWorkspace1", detectorWS);
          conjoinWorkspaceAlg->setProperty("InputWorkspace2", subRange);
          conjoinWorkspaceAlg->execute();
          detectorWS = conjoinWorkspaceAlg->getProperty("InputWorkspace1");
        }
      }
      // Now convert units. Do this after the conjoining step otherwise the x bins will not match up.
      auto convertUnitsAlg = this->createChildAlgorithm("ConvertUnits");
      convertUnitsAlg->initialize();
      convertUnitsAlg->setProperty("InputWorkspace", detectorWS);
      convertUnitsAlg->setProperty("Target", "Wavelength");
      convertUnitsAlg->setProperty("AlignBins", true);
      convertUnitsAlg->execute();
      detectorWS = convertUnitsAlg->getProperty("OutputWorkspace");

      // Crop out the lambda x-ranges now that the workspace is in wavelength.
      auto cropWorkspaceAlg = this->createChildAlgorithm("CropWorkspace");
      cropWorkspaceAlg->initialize();
      cropWorkspaceAlg->setProperty("InputWorkspace", detectorWS);
      cropWorkspaceAlg->setProperty("XMin", wavelengthMinMax.get<0>());
      cropWorkspaceAlg->setProperty("XMax", wavelengthMinMax.get<1>());
      cropWorkspaceAlg->execute();
      detectorWS = cropWorkspaceAlg->getProperty("OutputWorkspace");

      auto rebinWorkspaceAlg = this->createChildAlgorithm("Rebin");
      rebinWorkspaceAlg->initialize();
      std::vector<double> params = boost::assign::list_of(wavelengthStep);
      rebinWorkspaceAlg->setProperty("Params", params);
      rebinWorkspaceAlg->setProperty("InputWorkspace", detectorWS);
      rebinWorkspaceAlg->execute();
      detectorWS = rebinWorkspaceAlg->getProperty("OutputWorkspace");

      return detectorWS;
    }

    /**
     * Convert From a TOF workspace into a detector and monitor workspace both in Lambda.
     * @param toConvert: TOF workspace to convert
     * @param detectorIndexRange : Detector index ranges
     * @param monitorIndex : Monitor index
     * @param wavelengthMinMax : Wavelength min max for detector workspace
     * @param backgroundMinMax : Wavelength min max for flat background correction of monitor workspace
     * @param wavelengthStep : Wavlength step size for rebinning.
     * @return Tuple of detector and monitor workspaces
     */
    ReflectometryReductionOne::DetectorMonitorWorkspacePair ReflectometryReductionOne::toLam(MatrixWorkspace_sptr toConvert,
        const WorkspaceIndexList& detectorIndexRange, const int monitorIndex,
        const MinMax& wavelengthMinMax, const MinMax& backgroundMinMax, const double& wavelengthStep)
    {
      // Detector Workspace Processing
      MatrixWorkspace_sptr detectorWS = toLamDetector(detectorIndexRange, toConvert, wavelengthMinMax, wavelengthStep);

      // Monitor Workspace Processing
      MatrixWorkspace_sptr monitorWS = toLamMonitor(toConvert, monitorIndex, backgroundMinMax);

      // Rebin the Monitor Workspace to match the Detector Workspace.
      auto rebinToWorkspaceAlg = this->createChildAlgorithm("RebinToWorkspace");
      rebinToWorkspaceAlg->initialize();
      rebinToWorkspaceAlg->setProperty("WorkspaceToRebin", monitorWS);
      rebinToWorkspaceAlg->setProperty("WorkspaceToMatch", detectorWS);
      rebinToWorkspaceAlg->execute();
      monitorWS = rebinToWorkspaceAlg->getProperty("OutputWorkspace");

      return DetectorMonitorWorkspacePair( detectorWS, monitorWS );
    }

    /**
     * Perform Transmission Corrections.
     * @param IvsLam : Run workspace which is to be normalized by the results of the transmission corrections.
     * @param wavelengthInterval : Wavelength interval for the run workspace.
     * @param wavelengthMonitorBackgroundInterval : Wavelength interval for the monitor background
     * @param wavelengthMonitorIntegrationInterval : Wavelength interval for the monitor integration
     * @param i0MonitorIndex : Monitor index for the I0 monitor
     * @param firstTransmissionRun : The first transmission run
     * @param secondTransmissionRun : The second transmission run (optional)
     * @param stitchingStartQ : Stitching start Q (optional but dependent on secondTransmissionRun)
     * @param stitchingDeltaQ : Stitching delta Q (optional but dependent on secondTransmissionRun)
     * @param stitchingEndQ : Stitching end Q (optional but dependent on secondTransmissionRun)
     * @param stitchingStartOverlapQ : Stitching start Q overlap (optional but dependent on secondTransmissionRun)
     * @param stitchingEndOverlapQ : Stitching end Q overlap (optional but dependent on secondTransmissionRun)
     * @param wavelengthStep : Step in angstroms for rebinning for workspaces converted into wavelength.
     * @return Normalized run workspace by the transmission workspace, which have themselves been converted to Lam, normalized by monitors and possibly stitched together.
     */
    MatrixWorkspace_sptr ReflectometryReductionOne::transmissonCorrection(MatrixWorkspace_sptr IvsLam,
        const MinMax& wavelengthInterval,
        const MinMax& wavelengthMonitorBackgroundInterval,
        const MinMax& wavelengthMonitorIntegrationInterval,
        const int& i0MonitorIndex,
        MatrixWorkspace_sptr firstTransmissionRun,
        OptionalMatrixWorkspace_sptr secondTransmissionRun,
        const OptionalDouble& stitchingStartQ,
        const OptionalDouble& stitchingDeltaQ,
        const OptionalDouble& stitchingEndQ,
        const OptionalDouble& stitchingStartOverlapQ,
        const OptionalDouble& stitchingEndOverlapQ,
        const double& wavelengthStep
    )
    {
      g_log.debug("Extracting first transmission run workspace indexes from spectra");
      const WorkspaceIndexList detectorIndexes = createWorkspaceIndexListFromDetectorWorkspace(IvsLam, firstTransmissionRun);
      auto trans1InLam = toLam(firstTransmissionRun, detectorIndexes, i0MonitorIndex, wavelengthInterval, wavelengthMonitorBackgroundInterval, wavelengthStep);
      MatrixWorkspace_sptr trans1Detector = trans1InLam.get<0>();
      MatrixWorkspace_sptr trans1Monitor = trans1InLam.get<1>();

      // Monitor integration ... can this happen inside the toLam routine?
      auto integrationAlg = this->createChildAlgorithm("Integration");
      integrationAlg->initialize();
      integrationAlg->setProperty("InputWorkspace", trans1Monitor);
      integrationAlg->setProperty("RangeLower", wavelengthMonitorIntegrationInterval.get<0>());
      integrationAlg->setProperty("RangeUpper", wavelengthMonitorIntegrationInterval.get<1>());
      integrationAlg->execute();
      trans1Monitor = integrationAlg->getProperty("OutputWorkspace");

      MatrixWorkspace_sptr denominator = trans1Detector / trans1Monitor;

      if (secondTransmissionRun.is_initialized())
      {
        auto transRun2 = secondTransmissionRun.get();
        g_log.debug("Extracting second transmission run workspace indexes from spectra");
        const WorkspaceIndexList detectorIndexes = createWorkspaceIndexListFromDetectorWorkspace(IvsLam, transRun2);

        auto trans2InLam = toLam(transRun2, detectorIndexes, i0MonitorIndex, wavelengthInterval,
            wavelengthMonitorBackgroundInterval, wavelengthStep);

        // Unpack the conversion results.
        MatrixWorkspace_sptr trans2Detector = trans2InLam.get<0>();
        MatrixWorkspace_sptr trans2Monitor = trans2InLam.get<1>();

        // Monitor integration ... can this happen inside the toLam routine?
        auto integrationAlg = this->createChildAlgorithm("Integration");
        integrationAlg->initialize();
        integrationAlg->setProperty("InputWorkspace", trans2Monitor);
        integrationAlg->setProperty("RangeLower", wavelengthMonitorIntegrationInterval.get<0>());
        integrationAlg->setProperty("RangeUpper", wavelengthMonitorIntegrationInterval.get<1>());
        integrationAlg->execute();
        trans2Monitor = integrationAlg->getProperty("OutputWorkspace");

        MatrixWorkspace_sptr normalizedTrans2 = trans2Detector / trans2Monitor;

        // Stitch the results.
        auto stitch1DAlg = this->createChildAlgorithm("Stitch1D");
        stitch1DAlg->initialize();
        AnalysisDataService::Instance().addOrReplace("denominator", denominator);
        AnalysisDataService::Instance().addOrReplace("normalizedTrans2", normalizedTrans2);
        stitch1DAlg->setProperty("LHSWorkspace", denominator);
        stitch1DAlg->setProperty("RHSWorkspace", normalizedTrans2);
        stitch1DAlg->setProperty("StartOverlap", stitchingStartOverlapQ.get() );
        stitch1DAlg->setProperty("EndOverlap", stitchingEndOverlapQ.get() );
        const std::vector<double> params = boost::assign::list_of(stitchingStartQ.get())(stitchingDeltaQ.get())(stitchingEndQ.get()).convert_to_container<std::vector< double > >();
        stitch1DAlg->setProperty("Params", params);
        stitch1DAlg->execute();
        denominator = stitch1DAlg->getProperty("OutputWorkspace");
        AnalysisDataService::Instance().remove("denominator");
        AnalysisDataService::Instance().remove("normalizedTrans2");
      }

        auto rebinToWorkspaceAlg = this->createChildAlgorithm("RebinToWorkspace");
        rebinToWorkspaceAlg->initialize();
        rebinToWorkspaceAlg->setProperty("WorkspaceToMatch", IvsLam);
        rebinToWorkspaceAlg->setProperty("WorkspaceToRebin", denominator);
        rebinToWorkspaceAlg->execute();
        denominator = rebinToWorkspaceAlg->getProperty("OutputWorkspace");

        MatrixWorkspace_sptr normalizedIvsLam = IvsLam / denominator;
        return normalizedIvsLam;
    }

    /**
     * Correct the position of the detectors based on the input theta value.
     * @param toCorrect : Workspace to correct detector posisitions on.
     * @param thetaInDeg : Theta in degrees to use in correction calculations.
     */
    void ReflectometryReductionOne::correctPosition(API::MatrixWorkspace_sptr toCorrect, const double& thetaInDeg, IComponent_const_sptr sample, IComponent_const_sptr detector)
    {

      auto instrument = toCorrect->getInstrument();

      const V3D detectorPosition = detector->getPos();

      const V3D samplePosition = sample->getPos();

      const V3D sampleToDetector = detectorPosition - samplePosition;

      auto referenceFrame = instrument->getReferenceFrame();

      const double sampleToDetectorAlongBeam = sampleToDetector.scalar_prod( referenceFrame->vecPointingAlongBeam() ) ;

      const double thetaInRad = thetaInDeg * ( M_PI / 180.0 );

      double  acrossOffset = 0;

      double beamOffset = detectorPosition.scalar_prod( referenceFrame->vecPointingAlongBeam() );

      double upOffset = sampleToDetectorAlongBeam * std::sin( 2.0 * thetaInRad );

      auto moveComponentAlg = this->createChildAlgorithm("MoveInstrumentComponent");
      moveComponentAlg->initialize();
      moveComponentAlg->setProperty("Workspace", toCorrect);
      moveComponentAlg->setProperty("ComponentName", detector->getName());
      moveComponentAlg->setProperty("RelativePosition", false);
      // Movements
      moveComponentAlg->setProperty(referenceFrame->pointingAlongBeamAxis(), beamOffset);
      moveComponentAlg->setProperty(referenceFrame->pointingHorizontalAxis(), acrossOffset);
      moveComponentAlg->setProperty(referenceFrame->pointingUpAxis(), upOffset);
      // Execute the movement.
      moveComponentAlg->execute();

    }

    /**
     * Convert an input workspace into an IvsQ workspace.
     *
     * @param toConvert : Workspace to convert
     * @param bCorrectPosition : Flag to indicate that detector positions should be corrected based on the input theta values.
     * @param isPointDetector : Flag to indicate that this is a point detector reduction run.
     * @param thetaInDeg : Theta in Degrees. Used for correction.
     * @param sample : Sample component
     * @param detector : Detector component
     * @return
     */
    Mantid::API::MatrixWorkspace_sptr ReflectometryReductionOne::toIvsQ(API::MatrixWorkspace_sptr toConvert, const bool bCorrectPosition,
        const bool isPointDetector,  OptionalDouble& thetaInDeg, Geometry::IComponent_const_sptr sample, Geometry::IComponent_const_sptr detector)
    {
      /*
       * Can either calculate a missing theta value for the purposes of reporting, or correct positions based on a theta value,
       * but not both. The processing is effectively circular if both are applied.
       */
      if (!thetaInDeg.is_initialized())
      {
        g_log.debug("Calculating final theta.");
        const double thetaToRad = 180 / M_PI;

        Instrument_const_sptr instrument = toConvert->getInstrument();

        const V3D sampleToDetectorPos = detector->getPos() - sample->getPos();

        const V3D sourcePos = instrument->getSource()->getPos();
        const V3D beamPos = sample->getPos() - sourcePos;

        const V3D sampleDetVec = detector->getPos() - sample->getPos();
        const double calculatedTheta =  sampleDetVec.angle(beamPos) * thetaToRad * 1 / 2;

        thetaInDeg = calculatedTheta / thetaToRad; // Assign calculated value it.
      }
      else if( bCorrectPosition ) // This probably ought to be an automatic decision. How about making a guess about sample position holder and detector names. But also allowing the two component names (sample and detector) to be passed in.
      {
        g_log.debug("Correcting detector position");
        correctPosition(toConvert, thetaInDeg.get(), sample, detector);
      }

      // Always convert units.
      auto convertUnits = this->createChildAlgorithm("ConvertUnits");
      convertUnits->initialize();
      convertUnits->setProperty("InputWorkspace", toConvert);
      convertUnits->setProperty("Target", "MomentumTransfer");
      convertUnits->execute();
      MatrixWorkspace_sptr inQ = convertUnits->getProperty("OutputWorkspace");
      return inQ;
    }

    /**
     * Get the sample component. Use the name provided as a property as the basis for the lookup as a priority.
     *
     * Throws if the name is invalid.
     * @param inst : Instrument to search through
     * @return : The component : The component object found.
     */
    Mantid::Geometry::IComponent_const_sptr ReflectometryReductionOne::getSurfaceSampleComponent(Mantid::Geometry::Instrument_const_sptr inst)
    {
      std::string sampleComponent = "some-surface-holder";
      if (!isPropertyDefault("SampleComponentName"))
      {
        sampleComponent = this->getPropertyValue("SampleComponentName");
      }
      auto searchResult = inst->getComponentByName(sampleComponent);
      if (searchResult == NULL)
      {
        throw std::invalid_argument(sampleComponent + " does not exist. Check input properties.");
      }
      return searchResult;
    }

    /**
     * Get the detector component. Use the name provided as a property as the basis for the lookup as a priority.
     *
     * Throws if the name is invalid.
     * @param inst : Instrument to search through.
     * @param isPointDetector : True if this is a point detector. Used to guess a name.
     * @return The component : The component object found.
     */
    boost::shared_ptr<const Mantid::Geometry::IComponent> ReflectometryReductionOne::getDetectorComponent(Mantid::Geometry::Instrument_const_sptr inst, const bool isPointDetector)
    {
      std::string componentToCorrect = isPointDetector ? "point-detector" : "line-detector";
      if(!isPropertyDefault("DetectorComponentName"))
      {
        componentToCorrect = this->getPropertyValue("DetectorComponentName");
      }
      boost::shared_ptr<const IComponent>  searchResult = inst->getComponentByName(componentToCorrect);
      if(searchResult == NULL)
      {
        throw std::invalid_argument(componentToCorrect + " does not exist. Check input properties.");
      }
      return searchResult;
    }

    /**
     * Sum spectra over a specified range.
     * @param inWS
     * @param startIndex
     * @param endIndex
     * @return Workspace with spectra summed over the specified range.
     */
    MatrixWorkspace_sptr ReflectometryReductionOne::sumSpectraOverRange(MatrixWorkspace_sptr inWS, const int startIndex, const int endIndex)
    {
      auto sumSpectra = this->createChildAlgorithm("SumSpectra");
      sumSpectra->initialize();
      sumSpectra->setProperty("InputWorkspace", inWS);
      sumSpectra->setProperty("StartWorkspaceIndex", startIndex);
      sumSpectra->setProperty("EndWorkspaceIndex", endIndex);
      sumSpectra->execute();
      MatrixWorkspace_sptr outWS = sumSpectra->getProperty("OutputWorkspace");
      return outWS;
    }

    //----------------------------------------------------------------------------------------------
    /** Execute the algorithm.
     */
    void ReflectometryReductionOne::exec()
    {
      MatrixWorkspace_sptr runWS = getProperty("InputWorkspace");

      OptionalMatrixWorkspace_sptr firstTransmissionRun;
      OptionalMatrixWorkspace_sptr secondTransmissionRun;
      OptionalDouble stitchingStartQ;
      OptionalDouble stitchingDeltaQ;
      OptionalDouble stitchingEndQ;
      OptionalDouble stitchingStartOverlapQ;
      OptionalDouble stitchingEndOverlapQ;

      getTransmissionRunInfo(firstTransmissionRun, secondTransmissionRun, stitchingStartQ, stitchingDeltaQ, stitchingEndQ, stitchingStartOverlapQ, stitchingEndOverlapQ);

      OptionalDouble theta;
      if (!isPropertyDefault("ThetaIn"))
      {
        double temp = this->getProperty("ThetaIn");
        theta = temp;
      }

      const std::string strAnalysisMode = getProperty("AnalysisMode");
      const bool isPointDetector = (pointDetectorAnalysis.compare(strAnalysisMode) == 0);

      const MinMax wavelengthInterval = this->getMinMax("WavelengthMin","WavelengthMax");
      const double wavelengthStep = getProperty("WavelengthStep");
      const MinMax monitorBackgroundWavelengthInterval = getMinMax("MonitorBackgroundWavelengthMin", "MonitorBackgroundWavelengthMax");
      const MinMax monitorIntegrationWavelengthInterval = getMinMax("MonitorIntegrationWavelengthMin", "MonitorIntegrationWavelengthMax");

      const WorkspaceIndexList indexList = getWorkspaceIndexList();

      OptionalWorkspaceIndexes regionOfInterest;
      fetchOptionalLowerUpperPropertyValue("RegionOfInterest", isPointDetector, regionOfInterest);

      OptionalWorkspaceIndexes directBeam;
      fetchOptionalLowerUpperPropertyValue("RegionOfDirectBeam", isPointDetector, directBeam);

      const int i0MonitorIndex = getProperty("I0MonitorIndex");

      const bool correctDetctorPositions = getProperty("CorrectDetectorPositions");

      auto instrument = runWS->getInstrument();
      IComponent_const_sptr detector = this->getDetectorComponent(instrument, isPointDetector);
      IComponent_const_sptr sample = this->getSurfaceSampleComponent(instrument);

      DetectorMonitorWorkspacePair inLam = toLam(runWS, indexList, i0MonitorIndex, wavelengthInterval, monitorBackgroundWavelengthInterval, wavelengthStep);
      auto detectorWS = inLam.get<0>();
      auto monitorWS = inLam.get<1>();

      MatrixWorkspace_sptr IvsLam; // Output workspace
      MatrixWorkspace_sptr IvsQ; // Output workspace
      if(isPointDetector)
      {
        auto integrationAlg = this->createChildAlgorithm("Integration");
        integrationAlg->initialize();
        integrationAlg->setProperty("InputWorkspace", monitorWS);
        integrationAlg->setProperty("RangeLower", monitorIntegrationWavelengthInterval.get<0>());
        integrationAlg->setProperty("RangeUpper", monitorIntegrationWavelengthInterval.get<1>());
        integrationAlg->execute();
        MatrixWorkspace_sptr integratedMonitor = integrationAlg->getProperty("OutputWorkspace");

        IvsLam = detectorWS / integratedMonitor; // Normalize by the integrated monitor counts.

        if(firstTransmissionRun.is_initialized())
        {

          // Perform transmission correction.
          IvsLam = this->transmissonCorrection(IvsLam, wavelengthInterval, monitorBackgroundWavelengthInterval, monitorIntegrationWavelengthInterval,
              i0MonitorIndex, firstTransmissionRun.get(), secondTransmissionRun, stitchingStartQ, stitchingDeltaQ, stitchingEndQ, stitchingStartOverlapQ, stitchingEndOverlapQ, wavelengthStep);

        }
        else
        {
          g_log.warning("No transmission correction will be applied.");
        }
      }
      else
      {
        if(!regionOfInterest.is_initialized())
        {
          throw std::invalid_argument("RegionOfInterest must be provided for a multi-detector run.");
        }
        if(!directBeam.is_initialized())
        {
          throw std::invalid_argument("RegionOfDirectBeam must be provided for a multi-detector run.");
        }
        const WorkspaceIndexList roi = getIndexesInTermsOf(detectorWS, runWS, regionOfInterest.get());
        const WorkspaceIndexList db = getIndexesInTermsOf(detectorWS, runWS, directBeam.get());

        MatrixWorkspace_sptr regionOfInterestWS = this->sumSpectraOverRange(detectorWS, roi.front(), roi.back());
        MatrixWorkspace_sptr regionOfDirectBeamWS = this->sumSpectraOverRange(detectorWS, db.front(), db.back());

        IvsLam = regionOfInterestWS / regionOfDirectBeamWS; // TODO. This needs checking.
      }

      IvsQ = this->toIvsQ(IvsLam, correctDetctorPositions, isPointDetector, theta, sample, detector);

      setProperty("ThetaOut", theta.get());
      setProperty("OutputWorkspaceWavelength", IvsLam);
      setProperty("OutputWorkspace", IvsQ);

    }

  } // namespace Algorithms
} // namespace Mantid
