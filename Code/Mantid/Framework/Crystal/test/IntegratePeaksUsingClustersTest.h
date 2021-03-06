#ifndef MANTID_CRYSTAL_INTEGRATEPEAKSUSINGCLUSTERSTEST_H_
#define MANTID_CRYSTAL_INTEGRATEPEAKSUSINGCLUSTERSTEST_H_

#include <cxxtest/TestSuite.h>

#include "MantidCrystal/IntegratePeaksUsingClusters.h"
#include "MantidTestHelpers/MDEventsTestHelper.h"
#include "MantidTestHelpers/WorkspaceCreationHelper.h"
#include "MantidTestHelpers/ComponentCreationHelper.h"
#include "MantidAPI/FrameworkManager.h"
#include "MantidAPI/AlgorithmManager.h"
#include "MantidAPI/Workspace.h"
#include "MantidKernel/V3D.h"
#include "MantidDataObjects/PeaksWorkspace.h"

#include <boost/assign/list_of.hpp>
#include <boost/tuple/tuple.hpp>
#include <set>

using namespace Mantid::Crystal;
using namespace Mantid::API;
using namespace Mantid::Kernel;
using namespace Mantid::MDEvents;
using namespace Mantid::Geometry;
using namespace Mantid::DataObjects;

namespace
{  
  // Helper typedef
  typedef boost::tuple<IMDHistoWorkspace_sptr, IPeaksWorkspace_sptr> MDHistoPeaksWSTuple;

  // Helper to determine if container holds a value.
  template <typename Container >
  bool does_contain(const Container& container, const typename Container::value_type & value)
  {
    return container.end() != std::find(container.begin(), container.end(), value);
  }

  // Add a fake peak to an MDEventWorkspace
  void add_fake_md_peak(Workspace_sptr mdws, const size_t& nEvents, const double& h, const double& k, const double& l, const double& radius)
  {
    auto fakeMDEventDataAlg = AlgorithmManager::Instance().createUnmanaged("FakeMDEventData");
    fakeMDEventDataAlg->setChild(true);
    fakeMDEventDataAlg->initialize();
    fakeMDEventDataAlg->setProperty("InputWorkspace", mdws);
    std::stringstream peakstream;
    peakstream  << nEvents << ", " << h << ", " << k << ", " << l << ", " << radius;
    fakeMDEventDataAlg->setPropertyValue("PeakParams", peakstream.str());
    fakeMDEventDataAlg->execute();
  }

  // Make a fake peaks workspace and corresponding mdhistoworkspace and return both
   MDHistoPeaksWSTuple make_peak_and_md_ws(const std::vector<V3D>& hklValuesVec, 
    const double& min, const double& max, const std::vector<double>& peakRadiusVec, 
    const std::vector<size_t>& nEventsInPeakVec, const size_t& nBins=20)
  {
    Instrument_sptr inst = ComponentCreationHelper::createTestInstrumentRectangular(1, 100, 0.05);

    // --- Make a fake md histo workspace ---
    auto mdworkspaceAlg = AlgorithmManager::Instance().createUnmanaged("CreateMDWorkspace");
    mdworkspaceAlg->setChild(true);
    mdworkspaceAlg->initialize();
    mdworkspaceAlg->setProperty("Dimensions", 3);
    std::vector<double> extents = boost::assign::list_of(min)(max)(min)(max)(min)(max).convert_to_container<std::vector<double> >();
    mdworkspaceAlg->setProperty("Extents", extents);
    mdworkspaceAlg->setPropertyValue("Names", "H,K,L");
    mdworkspaceAlg->setPropertyValue("Units", "-,-,-");
    mdworkspaceAlg->setPropertyValue("OutputWorkspace", "IntegratePeaksMDTest_MDEWS");
    mdworkspaceAlg->execute();
    Workspace_sptr mdws = mdworkspaceAlg->getProperty("OutputWorkspace");

    // --- Set speical coordinates on fake mdworkspace --
    auto coordsAlg = AlgorithmManager::Instance().createUnmanaged("SetSpecialCoordinates");
    coordsAlg->setChild(true);
    coordsAlg->initialize();
    coordsAlg->setProperty("InputWorkspace", mdws);
    coordsAlg->setProperty("SpecialCoordinates", "HKL");
    coordsAlg->execute();

    // --- Make a fake PeaksWorkspace ---
    IPeaksWorkspace_sptr peakWS(new PeaksWorkspace());
    peakWS->setInstrument(inst);

    // --- Set speical coordinates on fake PeaksWorkspace --
    coordsAlg = AlgorithmManager::Instance().createUnmanaged("SetSpecialCoordinates");
    coordsAlg->setChild(true);
    coordsAlg->initialize();
    coordsAlg->setProperty("InputWorkspace", peakWS);
    coordsAlg->setProperty("SpecialCoordinates", "HKL");
    coordsAlg->execute();

    for(size_t i = 0; i<hklValuesVec.size(); ++i)
    {
      Peak peak(inst, 15050, 1.0);

      const double& h = hklValuesVec[i][0];
      const double& k = hklValuesVec[i][1];
      const double& l = hklValuesVec[i][2];

      peak.setHKL(h, k, l);
      peakWS->addPeak(peak);

      add_fake_md_peak(mdws, nEventsInPeakVec[i], h, k, l, peakRadiusVec[i]);
    }

    auto binMDAlg = AlgorithmManager::Instance().createUnmanaged("BinMD");
    binMDAlg->setChild(true);
    binMDAlg->initialize();
    binMDAlg->setProperty("InputWorkspace", mdws);
    binMDAlg->setPropertyValue("OutputWorkspace", "output_ws");
    binMDAlg->setProperty("AxisAligned", true);

    std::stringstream dimensionstring;
    dimensionstring  << "," << min << ", " << max << "," << nBins ;

    binMDAlg->setPropertyValue("AlignedDim0", "H" + dimensionstring.str());
    binMDAlg->setPropertyValue("AlignedDim1", "K" + dimensionstring.str());
    binMDAlg->setPropertyValue("AlignedDim2", "L" + dimensionstring.str());
    binMDAlg->execute();

    Workspace_sptr temp = binMDAlg->getProperty("OutputWorkspace");
    IMDHistoWorkspace_sptr outMDWS = boost::dynamic_pointer_cast<IMDHistoWorkspace>(temp);
    return MDHistoPeaksWSTuple(outMDWS, peakWS);
  }

      // Make a fake peaks workspace and corresponding mdhistoworkspace and return both
 MDHistoPeaksWSTuple make_peak_and_md_ws(const std::vector<V3D>& hklValues, 
    const double& min, const double& max, const double& peakRadius=1, 
    const size_t nEventsInPeak=1000, const size_t& nBins=20)
  {
    std::vector<size_t> nEventsInPeakVec(hklValues.size(), nEventsInPeak);
    std::vector<double> peakRadiusVec(hklValues.size(), peakRadius);
    return make_peak_and_md_ws(hklValues, min, max, peakRadiusVec, nEventsInPeakVec, nBins);
  }

  // Execute the clustering integration algorithm
  MDHistoPeaksWSTuple execute_integration(const MDHistoPeaksWSTuple& inputWorkspaces, const double& peakRadius, const double& threshold)
  {
    auto mdWS = inputWorkspaces.get<0>();
    auto peaksWS = inputWorkspaces.get<1>();
    // ------- Integrate the fake data
    IntegratePeaksUsingClusters alg;
    alg.initialize();
    alg.setRethrows(true);
    alg.setChild(true);
    alg.setProperty("InputWorkspace", mdWS);
    alg.setProperty("PeaksWorkspace", peaksWS);
    alg.setProperty("Threshold", threshold);
    alg.setProperty("RadiusEstimate", peakRadius + 0.1);
    alg.setPropertyValue("OutputWorkspace", "out_ws");
    alg.setPropertyValue("OutputWorkspaceMD", "out_ws_md");
    alg.execute();
    // ------- Get the integrated results
    IPeaksWorkspace_sptr outPeaksWS = alg.getProperty("OutputWorkspace");
    IMDHistoWorkspace_sptr outClustersWS = alg.getProperty("OutputWorkspaceMD");
    return MDHistoPeaksWSTuple(outClustersWS, outPeaksWS);
  }
}

//=====================================================================================
// Functional Tests
//=====================================================================================
class IntegratePeaksUsingClustersTest : public CxxTest::TestSuite
{

public:
  // This pair of boilerplate methods prevent the suite being created statically
  // This means the constructor isn't called when running other tests
  static IntegratePeaksUsingClustersTest *createSuite() { return new IntegratePeaksUsingClustersTest(); }
  static void destroySuite( IntegratePeaksUsingClustersTest *suite ) { delete suite; }

  IntegratePeaksUsingClustersTest()
  {
    FrameworkManager::Instance();
  }

  void test_Init()
  {
    IntegratePeaksUsingClusters alg;
    TS_ASSERT_THROWS_NOTHING( alg.initialize() )
    TS_ASSERT( alg.isInitialized() )
  }

  void test_peaks_workspace_mandatory()
  {
    IMDHistoWorkspace_sptr mdws = MDEventsTestHelper::makeFakeMDHistoWorkspace(1,1);

    IntegratePeaksUsingClusters alg; 
    alg.setRethrows(true);
    alg.initialize();
    alg.setProperty("InputWorkspace", mdws);
    alg.setPropertyValue("OutputWorkspaceMD", "out_md");
    alg.setPropertyValue("OutputWorkspace", "out_peaks");
    TSM_ASSERT_THROWS("PeaksWorkspace required", alg.execute(), std::runtime_error&);
  }

  void test_input_md_workspace_mandatory()
  {
    auto peaksws = WorkspaceCreationHelper::createPeaksWorkspace();

    IntegratePeaksUsingClusters alg; 
    alg.setRethrows(true);
    alg.initialize();
    alg.setProperty("PeaksWorkspace", peaksws);
    alg.setPropertyValue("OutputWorkspaceMD", "out_md");
    alg.setPropertyValue("OutputWorkspace", "out_peaks");
    TSM_ASSERT_THROWS("InputWorkspace required", alg.execute(), std::runtime_error&);
  }

  void test_throw_if_special_coordinates_unknown()
  {
    auto peaksws = WorkspaceCreationHelper::createPeaksWorkspace();
    IMDHistoWorkspace_sptr mdws = MDEventsTestHelper::makeFakeMDHistoWorkspace(1,1);

    IntegratePeaksUsingClusters alg; 
    alg.setRethrows(true);
    alg.initialize();
    alg.setProperty("InputWorkspace", mdws);
    alg.setProperty("PeaksWorkspace", peaksws);
    alg.setPropertyValue("OutputWorkspaceMD", "out_md");
    alg.setPropertyValue("OutputWorkspace", "out_peaks");
    TSM_ASSERT_THROWS("Unknown special coordinates", alg.execute(), std::invalid_argument&);
  }

  void test_threshold_too_high_gives_no_peaks()
  {
    // ------- Make the fake input
    std::vector<V3D> hklValues;
    // Add a single peak.
    hklValues.push_back(V3D(2,2,2)); 
    const double peakRadius = 1;
    const double threshold = 10000; // Threshold will filter out everything given the nEventsInPeak restriction.
    const size_t nEventsInPeak = 10000;
    MDHistoPeaksWSTuple inputWorkspaces = make_peak_and_md_ws(hklValues, -10, 10, peakRadius, nEventsInPeak);
    //-------- Execute the integratioin
    MDHistoPeaksWSTuple integratedWorkspaces = execute_integration(inputWorkspaces, peakRadius, threshold);
    // ------- Get the integrated results
    IMDHistoWorkspace_sptr outClustersWS = integratedWorkspaces.get<0>();
    IPeaksWorkspace_sptr outPeaksWS = integratedWorkspaces.get<1>();
    
    std::set<Mantid::signal_t> labelIds;
    for(size_t i = 0; i < outClustersWS->getNPoints(); ++i)
    {
      labelIds.insert(outClustersWS->getSignalAt(i));
    }
    TSM_ASSERT_EQUALS("Should only have one type of label", 1, labelIds.size());
    TSM_ASSERT("Should have 'empy' label", does_contain(labelIds, 0));

    TSM_ASSERT_EQUALS("Integrated intensity should be zero since no integration has occured", 0, outPeaksWS->getPeak(0).getIntensity());
    TSM_ASSERT_EQUALS("Integrated intensity should be zero since no integration has occured", 0, outPeaksWS->getPeak(0).getSigmaIntensity());
  }

  void test_integrate_single_peak()
  {
    // ------- Make the fake input
    std::vector<V3D> hklValues;
    // Add a single peak.
    hklValues.push_back(V3D(2,2,2)); 
    const double peakRadius = 1;
    const double threshold = 100;
    const size_t nEventsInPeak = 10000;
    MDHistoPeaksWSTuple inputWorkspaces = make_peak_and_md_ws(hklValues, -10, 10, peakRadius, nEventsInPeak);
    //-------- Execute the integratioin
    MDHistoPeaksWSTuple integratedWorkspaces = execute_integration(inputWorkspaces, peakRadius, threshold);
    // ------- Get the integrated results
    IMDHistoWorkspace_sptr outClustersWS = integratedWorkspaces.get<0>();
    IPeaksWorkspace_sptr outPeaksWS = integratedWorkspaces.get<1>();
    
    // ------- Check the results.
    // Basic checks
    auto mdWS = inputWorkspaces.get<0>();
    auto peaksWS = inputWorkspaces.get<1>();
    TS_ASSERT_EQUALS(outPeaksWS->getNumberPeaks(), peaksWS->getNumberPeaks());
    TS_ASSERT_EQUALS(mdWS->getNPoints(), outClustersWS->getNPoints());
    // Check clusters by extracting unique label ids. 
    std::set<Mantid::signal_t> labelIds;
    for(size_t i = 0; i < outClustersWS->getNPoints(); ++i)
    {
      labelIds.insert(outClustersWS->getSignalAt(i));
    }
    TSM_ASSERT_EQUALS("Only one peak present, so should only have two unique label ids", 2, labelIds.size());

    TSM_ASSERT("Should have 'empy' label", does_contain(labelIds, 0));
    TSM_ASSERT("Should have non-empy label", does_contain(labelIds, 1));
  }

  void test_integrate_two_separte_but_identical_peaks()
  {
    // ------- Make the fake input
    std::vector<V3D> hklValues;
    // Add several peaks. These are NOT overlapping.
    hklValues.push_back(V3D(1,1,1)); 
    hklValues.push_back(V3D(6,6,6));
    const double peakRadius = 1;
    const double threshold = 100;
    const size_t nEventsInPeak = 10000;
    MDHistoPeaksWSTuple inputWorkspaces = make_peak_and_md_ws(hklValues, -10, 10, peakRadius, nEventsInPeak);
    //-------- Execute the integratioin
    MDHistoPeaksWSTuple integratedWorkspaces = execute_integration(inputWorkspaces, peakRadius, threshold);
    // ------- Get the integrated results
    IMDHistoWorkspace_sptr outClustersWS = integratedWorkspaces.get<0>();
    IPeaksWorkspace_sptr outPeaksWS = integratedWorkspaces.get<1>();
    
    // ------- Check the results.
    // Basic checks
    auto mdWS = inputWorkspaces.get<0>();
    auto peaksWS = inputWorkspaces.get<1>();
    TS_ASSERT_EQUALS(outPeaksWS->getNumberPeaks(), peaksWS->getNumberPeaks());
    TS_ASSERT_EQUALS(mdWS->getNPoints(), outClustersWS->getNPoints());
    // Check clusters by extracting unique label ids. 
    std::set<Mantid::signal_t> labelIds;
    for(size_t i = 0; i < outClustersWS->getNPoints(); ++i)
    {
      labelIds.insert(outClustersWS->getSignalAt(i));
    }
    TSM_ASSERT_EQUALS("N peaks present, so should only have n+1 unique label ids", 3, labelIds.size());

    TSM_ASSERT("Should have 'empy' label", does_contain(labelIds, 0));
    TSM_ASSERT("Should have non-empy label", does_contain(labelIds, 1));
    TSM_ASSERT("Should have non-empy label", does_contain(labelIds, 2));

    // Two peaks are identical, so integrated values should be the same.
    TSM_ASSERT("Integrated intensity should be greater than zero", outPeaksWS->getPeak(0).getIntensity() > 0);
    TSM_ASSERT("Integrated error should be greater than zero", outPeaksWS->getPeak(0).getSigmaIntensity() > 0);
    TSM_ASSERT_EQUALS("Peaks are identical, so integrated values should be identical", outPeaksWS->getPeak(0).getIntensity(), outPeaksWS->getPeak(1).getIntensity());
    TSM_ASSERT_EQUALS("Peaks are identical, so integrated error values should be identical", outPeaksWS->getPeak(0).getSigmaIntensity(), outPeaksWS->getPeak(1).getSigmaIntensity());
  }

  void test_integrate_two_peaks_of_different_magnitude()
  {
    // ------- Make the fake input
    std::vector<V3D> hklValues;
    // Add several peaks. These are NOT overlapping.
    hklValues.push_back(V3D(1,1,1)); 
    hklValues.push_back(V3D(6,6,6));
    const double peakRadius = 1;
    const double threshold = 100;
    std::vector<size_t> nEventsInPeakVec;
    nEventsInPeakVec.push_back(10000);
    nEventsInPeakVec.push_back(20000); // Second peak has DOUBLE the intensity of the firse one.

    MDHistoPeaksWSTuple inputWorkspaces = make_peak_and_md_ws(hklValues, -10, 10, std::vector<double>(hklValues.size(), peakRadius), nEventsInPeakVec);
    //-------- Execute the integratioin
    MDHistoPeaksWSTuple integratedWorkspaces = execute_integration(inputWorkspaces, peakRadius, threshold);
    // ------- Get the integrated results
    IMDHistoWorkspace_sptr outClustersWS = integratedWorkspaces.get<0>();
    IPeaksWorkspace_sptr outPeaksWS = integratedWorkspaces.get<1>();
    
    // ------- Check the results.
    // Basic checks
    auto mdWS = inputWorkspaces.get<0>();
    auto peaksWS = inputWorkspaces.get<1>();
    TS_ASSERT_EQUALS(outPeaksWS->getNumberPeaks(), peaksWS->getNumberPeaks());
    TS_ASSERT_EQUALS(mdWS->getNPoints(), outClustersWS->getNPoints());
    // Check clusters by extracting unique label ids. 
    std::set<Mantid::signal_t> labelIds;
    for(size_t i = 0; i < outClustersWS->getNPoints(); ++i)
    {
      labelIds.insert(outClustersWS->getSignalAt(i));
    }
    TSM_ASSERT_EQUALS("N peaks present, so should only have n+1 unique label ids", 3, labelIds.size());

    TSM_ASSERT("Should have 'empy' label", does_contain(labelIds, 0));
    TSM_ASSERT("Should have non-empy label", does_contain(labelIds, 1));
    TSM_ASSERT("Should have non-empy label", does_contain(labelIds, 2));

    // Two peaks are identical, so integrated values should be the same.
    TSM_ASSERT("Integrated intensity should be greater than zero", outPeaksWS->getPeak(0).getIntensity() > 0);
    TSM_ASSERT("Integrated error should be greater than zero", outPeaksWS->getPeak(0).getSigmaIntensity() > 0);
    TSM_ASSERT_EQUALS("Second peak is twice as 'bright'", outPeaksWS->getPeak(0).getIntensity() * 2, outPeaksWS->getPeak(1).getIntensity());
    TSM_ASSERT_EQUALS("Second peak is twice as 'bright'", outPeaksWS->getPeak(0).getSigmaIntensity() * 2, outPeaksWS->getPeak(1).getSigmaIntensity());
  }

};

//=====================================================================================
// Performance Tests
//=====================================================================================
class IntegratePeaksUsingClustersTestPerformance : public CxxTest::TestSuite
{

private:

  // Input data
  MDHistoPeaksWSTuple m_inputWorkspaces;
  double m_peakRadius;
  double m_threshold;

public:
  // This pair of boilerplate methods prevent the suite being created statically
  // This means the constructor isn't called when running other tests
  static IntegratePeaksUsingClustersTestPerformance *createSuite() { return new IntegratePeaksUsingClustersTestPerformance(); }
  static void destroySuite( IntegratePeaksUsingClustersTestPerformance *suite ) { delete suite; }

  IntegratePeaksUsingClustersTestPerformance()
  {
    FrameworkManager::Instance();
   
    std::vector<V3D> hklValues;
    for(double i = -10; i < 10; i+=4)
    {
      for(double j = -10; j < 10; j+=4)
      {
        for(double k = -10; k < 10; k+=4)
        {
          hklValues.push_back(V3D(i,j,k));
        }
      }
    }
    
    m_peakRadius = 1;
    m_threshold = 10;
    const size_t nEventsInPeak = 1000;
    m_inputWorkspaces = make_peak_and_md_ws(hklValues, -10, 10, m_peakRadius, nEventsInPeak, 50);
    
  }

  void test_execute()
  {
    // Just run the integration. Functional tests handled in separate suite.
    execute_integration(m_inputWorkspaces, m_peakRadius, m_threshold);
  }

};


#endif /* MANTID_CRYSTAL_INTEGRATEPEAKSUSINGCLUSTERSTEST_H_ */