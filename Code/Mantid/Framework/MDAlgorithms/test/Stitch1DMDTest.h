#ifndef MANTID_MDALGORITHMS_STITCH1DTEST_H_
#define MANTID_MDALGORITHMS_STITCH1DTEST_H_

#include <cxxtest/TestSuite.h>

#include "MantidMDAlgorithms/Stitch1DMD.h"
#include "MantidAPI/FrameworkManager.h"
#include "MantidAPI/IMDHistoWorkspace.h"
#include <boost/assign.hpp>
#include <boost/make_shared.hpp>

using Mantid::MDAlgorithms::Stitch1DMD;

using namespace Mantid::API;
using namespace Mantid::Kernel;

class Stitch1DMDTest : public CxxTest::TestSuite
{

private:

  // Helper method to fabricate multi-period group workspaces.
  void add_periods_logs(WorkspaceGroup_sptr ws)
  {
    int nperiods = static_cast<int>(ws->size());
    for(size_t i = 0; i < ws->size(); ++i)
    { 
      MatrixWorkspace_sptr currentWS = boost::dynamic_pointer_cast<MatrixWorkspace>(ws->getItem(i));

      PropertyWithValue<int>* nperiodsProp = new PropertyWithValue<int>("nperiods", nperiods);
      currentWS->mutableRun().addLogData(nperiodsProp);
      PropertyWithValue<int>* currentPeriodsProp = new PropertyWithValue<int>("current_period", static_cast<int>(i+1));
      currentWS->mutableRun().addLogData(currentPeriodsProp);
    }
  }

  // Helper method to make the stitch group 1D algorithm
  IAlgorithm_sptr make_algorithm(Workspace_sptr lhsWorkspace, Workspace_sptr rhsWorkspace, const std::string& outputWorkspaceName, const double& startOverlap, const double& endOverlap, const bool scaleRHSWS = true)
  {
      IAlgorithm_sptr alg = boost::make_shared<Stitch1DMD>();
      alg->setRethrows(true);
      alg->initialize();

      alg->setProperty("LHSWorkspace", lhsWorkspace);
      alg->setProperty("RHSWorkspace", rhsWorkspace);
      alg->setPropertyValue("OutputWorkspace", outputWorkspaceName);
      alg->setProperty("StartOverlap", startOverlap);
      alg->setProperty("EndOverlap", endOverlap);
      alg->setProperty("ScaleRHSWorkspace", scaleRHSWS);
      return alg;
  }

  // Helper method to make the stitch group 1D algorithm
  IAlgorithm_sptr make_algorithm(const std::string& lhsWorkspaceName, const std::string& rhsWorkspaceName, const std::string& outputWorkspaceName, const double& startOverlap, const double& endOverlap, const bool scaleRHSWS = true)
  {
    auto lhsWS = Mantid::API::AnalysisDataService::Instance().retrieve(lhsWorkspaceName);
    auto rhsWS = Mantid::API::AnalysisDataService::Instance().retrieve(rhsWorkspaceName);
    return make_algorithm(lhsWS, rhsWS, outputWorkspaceName, startOverlap, endOverlap, scaleRHSWS);
  }


public:
  // This pair of boilerplate methods prevent the suite being created statically
  // This means the constructor isn't called when running other tests
  static Stitch1DMDTest *createSuite() { return new Stitch1DMDTest(); }
  static void destroySuite( Stitch1DMDTest *suite ) { delete suite; }

    std::string __good_workspace_name;
    std::string __bad_type_of_workspace_name;
    std::string __three_dim_workspace_name;
    std::string __integrated_two_dim_workspace_name;
    std::string __unintegrated_two_dim_workspace_name;

    Stitch1DMDTest()
    {
      Mantid::API::FrameworkManagerImpl& frameworkManager = Mantid::API::FrameworkManager::Instance();

      // Create a workspace of the wrong type
      Mantid::API::IAlgorithm* bad_md_workspace_alg = frameworkManager.exec("CreateMDWorkspace", "Extents=0,1;Names=A;Units=U;OutputWorkspace=Stitch1DMD_test_workspace_1");
      this->__bad_type_of_workspace_name = bad_md_workspace_alg->getPropertyValue("OutputWorkspace");

      // Create a workspace that is of the right type and shape
      Mantid::API::IAlgorithm* good_workspace_alg = frameworkManager.exec("CreateMDHistoWorkspace", "SignalInput=1,2;ErrorInput=1,2;Dimensionality=2;Extents=-1,1,-1,1;NumberOfBins=2,1;Names=A,B;Units=U1,U2;OutputWorkspace=Stitch1DMD_test_workspace_2");
      this->__good_workspace_name = good_workspace_alg->getPropertyValue("OutputWorkspace");

      // Create a workspace that is of the right type, but the wrong shape
      Mantid::API::IAlgorithm* three_dim_alg = frameworkManager.exec("CreateMDHistoWorkspace","SignalInput=1;ErrorInput=1;Dimensionality=3;Extents=-1,1,-1,1,-1,1;NumberOfBins=1,1,1;Names=A,B,C;Units=U1,U2,U3;OutputWorkspace=Stitch1DMD_test_workspace_3");
      this->__three_dim_workspace_name = three_dim_alg->getPropertyValue("OutputWorkspace");

      // Create a workspace that is of the right type and shape, but wrong size, with 1 bin in each dimension (completely integrated).
      Mantid::API::IAlgorithm* integrated_two_dim_alg = frameworkManager.exec("CreateMDHistoWorkspace","SignalInput=1;ErrorInput=1;Dimensionality=2;Extents=-1,1,-1,1;NumberOfBins=1,1;Names=A,B;Units=U1,U2;OutputWorkspace=Stitch1DMD_test_workspace_4");
      this->__integrated_two_dim_workspace_name = integrated_two_dim_alg->getPropertyValue("OutputWorkspace");

      // Create a workspace that is of the right type and shape, but wrong size, with more than one bin in both dimensions (completely unintegrated).
      Mantid::API::IAlgorithm* unintegrated_two_dim_alg = frameworkManager.exec("CreateMDHistoWorkspace","SignalInput=1,1,1,1;ErrorInput=1,1,1,1;Dimensionality=2;Extents=-1,1,-1,1;NumberOfBins=2,2;Names=A,B;Units=U1,U2;OutputWorkspace=Stitch1DMD_test_workspace_5");
      this->__unintegrated_two_dim_workspace_name = unintegrated_two_dim_alg->getPropertyValue("OutputWorkspace");
    }

    virtual ~Stitch1DMDTest()
    {
      Mantid::API::AnalysisDataService::Instance().clear();
    }

    void test_Init()
    {
      Stitch1DMD alg;
      TS_ASSERT_THROWS_NOTHING( alg.initialize() )
      TS_ASSERT( alg.isInitialized() )
    }

    void test_does_not_accept_mdeventworkspaces_for_lhsworkspace()
    {
      Stitch1DMD alg;
      alg.setRethrows(true);
      alg.initialize();

      TS_ASSERT_THROWS(alg.setPropertyValue("LHSWorkspace", this->__bad_type_of_workspace_name), std::invalid_argument);
    }

    void test_does_not_accept_mdeventworkspaces_for_rhsworkspace()
    {
      Stitch1DMD alg;
      alg.setRethrows(true);
      alg.initialize();

      TS_ASSERT_THROWS(alg.setPropertyValue("RHSWorkspace", this->__bad_type_of_workspace_name), std::invalid_argument);
    }

    void test_lhsworkspace_with_three_input_dimensions_throws()
    {
      IAlgorithm_sptr alg = make_algorithm(this->__three_dim_workspace_name, this->__good_workspace_name, "converted", 0.0, 0.3);

      TS_ASSERT_THROWS(alg->execute(), std::runtime_error);
    }

    void test_rhsworkspace_with_three_input_dimensions_throws()
    {
      IAlgorithm_sptr alg = make_algorithm(this->__good_workspace_name, this->__three_dim_workspace_name, "converted", 0.0, 0.3);

      TS_ASSERT_THROWS(alg->execute(), std::runtime_error);
    }

    void test_lhsworkspace_with_two_integrated_input_dimensions_throws()
    {
      IAlgorithm_sptr alg = make_algorithm(this->__integrated_two_dim_workspace_name, this->__good_workspace_name, "converted", 0.0, 0.3);

      TS_ASSERT_THROWS(alg->execute(), std::runtime_error);
    }

    void test_rhsworkspace_with_two_integrated_input_dimensions_throws()
    {
      IAlgorithm_sptr alg = make_algorithm(this->__good_workspace_name, this->__integrated_two_dim_workspace_name, "converted", 0.0, 0.3);

      TS_ASSERT_THROWS(alg->execute(), std::runtime_error);
    }

    void test_lhsworkspace_with_two_non_integrated_dimensions_throws()
    {
      IAlgorithm_sptr alg = make_algorithm(this->__unintegrated_two_dim_workspace_name, this->__good_workspace_name, "converted", 0.0, 0.3);

      TS_ASSERT_THROWS(alg->execute(), std::runtime_error);
    }

    void test_rhsworkspace_with_two_non_integrated_dimensions_throws()
    {
      IAlgorithm_sptr alg = make_algorithm(this->__good_workspace_name, this->__unintegrated_two_dim_workspace_name, "converted", 0.0, 0.3);

      TS_ASSERT_THROWS(alg->execute(), std::runtime_error);
    }

    void test_lhsworkspace_and_rhsworkspace_have_different_binning_throws()
    {
      Mantid::API::FrameworkManagerImpl& frameworkManager = Mantid::API::FrameworkManager::Instance();

      Mantid::API::IAlgorithm* algA = frameworkManager.exec("CreateMDHistoWorkspace", "SignalInput=1,2;ErrorInput=1,1;Dimensionality=2;Extents=-1,1,-1,1;NumberOfBins=2,1;Names=A,B;Units=U1,U2;OutputWorkspace=Stitch1DMD_test_workspace_A");
      Mantid::API::IAlgorithm* algB = frameworkManager.exec("CreateMDHistoWorkspace", "SignalInput=1,2,3;ErrorInput=1,1,1;Dimensionality=2;Extents=-1,1,-1,1;NumberOfBins=3,1;Names=A,B;Units=U1,U2;OutputWorkspace=Stitch1DMD_test_workspace_B");

      IMDHistoWorkspace_sptr lhsWorkspace = AnalysisDataService::Instance().retrieveWS<IMDHistoWorkspace>(algA->getPropertyValue("OutputWorkspace"));
      IMDHistoWorkspace_sptr rhsWorkspace = AnalysisDataService::Instance().retrieveWS<IMDHistoWorkspace>(algB->getPropertyValue("OutputWorkspace"));

      IAlgorithm_sptr alg = make_algorithm(lhsWorkspace, rhsWorkspace, "converted", 0.0, 0.3);
        
      TS_ASSERT_THROWS(alg->execute(), std::runtime_error);

      Mantid::API::AnalysisDataService::Instance().remove(algA->getPropertyValue("OutputWorkspace"));
      Mantid::API::AnalysisDataService::Instance().remove(algB->getPropertyValue("OutputWorkspace"));
    }

    void do_test_permitted_dimensionalities(IMDHistoWorkspace_sptr a, IMDHistoWorkspace_sptr  b)
    {
      IAlgorithm_sptr alg = make_algorithm(a, b, "converted", 0.0, 0.3);

      TS_ASSERT_THROWS_NOTHING(alg->execute());
    }

    void test_can_have_single_1d_input_workspaces()
    {
      Mantid::API::FrameworkManagerImpl& frameworkManager = Mantid::API::FrameworkManager::Instance();

      // Create a one-d input workspace with 3 bins
      auto algA = frameworkManager.exec("CreateMDHistoWorkspace", "SignalInput=1,2,3,4,5,6,7,8,9,10;ErrorInput=1,1,1,1,1,1,1,1,1,1;Dimensionality=1;Extents=-1,1;NumberOfBins=10;Names=A;Units=U1;OutputWorkspace=Stitch1DMD_test_workspace_A");
      // Create a two-d input workspace with 3 * 1 bins.
      auto algB = frameworkManager.exec("CreateMDHistoWorkspace", "SignalInput=1,2,3,4,5,6,7,8,9,10;ErrorInput=1,1,1,1,1,1,1,1,1,1;Dimensionality=2;Extents=-1,1,-1,1;NumberOfBins=10,1;Names=A,B;Units=U1,U2;OutputWorkspace=Stitch1DMD_test_workspace_B");

      IMDHistoWorkspace_sptr lhsWorkspace = AnalysisDataService::Instance().retrieveWS<IMDHistoWorkspace>(algA->getPropertyValue("OutputWorkspace"));
      IMDHistoWorkspace_sptr rhsWorkspace = AnalysisDataService::Instance().retrieveWS<IMDHistoWorkspace>(algB->getPropertyValue("OutputWorkspace"));

      // Test with LHS as one dimensional and RHS as two dimensional
      do_test_permitted_dimensionalities(lhsWorkspace, rhsWorkspace);

      // Test with RHS as one dimensional and LHS as two dimensional
      do_test_permitted_dimensionalities(rhsWorkspace, lhsWorkspace);

      Mantid::API::AnalysisDataService::Instance().remove(algA->getPropertyValue("OutputWorkspace"));
      Mantid::API::AnalysisDataService::Instance().remove(algB->getPropertyValue("OutputWorkspace"));
    }

    void test_can_have_both_input_workspaces_as_1d()
    {
      Mantid::API::FrameworkManagerImpl& frameworkManager = Mantid::API::FrameworkManager::Instance();

      // Create a one-d input workspace with 3 bins
      auto algA = frameworkManager.exec("CreateMDHistoWorkspace", "SignalInput=1,2,3,4,5,6,7,8,9,10;ErrorInput=1,1,1,1,1,1,1,1,1,1;Dimensionality=1;Extents=-1,1;NumberOfBins=10;Names=A;Units=U1;OutputWorkspace=Stitch1DMD_test_workspace_A");
      // Create a two-d input workspace with 3 * 1 bins.
      auto algB = frameworkManager.exec("CreateMDHistoWorkspace", "SignalInput=1,2,3,4,5,6,7,8,9,10;ErrorInput=1,1,1,1,1,1,1,1,1,1;Dimensionality=1;Extents=-1,1;NumberOfBins=10;Names=A;Units=U1;OutputWorkspace=Stitch1DMD_test_workspace_B");

      IMDHistoWorkspace_sptr lhsWorkspace = AnalysisDataService::Instance().retrieveWS<IMDHistoWorkspace>(algA->getPropertyValue("OutputWorkspace"));
      IMDHistoWorkspace_sptr rhsWorkspace = AnalysisDataService::Instance().retrieveWS<IMDHistoWorkspace>(algB->getPropertyValue("OutputWorkspace"));

      // Test with LHS as one dimensional and RHS as two dimensional
      do_test_permitted_dimensionalities(lhsWorkspace, rhsWorkspace);

      // Test with RHS as one dimensional and LHS as two dimensional
      do_test_permitted_dimensionalities(rhsWorkspace, lhsWorkspace);

      Mantid::API::AnalysisDataService::Instance().remove(algA->getPropertyValue("OutputWorkspace"));
      Mantid::API::AnalysisDataService::Instance().remove(algB->getPropertyValue("OutputWorkspace"));
    }

    void do_test_ws1_and_ws2_have_different_dimension_names_throws(const std::string& ws1_dim_names, const std::string& ws2_dim_names)
    {
      Mantid::API::FrameworkManagerImpl& frameworkManager = Mantid::API::FrameworkManager::Instance();

      // Create Workspace with dim names in ws1_dim_names
      auto algA = frameworkManager.exec("CreateMDHistoWorkspace","SignalInput=1,1;ErrorInput=1,1;Dimensionality=2;Extents=-1,1,-1,1;NumberOfBins=2,1;Names=" + ws1_dim_names + ";Units=U1,U2;OutputWorkspace=Stitch1DMD_test_workspace_C");
      // Create Workspace with dim names in ws2_dim_names
      auto algB = frameworkManager.exec("CreateMDHistoWorkspace","SignalInput=1,1;ErrorInput=1,1;Dimensionality=2;Extents=-1,1,-1,1;NumberOfBins=2,1;Names=" + ws2_dim_names + ";Units=U1,U2;OutputWorkspace=Stitch1DMD_test_workspace_D");

      IMDHistoWorkspace_sptr a = AnalysisDataService::Instance().retrieveWS<IMDHistoWorkspace>(algA->getPropertyValue("OutputWorkspace"));
      IMDHistoWorkspace_sptr b = AnalysisDataService::Instance().retrieveWS<IMDHistoWorkspace>(algB->getPropertyValue("OutputWorkspace"));

      IAlgorithm_sptr alg = make_algorithm(a, b, "converted", 0.0, 0.3);

      TS_ASSERT_THROWS(alg->execute(), std::runtime_error);

      Mantid::API::AnalysisDataService::Instance().remove(algA->getPropertyValue("OutputWorkspace"));
      Mantid::API::AnalysisDataService::Instance().remove(algB->getPropertyValue("OutputWorkspace"));
    }

    void test_ws1_and_ws2_dim1_have_different_dimension_names_throws()
    {
      do_test_ws1_and_ws2_have_different_dimension_names_throws("A1, B1", "A2, B1");
    }

    void test_ws1_and_ws2_dim2_have_different_dimension_names_throws()
    {
      do_test_ws1_and_ws2_have_different_dimension_names_throws("A1, B1", "A1, B2");
    }

    void test_start_overlap_too_low()
    {
      Stitch1DMD alg;
      alg.setRethrows(true);
      alg.initialize();

      TS_ASSERT_THROWS(alg.setProperty("StartOverlap", -1), std::invalid_argument);
    }

    void test_start_overlap_too_high()
    {
      Stitch1DMD alg;
      alg.setRethrows(true);
      alg.initialize();

      TS_ASSERT_THROWS(alg.setProperty("StartOverlap", 1.001), std::invalid_argument);
    }

    void test_end_overlap_too_low()
    {
      Stitch1DMD alg;
      alg.setRethrows(true);
      alg.initialize();

      TS_ASSERT_THROWS(alg.setProperty("EndOverlap", -1), std::invalid_argument);
    }

    void test_end_overlap_too_high()
    {
      Stitch1DMD alg;
      alg.setRethrows(true);
      alg.initialize();

      TS_ASSERT_THROWS(alg.setProperty("EndOverlap", 1.001), std::invalid_argument);
    }

    void test_end_overlap_equal_to_start_overlap_throws()
    {
      IAlgorithm_sptr alg = make_algorithm(this->__good_workspace_name, this->__good_workspace_name, "converted", 0.5, 0.5);

      TS_ASSERT_THROWS(alg->execute(), std::runtime_error);
    }

    void test_calculates_scaling_factor_correctly()
    { 
      Mantid::API::FrameworkManagerImpl& frameworkManager = Mantid::API::FrameworkManager::Instance();

      //Signal = 1, 1, 1, but only the middle to the end of the range is marked as overlap, so only 1, 1 used.
      auto algA = frameworkManager.exec("CreateMDHistoWorkspace", "SignalInput=1,1,1;ErrorInput=1,1,1;Dimensionality=2;Extents=-1,1,-1,1;NumberOfBins=3,1;Names=A,B;Units=U1,U2;OutputWorkspace=flat_signal_a");
      // Signal = 1, 2, 3, but only the middle to the end of the range is marked as overlap, so only 2, 3 used.
      auto algB = frameworkManager.exec("CreateMDHistoWorkspace", "SignalInput=1,2,3;ErrorInput=1,1,1;Dimensionality=2;Extents=-1,1,-1,1;NumberOfBins=3,1;Names=A,B;Units=U1,U2;OutputWorkspace=flat_signal_b");

      IMDHistoWorkspace_sptr a = AnalysisDataService::Instance().retrieveWS<IMDHistoWorkspace>(algA->getPropertyValue("OutputWorkspace"));
      IMDHistoWorkspace_sptr b = AnalysisDataService::Instance().retrieveWS<IMDHistoWorkspace>(algB->getPropertyValue("OutputWorkspace"));

      IAlgorithm_sptr alg = make_algorithm(a, b, "converted", 0.5, 1.0);
      alg->execute();

      Mantid::API::AnalysisDataService::Instance().remove(algA->getPropertyValue("OutputWorkspace"));
      Mantid::API::AnalysisDataService::Instance().remove(algB->getPropertyValue("OutputWorkspace"));  

      // Check defaults.
      bool useManualScaling = alg->getProperty("UseManualScaleFactor");
      TS_ASSERT(!useManualScaling);
      bool scaleRHSWorkspace = alg->getProperty("ScaleRHSWorkspace");
      TS_ASSERT(scaleRHSWorkspace);

      double scaleFactor = alg->getProperty("OutScaleFactor");

      // 1 * (( 1 + 1) / (2 + 3)) = 0.4
      const double expectedScaleFactor = 0.4;
      TS_ASSERT_EQUALS(expectedScaleFactor, scaleFactor);
    }

    void test_calculates_scaling_factor_correctly_inverted()
    { 
      Mantid::API::FrameworkManagerImpl& frameworkManager = Mantid::API::FrameworkManager::Instance();

      //Signal = 1, 1, 1, but only the middle to the end of the range is marked as overlap, so only 1, 1 used.
      auto algA = frameworkManager.exec("CreateMDHistoWorkspace", "SignalInput=1,1,1;ErrorInput=1,1,1;Dimensionality=2;Extents=-1,1,-1,1;NumberOfBins=3,1;Names=A,B;Units=U1,U2;OutputWorkspace=flat_signal_a");
      // Signal = 1, 2, 3, but only the middle to the end of the range is marked as overlap, so only 2, 3 used.
      auto algB = frameworkManager.exec("CreateMDHistoWorkspace", "SignalInput=1,2,3;ErrorInput=1,1,1;Dimensionality=2;Extents=-1,1,-1,1;NumberOfBins=3,1;Names=A,B;Units=U1,U2;OutputWorkspace=flat_signal_b");

      IMDHistoWorkspace_sptr a = AnalysisDataService::Instance().retrieveWS<IMDHistoWorkspace>(algA->getPropertyValue("OutputWorkspace"));
      IMDHistoWorkspace_sptr b = AnalysisDataService::Instance().retrieveWS<IMDHistoWorkspace>(algB->getPropertyValue("OutputWorkspace"));

      IAlgorithm_sptr alg = make_algorithm(a, b, "converted", 0.5, 1.0, false);
      alg->execute();

      Mantid::API::AnalysisDataService::Instance().remove(algA->getPropertyValue("OutputWorkspace"));
      Mantid::API::AnalysisDataService::Instance().remove(algB->getPropertyValue("OutputWorkspace"));  

      bool useManualScaling = alg->getProperty("UseManualScaleFactor");
      TS_ASSERT(!useManualScaling);

      double scaleFactor = alg->getProperty("OutScaleFactor");

      // 1 * ((2 + 3)/( 1 + 1)) = 2.5
      const double expectedScaleFactor = 2.5;
      TS_ASSERT_EQUALS(expectedScaleFactor, scaleFactor);
    }

    void test_manual_scaling_factor()
    {
      double expected_manual_scale_factor = 2.2;

      auto a = AnalysisDataService::Instance().retrieveWS<IMDHistoWorkspace>(this->__good_workspace_name);

      Stitch1DMD alg;
      alg.setRethrows(true);
      alg.initialize();

      alg.setProperty("LHSWorkspace", a);
      alg.setProperty("RHSWorkspace", a);
      alg.setPropertyValue("OutputWorkspace", "converted");
      alg.setProperty("StartOverlap", 0.5);
      alg.setProperty("EndOverlap", 1.0);
      alg.setProperty("UseManualScaleFactor", true);
      alg.setProperty("ManualScaleFactor", expected_manual_scale_factor);
      alg.execute();

      TS_ASSERT(alg.isExecuted());
      double scale_factor = alg.getProperty("OutScaleFactor");

      TS_ASSERT_EQUALS(expected_manual_scale_factor, scale_factor);
    }

    void test_overlap_in_center()
    {
      Mantid::API::FrameworkManagerImpl& frameworkManager = Mantid::API::FrameworkManager::Instance();

      auto algA = frameworkManager.exec("CreateMDHistoWorkspace","SignalInput=0,0,0,3,3,3,3,3,3,3;ErrorInput=1,1,1,1,1,1,1,1,1,1;Dimensionality=2;Extents=-1,1,-1,1;NumberOfBins=10,1;Names=A,B;Units=U1,U2;OutputWorkspace=flat_signal_a");
      auto algB = frameworkManager.exec("CreateMDHistoWorkspace","SignalInput=2,2,2,2,2,2,2,0,0,0;ErrorInput=1,1,1,1,1,1,1,1,1,1;Dimensionality=2;Extents=-1,1,-1,1;NumberOfBins=10,1;Names=A,B;Units=U1,U2;OutputWorkspace=flat_signal_b");
      std::vector<double> expected_output_signal(10, 3); 

      IMDHistoWorkspace_sptr a = AnalysisDataService::Instance().retrieveWS<IMDHistoWorkspace>(algA->getPropertyValue("OutputWorkspace"));
      IMDHistoWorkspace_sptr b = AnalysisDataService::Instance().retrieveWS<IMDHistoWorkspace>(algB->getPropertyValue("OutputWorkspace"));

      IAlgorithm_sptr alg = make_algorithm(a, b, "converted", 0.3, 0.7);
      alg->execute();

      auto outWS = AnalysisDataService::Instance().retrieveWS<IMDHistoWorkspace>(alg->getPropertyValue("OutputWorkspace"));
      for(size_t i = 0; i < 10; ++i)
      {
        TS_ASSERT_DELTA(expected_output_signal[i], outWS->signalAt(i), 0.0001);  
      }

      Mantid::API::AnalysisDataService::Instance().remove(algA->getPropertyValue("OutputWorkspace"));
      Mantid::API::AnalysisDataService::Instance().remove(algB->getPropertyValue("OutputWorkspace"));  
    }

    void test_flat_offsetting_schenario_with_manual_scaling()
    {
      Mantid::API::FrameworkManagerImpl& frameworkManager = Mantid::API::FrameworkManager::Instance();

      std::vector<double> expected_output_signal = boost::assign::list_of(1)(1)(1)(1)(2)(2)(6)(6)(6)(6);
      auto algA = frameworkManager.exec("CreateMDHistoWorkspace","SignalInput=1,1,1,1,1,1,0,0,0,0;ErrorInput=1,1,1,1,1,1,1,1,1,1;Dimensionality=2;Extents=-1,1,-1,1;NumberOfBins=10,1;Names=A,B;Units=U1,U2;OutputWorkspace=flat_signal_a");
      auto algB = frameworkManager.exec("CreateMDHistoWorkspace","SignalInput=0,0,0,0,3,3,3,3,3,3;ErrorInput=1,1,1,1,1,1,1,1,1,1;Dimensionality=2;Extents=-1,1,-1,1;NumberOfBins=10,1;Names=A,B;Units=U1,U2;OutputWorkspace=flat_signal_b");

      // Supply a manual scale factor, this will mean that Workspace 1 is scaled by this amount.
      double manual_scale_factor = 2;

      IMDHistoWorkspace_sptr a = AnalysisDataService::Instance().retrieveWS<IMDHistoWorkspace>(algA->getPropertyValue("OutputWorkspace"));
      IMDHistoWorkspace_sptr b = AnalysisDataService::Instance().retrieveWS<IMDHistoWorkspace>(algB->getPropertyValue("OutputWorkspace"));

      Stitch1DMD alg;
      alg.setRethrows(true);
      alg.initialize();

      alg.setProperty("LHSWorkspace", a);
      alg.setProperty("RHSWorkspace", b);
      alg.setPropertyValue("OutputWorkspace", "converted");
      alg.setProperty("StartOverlap", 0.4);
      alg.setProperty("EndOverlap", 0.6);
      alg.setProperty("UseManualScaleFactor", true);
      alg.setProperty("ManualScaleFactor", manual_scale_factor);
      alg.execute();

      auto outWS = AnalysisDataService::Instance().retrieveWS<IMDHistoWorkspace>(alg.getPropertyValue("OutputWorkspace"));
      for(size_t i = 0; i < 10; ++i)
      {
        TS_ASSERT_DELTA(expected_output_signal[i], outWS->signalAt(i), 0.0001);  
      }

      double scale_factor = alg.getProperty("OutScaleFactor");
      TS_ASSERT_EQUALS(manual_scale_factor, scale_factor);

      Mantid::API::AnalysisDataService::Instance().remove(algA->getPropertyValue("OutputWorkspace"));
      Mantid::API::AnalysisDataService::Instance().remove(algB->getPropertyValue("OutputWorkspace"));  
    }

    void test_no_overlap_bins_throws()
    {
      Mantid::API::FrameworkManagerImpl& frameworkManager = Mantid::API::FrameworkManager::Instance();
      auto algA = frameworkManager.exec("CreateMDHistoWorkspace","SignalInput=0,0;ErrorInput=1,1;Dimensionality=1;Extents=-1,1;NumberOfBins=2;Names=A;Units=U1;OutputWorkspace=mdhw");
      IMDHistoWorkspace_sptr mdhw = AnalysisDataService::Instance().retrieveWS<IMDHistoWorkspace>(algA->getPropertyValue("OutputWorkspace"));

      IAlgorithm_sptr alg = boost::make_shared<Stitch1DMD>();
      alg->setRethrows(true);
      alg->initialize();

      alg->setProperty("LHSWorkspace", mdhw);
      alg->setProperty("RHSWorkspace", mdhw);
      alg->setPropertyValue("OutputWorkspace", "converted");
      alg->setProperty("StartOverlap", 0.001); // The overlap region would correspond to zero bins with these inputs!
      alg->setProperty("EndOverlap", 0.002);
      TS_ASSERT_THROWS(alg->execute(), std::invalid_argument);
    }

    void test_on_multi_period_group()
    {
      Mantid::API::FrameworkManagerImpl& frameworkManager = Mantid::API::FrameworkManager::Instance();

      auto algA = frameworkManager.exec("CreateMDHistoWorkspace","SignalInput=0,0,0,3,3,3,3,3,3,3;ErrorInput=1,1,1,1,1,1,1,1,1,1;Dimensionality=2;Extents=-1,1,-1,1;NumberOfBins=10,1;Names=A,B;Units=U1,U2;OutputWorkspace=flat_signal_a");
      auto algB = frameworkManager.exec("CreateMDHistoWorkspace","SignalInput=2,2,2,2,2,2,2,0,0,0;ErrorInput=1,1,1,1,1,1,1,1,1,1;Dimensionality=2;Extents=-1,1,-1,1;NumberOfBins=10,1;Names=A,B;Units=U1,U2;OutputWorkspace=flat_signal_b");
      auto algC = frameworkManager.exec("CreateMDHistoWorkspace","SignalInput=0,0,0,4,4,4,4,4,4,4;ErrorInput=1,1,1,1,1,1,1,1,1,1;Dimensionality=2;Extents=-1,1,-1,1;NumberOfBins=10,1;Names=A,B;Units=U1,U2;OutputWorkspace=flat_signal_c");
      auto algD = frameworkManager.exec("CreateMDHistoWorkspace","SignalInput=2,2,2,2,2,2,2,0,0,0;ErrorInput=1,1,1,1,1,1,1,1,1,1;Dimensionality=2;Extents=-1,1,-1,1;NumberOfBins=10,1;Names=A,B;Units=U1,U2;OutputWorkspace=flat_signal_d");
      
      std::vector<double> expected_output_signalAB(10, 3); 
      std::vector<double> expected_output_signalCD(10, 4); 

      IMDHistoWorkspace_sptr a = AnalysisDataService::Instance().retrieveWS<IMDHistoWorkspace>(algA->getPropertyValue("OutputWorkspace"));
      IMDHistoWorkspace_sptr b = AnalysisDataService::Instance().retrieveWS<IMDHistoWorkspace>(algB->getPropertyValue("OutputWorkspace"));
      IMDHistoWorkspace_sptr c = AnalysisDataService::Instance().retrieveWS<IMDHistoWorkspace>(algC->getPropertyValue("OutputWorkspace"));
      IMDHistoWorkspace_sptr d = AnalysisDataService::Instance().retrieveWS<IMDHistoWorkspace>(algD->getPropertyValue("OutputWorkspace"));

      WorkspaceGroup_sptr lhs = boost::make_shared<Mantid::API::WorkspaceGroup>();
      WorkspaceGroup_sptr rhs = boost::make_shared<Mantid::API::WorkspaceGroup>();
      add_periods_logs(lhs);
      add_periods_logs(rhs);
      lhs->addWorkspace(a);
      lhs->addWorkspace(c);
      rhs->addWorkspace(b);
      rhs->addWorkspace(d);
      AnalysisDataService::Instance().addOrReplace("lhs", lhs);
      AnalysisDataService::Instance().addOrReplace("rhs", rhs);

      IAlgorithm_sptr alg = boost::make_shared<Stitch1DMD>();
      alg->setRethrows(true);
      alg->initialize();

      alg->setProperty("LHSWorkspace", "lhs");
      alg->setProperty("RHSWorkspace", "rhs");
      alg->setPropertyValue("OutputWorkspace", "converted");
      alg->setProperty("StartOverlap", 0.3);
      alg->setProperty("EndOverlap", 0.7);
      alg->execute();

      auto outWS = AnalysisDataService::Instance().retrieveWS<WorkspaceGroup>(alg->getPropertyValue("OutputWorkspace"));
      TS_ASSERT_EQUALS(2, outWS->size());
      auto outWS_1 = boost::dynamic_pointer_cast<IMDHistoWorkspace>(outWS->getItem(0));
      for(size_t i = 0; i < 10; ++i)
      {
        TS_ASSERT_DELTA(expected_output_signalAB[i], outWS_1->signalAt(i), 0.0001);  
      }
      auto outWS_2 = boost::dynamic_pointer_cast<IMDHistoWorkspace>(outWS->getItem(1));
      for(size_t i = 0; i < 10; ++i)
      {
        TS_ASSERT_DELTA(expected_output_signalCD[i], outWS_2->signalAt(i), 0.0001);  
      }

      Mantid::API::AnalysisDataService::Instance().remove("lhs");
      Mantid::API::AnalysisDataService::Instance().remove("rhs"); 
    }
};


#endif /* MANTID_MDALGORITHMS_STITCH1DTEST_H_ */
