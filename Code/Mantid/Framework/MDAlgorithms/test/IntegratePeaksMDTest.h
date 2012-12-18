#ifndef MANTID_MDAGORITHMS_MDEWPEAKINTEGRATIONTEST_H_
#define MANTID_MDAGORITHMS_MDEWPEAKINTEGRATIONTEST_H_

#include "MantidAPI/AnalysisDataService.h"
#include "MantidAPI/IMDEventWorkspace.h"
#include "MantidAPI/FrameworkManager.h"
#include "MantidDataObjects/PeaksWorkspace.h"
#include "MantidGeometry/MDGeometry/MDHistoDimension.h"
#include "MantidKernel/System.h"
#include "MantidKernel/Timer.h"
#include "MantidMDEvents/MDEventFactory.h"
#include "MantidMDAlgorithms/IntegratePeaksMD.h"
#include "MantidTestHelpers/ComponentCreationHelper.h"
#include <boost/math/distributions/normal.hpp>
#include <boost/math/special_functions/fpclassify.hpp>
#include <boost/math/special_functions/pow.hpp>
#include <boost/random/linear_congruential.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/uniform_real.hpp>
#include <boost/random/variate_generator.hpp>
#include <cxxtest/TestSuite.h>
#include <iomanip>
#include <iostream>

using Mantid::API::AnalysisDataService;
using Mantid::Geometry::MDHistoDimension;
using namespace Mantid::API;
using namespace Mantid::DataObjects;
using namespace Mantid::Geometry;
using namespace Mantid::MDEvents;
using namespace Mantid::MDAlgorithms;
using Mantid::Kernel::V3D;


class IntegratePeaksMDTest : public CxxTest::TestSuite
{
public:

  void test_Init()
  {
    IntegratePeaksMD alg;
    TS_ASSERT_THROWS_NOTHING( alg.initialize() )
    TS_ASSERT( alg.isInitialized() )
  }
  

  //-------------------------------------------------------------------------------
  /** Run the IntegratePeaksMD with the given peak radius integration param */
  static void doRun(double PeakRadius, double BackgroundRadius,
      std::string OutputWorkspace = "IntegratePeaksMDTest_peaks",
      double BackgroundStartRadius = 0.0, bool edge = true)
  {
    IntegratePeaksMD alg;
    TS_ASSERT_THROWS_NOTHING( alg.initialize() )
    TS_ASSERT( alg.isInitialized() )
    TS_ASSERT_THROWS_NOTHING( alg.setPropertyValue("InputWorkspace", "IntegratePeaksMDTest_MDEWS" ) );
    TS_ASSERT_THROWS_NOTHING( alg.setPropertyValue("CoordinatesToUse", "HKL" ) );
    TS_ASSERT_THROWS_NOTHING( alg.setProperty("PeakRadius", PeakRadius ) );
    TS_ASSERT_THROWS_NOTHING( alg.setProperty("BackgroundOuterRadius", BackgroundRadius ) );
    TS_ASSERT_THROWS_NOTHING( alg.setProperty("BackgroundInnerRadius", BackgroundStartRadius ) );
    TS_ASSERT_THROWS_NOTHING( alg.setProperty("IntegrateIfOnEdge", edge ) );
    TS_ASSERT_THROWS_NOTHING( alg.setPropertyValue("PeaksWorkspace", "IntegratePeaksMDTest_peaks" ) );
    TS_ASSERT_THROWS_NOTHING( alg.setPropertyValue("OutputWorkspace", OutputWorkspace) );
    TS_ASSERT_THROWS_NOTHING( alg.execute() );
    TS_ASSERT( alg.isExecuted() );
  }


  //-------------------------------------------------------------------------------
  /** Create the (blank) MDEW */
  static void createMDEW()
  {
    // ---- Start with empty MDEW ----
    FrameworkManager::Instance().exec("CreateMDWorkspace", 14,
        "Dimensions", "3",
        "Extents", "-10,10,-10,10,-10,10",
        "Names", "h,k,l",
        "Units", "-,-,-",
        "SplitInto", "5",
        "MaxRecursionDepth", "2",
        "OutputWorkspace", "IntegratePeaksMDTest_MDEWS");
  }


  //-------------------------------------------------------------------------------
  /** Add a fake peak */
  static void addPeak(size_t num, double x, double y, double z, double radius)
  {
    std::ostringstream mess;
    mess << num << ", " << x << ", " << y << ", " << z << ", " << radius;
    FrameworkManager::Instance().exec("FakeMDEventData", 4,
        "InputWorkspace", "IntegratePeaksMDTest_MDEWS", "PeakParams", mess.str().c_str());
  }


  //-------------------------------------------------------------------------------
  /** Full test using faked-out peak data */
  void test_exec()
  {
    // --- Fake workspace with 3 peaks ------
    createMDEW();
    addPeak(1000, 0.,0.,0., 1.0);
    addPeak(1000, 2.,3.,4., 0.5);
    addPeak(1000, 6.,6.,6., 2.0);

    MDEventWorkspace3Lean::sptr mdews =
        AnalysisDataService::Instance().retrieveWS<MDEventWorkspace3Lean>("IntegratePeaksMDTest_MDEWS");
    TS_ASSERT_EQUALS( mdews->getNPoints(), 3000);
    TS_ASSERT_DELTA( mdews->getBox()->getSignal(), 3000.0, 1e-2);

    // Make a fake instrument - doesn't matter, we won't use it really
    Instrument_sptr inst = ComponentCreationHelper::createTestInstrumentRectangular(1, 100, 0.05);

    // --- Make a fake PeaksWorkspace ---
    PeaksWorkspace_sptr peakWS0(new PeaksWorkspace());
    peakWS0->setInstrument(inst);
    peakWS0->addPeak( Peak(inst, 15050, 1.0 ) );

    TS_ASSERT_EQUALS( peakWS0->getPeak(0).getIntensity(), 0.0);
    AnalysisDataService::Instance().add("IntegratePeaksMDTest_peaks",peakWS0);

    // ------------- Integrate with 0.1 radius but IntegrateIfOnEdge false------------------------
    doRun(0.1,0.0,"IntegratePeaksMDTest_peaks",0.0,false);

    TS_ASSERT_DELTA( peakWS0->getPeak(0).getIntensity(), 2.0, 1e-2);

    // Error is also calculated
    TS_ASSERT_DELTA( peakWS0->getPeak(0).getSigmaIntensity(), sqrt(2.0), 1e-2);
    AnalysisDataService::Instance().remove("IntegratePeaksMDTest_peaks");

    // --- Make a fake PeaksWorkspace ---
    PeaksWorkspace_sptr peakWS(new PeaksWorkspace());
    peakWS->addPeak( Peak(inst, 15050, 1.0, V3D(0., 0., 0.) ) );
    peakWS->addPeak( Peak(inst, 15050, 1.0, V3D(2., 3., 4.) ) );
    peakWS->addPeak( Peak(inst, 15050, 1.0, V3D(6., 6., 6.) ) );

    TS_ASSERT_EQUALS( peakWS->getPeak(0).getIntensity(), 0.0);
    AnalysisDataService::Instance().add("IntegratePeaksMDTest_peaks",peakWS);

    // ------------- Integrate with 1.0 radius ------------------------
    doRun(1.0,0.0);

    TS_ASSERT_DELTA( peakWS->getPeak(0).getIntensity(), 1000.0, 1e-2);
    TS_ASSERT_DELTA( peakWS->getPeak(1).getIntensity(), 1000.0, 1e-2);
    // Peak is of radius 2.0, but we get half that radius = ~1/8th the volume
    TS_ASSERT_DELTA( peakWS->getPeak(2).getIntensity(),  125.0, 10.0);

    // Error is also calculated
    TS_ASSERT_DELTA( peakWS->getPeak(0).getSigmaIntensity(), sqrt(1000.0), 1e-2);
    TS_ASSERT_DELTA( peakWS->getPeak(1).getSigmaIntensity(), sqrt(1000.0), 1e-2);
    TS_ASSERT_DELTA( peakWS->getPeak(2).getSigmaIntensity(), sqrt(peakWS->getPeak(2).getIntensity()), 1e-2);


    // ------------- Let's do it again with 2.0 radius ------------------------
    doRun(2.0,0.0);

    // All peaks are fully contained
    TS_ASSERT_DELTA( peakWS->getPeak(0).getIntensity(), 1000.0, 1e-2);
    TS_ASSERT_DELTA( peakWS->getPeak(1).getIntensity(), 1000.0, 1e-2);
    TS_ASSERT_DELTA( peakWS->getPeak(2).getIntensity(), 1000.0, 1e-2);

    // ------------- Let's do it again with 0.5 radius ------------------------
    doRun(0.5,0.0);

    TS_ASSERT_DELTA( peakWS->getPeak(0).getIntensity(), 125.0, 10.0);
    TS_ASSERT_DELTA( peakWS->getPeak(1).getIntensity(), 1000.0, 1e-2);
    TS_ASSERT_DELTA( peakWS->getPeak(2).getIntensity(), 15.0, 10.0);

    // ===============================================================================
    // ---- Now add a background signal over one peak--------------
    addPeak(1000, 0.,0.,0., 2.0);

    // ------------- Integrate with 1.0 radius and 2.0 background------------------------
    doRun(1.0, 2.0);
    // Same 1000 since the background (~125) was subtracted, with some random variation of the BG around
//    TS_ASSERT_DELTA( peakWS->getPeak(0).getIntensity(), 1000.0, 10.0);
    // Error on peak is the SUM of the error of peak and the subtracted background
    TS_ASSERT_DELTA( peakWS->getPeak(0).getSigmaIntensity(), sqrt(1125.0 + 125.0), 2.0);

    // Had no bg, so they are the same
    TS_ASSERT_DELTA( peakWS->getPeak(1).getIntensity(), 1000.0, 1e-2);
    TS_ASSERT_DELTA( peakWS->getPeak(1).getSigmaIntensity(), sqrt(1000.0), 1e-1);

    // This one is a 2.0 radius fake peak, so the background and peak have ~ the same density! So ~0 total intensity.
    TS_ASSERT_DELTA( peakWS->getPeak(2).getIntensity(), 0.0, 12.0);
    // But the error is large since it is 125 - 125 (with errors)
    TS_ASSERT_DELTA( peakWS->getPeak(2).getSigmaIntensity(), sqrt(2*125.0), 2.);


    // ------------- Integrating without the background gives higher counts ------------------------
    doRun(1.0, 0.0);

    // +125 counts due to background
    TS_ASSERT_DELTA( peakWS->getPeak(0).getIntensity(), 1125.0, 10.0);

    // These had no bg, so they are the same
    TS_ASSERT_DELTA( peakWS->getPeak(1).getIntensity(), 1000.0, 1e-2);
    TS_ASSERT_DELTA( peakWS->getPeak(2).getIntensity(), 125.0, 10.0);
    
    AnalysisDataService::Instance().remove("IntegratePeaksMDTest_MDEWS");
    AnalysisDataService::Instance().remove("IntegratePeaksMDTest_peaks");

  }



  //-------------------------------------------------------------------------------
  void test_exec_NotInPlace()
  {
    // --- Fake workspace with 3 peaks ------
    createMDEW();
    addPeak(1000, 0.,0.,0., 1.0);

    // Make a fake instrument - doesn't matter, we won't use it really
    Instrument_sptr inst = ComponentCreationHelper::createTestInstrumentCylindrical(5);
    // --- Make a fake PeaksWorkspace ---
    PeaksWorkspace_sptr peakWS(new PeaksWorkspace());
    peakWS->addPeak( Peak(inst, 1, 1.0, V3D(0., 0., 0.) ) );
    AnalysisDataService::Instance().add("IntegratePeaksMDTest_peaks",peakWS);

    // Integrate and copy to a new peaks workspace
    doRun(1.0,0.0, "IntegratePeaksMDTest_peaks_out");

    // Old workspace is unchanged
    TS_ASSERT_EQUALS( peakWS->getPeak(0).getIntensity(), 0.0);

    PeaksWorkspace_sptr newPW = boost::dynamic_pointer_cast<PeaksWorkspace>(
        AnalysisDataService::Instance().retrieve("IntegratePeaksMDTest_peaks_out"));
    TS_ASSERT( newPW );

    TS_ASSERT_DELTA( newPW->getPeak(0).getIntensity(), 1000.0, 1e-2);
  }


  //-------------------------------------------------------------------------------
  /// Integrate background between start/end background radius
  void test_exec_shellBackground()
  {
    createMDEW();
    /* Create 3 overlapping shells so that density goes like this:
     * r < 1 : density 1.0
     * 1 < r < 2 : density 1/2
     * 2 < r < 3 : density 1/3
     */
    addPeak(1000, 0.,0.,0., 1.0);
    addPeak(1000 * 4, 0.,0.,0., 2.0); // 8 times the volume / 4 times the counts = 1/2 density
    addPeak(1000 * 9, 0.,0.,0., 3.0); // 27 times the volume / 9 times the counts = 1/3 density

    // --- Make a fake PeaksWorkspace ---
    PeaksWorkspace_sptr peakWS(new PeaksWorkspace());
    Instrument_sptr inst = ComponentCreationHelper::createTestInstrumentCylindrical(5);
    peakWS->addPeak( Peak(inst, 1, 1.0, V3D(0., 0., 0.) ) );
    TS_ASSERT_EQUALS( peakWS->getPeak(0).getIntensity(), 0.0);
    AnalysisDataService::Instance().addOrReplace("IntegratePeaksMDTest_peaks",peakWS);

    // First, a check with no background
    doRun(1.0, 0.0, "IntegratePeaksMDTest_peaks", 0.0);
    // approx. + 500 + 333 counts due to 2 backgrounds
    TS_ASSERT_DELTA( peakWS->getPeak(0).getIntensity(), 1000 + 500 + 333, 30.0);
    TSM_ASSERT_DELTA( "Simple sqrt() error", peakWS->getPeak(0).getSigmaIntensity(), sqrt(1833.0), 2);

    // Set background from 2.0 to 3.0.
    // So the 1/2 density background remains, we subtract the 1/3 density = about 1500 counts
    doRun(1.0, 3.0, "IntegratePeaksMDTest_peaks", 2.0);
    TS_ASSERT_DELTA( peakWS->getPeak(0).getIntensity(), 1000 + 500, 80.0);
    // Error is larger, since it is error of peak + error of background
    TSM_ASSERT_DELTA( "Error has increased", peakWS->getPeak(0).getSigmaIntensity(), sqrt(1833.0 + 333.0), 2);

    // Now do the same without the background start radius
    // So we subtract both densities = a lower count
    doRun(1.0, 3.0);
    TSM_ASSERT_LESS_THAN( "Peak intensity is lower if you do not include the spacer shell (higher background)",
                          peakWS->getPeak(0).getIntensity(), 1500);


  }

  void test_writes_out_selected_algorithm_parameters()
  {
    createMDEW();
    const double peakRadius = 2;
    const double backgroundOutterRadius = 3;
    const double backgroundInnerRadius = 2.5;

    doRun(peakRadius, backgroundOutterRadius, "OutWS", backgroundInnerRadius);

    auto outWS = AnalysisDataService::Instance().retrieveWS<PeaksWorkspace>("OutWS");

    double actualPeakRadius = atof(outWS->mutableRun().getProperty("PeakRadius")->value().c_str());
    double actualBackgroundOutterRadius= atof(outWS->mutableRun().getProperty("BackgroundOuterRadius")->value().c_str());
    double actualBackgroundInnerRadius = atof(outWS->mutableRun().getProperty("BackgroundInnerRadius")->value().c_str());

    TS_ASSERT_EQUALS(peakRadius, actualPeakRadius);
    TS_ASSERT_EQUALS(backgroundOutterRadius, actualBackgroundOutterRadius);
    TS_ASSERT_EQUALS(backgroundInnerRadius, actualBackgroundInnerRadius);
    TS_ASSERT(outWS->hasIntegratedPeaks());
  }

};


//=========================================================================================
class IntegratePeaksMDTestPerformance : public CxxTest::TestSuite
{
public:
  size_t numPeaks;
  PeaksWorkspace_sptr peakWS;

  // This pair of boilerplate methods prevent the suite being created statically
  // This means the constructor isn't called when running other tests
  static IntegratePeaksMDTestPerformance *createSuite() { return new IntegratePeaksMDTestPerformance(); }
  static void destroySuite( IntegratePeaksMDTestPerformance *suite ) { delete suite; }


  IntegratePeaksMDTestPerformance()
  {
    numPeaks = 1000;
    // Original MDEW.
    IntegratePeaksMDTest::createMDEW();

    // Add a uniform, random background.
    FrameworkManager::Instance().exec("FakeMDEventData", 4,
        "InputWorkspace", "IntegratePeaksMDTest_MDEWS", "UniformParams", "100000");


    // Make a fake instrument - doesn't matter, we won't use it really
    Instrument_sptr inst = ComponentCreationHelper::createTestInstrumentCylindrical(5);

    boost::mt19937 rng;
    boost::uniform_real<double> u(-9.0, 9.0); // Random from -9 to 9.0
    boost::variate_generator<boost::mt19937&, boost::uniform_real<double> > gen(rng, u);

    peakWS = PeaksWorkspace_sptr(new PeaksWorkspace());
    for (size_t i=0; i < numPeaks; ++i)
    {
      // Random peak center
      double x = gen();
      double y = gen();
      double z = gen();

      // Make the peak
      IntegratePeaksMDTest::addPeak(1000, x,y,z, 0.02);
      // With a center with higher density. 2000 events total.
      IntegratePeaksMDTest::addPeak(1000, x,y,z, 0.005);

      // Make a few very strong peaks
      if (i%21 == 0)
        IntegratePeaksMDTest::addPeak(10000, x,y,z, 0.015);

      // Add to peaks workspace
      peakWS->addPeak( Peak(inst, 1, 1.0, V3D(x, y, z) ) );

      if (i%100==0)
        std::cout << "Peak " << i << " added\n";
    }
    AnalysisDataService::Instance().add("IntegratePeaksMDTest_peaks",peakWS);

  }

  ~IntegratePeaksMDTestPerformance()
  {
    AnalysisDataService::Instance().remove("IntegratePeaksMDTest_MDEWS");
    AnalysisDataService::Instance().remove("IntegratePeaksMDTest_peaks");
  }


  void setUp()
  {

  }

  void tearDown()
  {
  }


  void test_performance_NoBackground()
  {
    for (size_t i=0; i<10; i++)
    {
      IntegratePeaksMDTest::doRun(0.02, 0.0);
    }
    // All peaks should be at least 1000 counts (some might be more if they overla)
    for (size_t i=0; i<numPeaks; i += 7)
    {
      double expected=2000.0;
      if ((i % 21) == 0)
          expected += 10000.0;
      TS_ASSERT_LESS_THAN(expected-1, peakWS->getPeak(int(i)).getIntensity());
    }
  }

  void test_performance_WithBackground()
  {
    for (size_t i=0; i<10; i++)
    {
      IntegratePeaksMDTest::doRun(0.02, 0.03);
    }
  }
};



#endif /* MANTID_MDEVENTS_MDEWPEAKINTEGRATIONTEST_H_ */
