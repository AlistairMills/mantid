/*WIKI*
TODO: Enter a full wiki-markup description of your algorithm here. You can then use the Build/wiki_maker.py script to generate your full wiki page.
*WIKI*/

#include "MantidCrystal/FilterPeaks.h"
#include "MantidKernel/ListValidator.h"
#include "MantidKernel/MandatoryValidator.h"
#include "MantidDataObjects/PeaksWorkspace.h"

namespace {
  double HKL(const Mantid::DataObjects::Peak & p)
  {
    return p.getH() + p.getK() + p.getL();
  }

  double HKL2(const Mantid::DataObjects::Peak & p)
  {
    return p.getH()*p.getH() + p.getK()*p.getK() + p.getL()*p.getL();
  }

  double intensity(const Mantid::DataObjects::Peak & p)
  {
    return p.getIntensity();
  }

  double SN(const Mantid::DataObjects::Peak & p)
  {
    return p.getIntensity()/p.getSigmaIntensity();
  }
}

namespace Mantid
{
namespace Crystal
{
  // Register the algorithm into the AlgorithmFactory
  DECLARE_ALGORITHM(FilterPeaks)

  using namespace Kernel;
  using namespace API;
  using DataObjects::PeaksWorkspace;
  using DataObjects::PeaksWorkspace_const_sptr;
  using DataObjects::PeaksWorkspace_sptr;
  using DataObjects::Peak;

  /** Constructor
   */
  FilterPeaks::FilterPeaks()
  {
  }
    
  /** Destructor
   */
  FilterPeaks::~FilterPeaks()
  {
  }
  
  /// Algorithm's name for identification. @see Algorithm::name
  const std::string FilterPeaks::name() const { return "FilterPeaks";};
  /// Algorithm's version for identification. @see Algorithm::version
  int FilterPeaks::version() const { return 1;};
  /// Algorithm's category for identification. @see Algorithm::category
  const std::string FilterPeaks::category() const { return "Crystal";}

  /// Sets documentation strings for this algorithm
  void FilterPeaks::initDocs()
  {
    this->setWikiSummary("Filters the peaks in a peaks workspace based upon a chosen  ");
    this->setOptionalMessage("TODO: Enter a quick description of your algorithm.");
  }

  /** Initialize the algorithm's properties.
   */
  void FilterPeaks::init()
  {
    declareProperty(new WorkspaceProperty<PeaksWorkspace>("InputWorkspace","",Direction::Input), "The input workspace");
    declareProperty(new WorkspaceProperty<IPeaksWorkspace>("OutputWorkspace","",Direction::Output), "The filtered workspace");

    std::vector<std::string> filters;
    filters.push_back("H+K+L");
    filters.push_back("H2+K2+L2");
    filters.push_back("Intensity");
    filters.push_back("Signal/Noise");
    declareProperty("FilterVariable","",boost::make_shared<StringListValidator>(filters),"The variable on which to filter the peaks");

    declareProperty("FilterValue", EMPTY_DBL(), boost::make_shared<MandatoryValidator<double>>(),
                    "The value of the FilterVariable to compare each peak to");

    std::vector<std::string> operation;
    operation.push_back("<");
    operation.push_back(">");
    operation.push_back("=");
    operation.push_back("<=");
    operation.push_back(">=");
    declareProperty("Operator","<",boost::make_shared<StringListValidator>(operation),"");
  }

  /** Execute the algorithm.
   */
  void FilterPeaks::exec()
  {
    PeaksWorkspace_const_sptr inputWS = getProperty("InputWorkspace");

    IPeaksWorkspace_sptr filteredWS = WorkspaceFactory::Instance().createPeaks();
    // Copy over ExperimentInfo from input workspace
    filteredWS->copyExperimentInfoFrom(inputWS.get());

    // TODO: Refactor to avoid all the else-ifs and multiple edits required on adding new variables

    const std::string FilterVariable = getProperty("FilterVariable");
    double (*variable)(const Mantid::DataObjects::Peak &);
    if ( FilterVariable == "H+K+L" )
      variable = &HKL;
    else if ( FilterVariable == "H2+K2+L2" )
      variable = &HKL2;
    else if ( FilterVariable == "Intensity" )
      variable = &intensity;
    else if ( FilterVariable == "Signal/Noise" )
      variable = &SN;

    const double FilterValue = getProperty("FilterValue");
    const std::string Operator = getProperty("Operator");

    for ( int i = 0; i < inputWS->getNumberPeaks(); ++i )
    {
      bool pass;
      const Peak& currentPeak = inputWS->getPeak(i);
      const double currentValue = variable(currentPeak);

      if ( Operator == "<" ) pass = (currentValue < FilterValue);
      else if ( Operator == ">" ) pass = (currentValue > FilterValue);
      else if ( Operator == "=" ) pass = (currentValue == FilterValue);
      else if ( Operator == "<=" ) pass = (currentValue <= FilterValue);
      else if ( Operator == ">=" ) pass = (currentValue >= FilterValue);

      if ( pass ) filteredWS->addPeak(currentPeak);
    }

    setProperty("OutputWorkspace", filteredWS);
  }



} // namespace Crystal
} // namespace Mantid