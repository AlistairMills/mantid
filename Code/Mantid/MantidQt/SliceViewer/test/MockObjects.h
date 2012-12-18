#ifndef SLICE_VIEWER_MOCKOBJECTS_H_
#define SLICE_VIEWER_MOCKOBJECTS_H_

#include "MantidAPI/IMDWorkspace.h"
#include "MantidAPI/IPeaksWorkspace.h"
#include "MantidQtSliceViewer/PeaksPresenter.h"
#include "MantidQtSliceViewer/PeakTransform.h"
#include "MantidQtSliceViewer/PeakTransformFactory.h"
#include "MantidQtSliceViewer/PeakOverlayView.h"
#include "MantidQtSliceViewer/PeakOverlayViewFactory.h"
#include "MantidAPI/IPeak.h"
#include <boost/regex.hpp>
#include <gmock/gmock.h>

using namespace MantidQt::SliceViewer;
using namespace Mantid;
using boost::regex;

namespace
{

  /*------------------------------------------------------------
  Mock Peaks Presenter
  ------------------------------------------------------------*/
  class MockPeaksPresenter : public PeaksPresenter
  {
  public:
    MOCK_METHOD0(update, void());
    MOCK_METHOD1(updateWithSlicePoint, void(const double&));
    MOCK_METHOD0(changeShownDim, bool());
    MOCK_CONST_METHOD1(isLabelOfFreeAxis, bool(const std::string&));
    ~MockPeaksPresenter(){}
  };

  /*------------------------------------------------------------
  Mock Peak Transform
  ------------------------------------------------------------*/
  class MockPeakTransform : public PeakTransform 
  {
  public:
    MockPeakTransform()
      :PeakTransform("H (Lattice)", "K (Lattice)", regex("^H.*$"), regex("^K.*$"), regex("^L.*$"))
    {
    }
    ~MockPeakTransform()
    {
    }
    MOCK_CONST_METHOD0(clone, PeakTransform_sptr());
    MOCK_CONST_METHOD1(transform, Mantid::Kernel::V3D(const Mantid::Kernel::V3D&));
    MOCK_CONST_METHOD1(transformPeak, Mantid::Kernel::V3D(const Mantid::API::IPeak&)); 
  };

  /*------------------------------------------------------------
  Mock Peak Transform Factory
  ------------------------------------------------------------*/
class MockPeakTransformFactory : public PeakTransformFactory 
{
 public:
  MOCK_CONST_METHOD0(createDefaultTransform, PeakTransform_sptr());
  MOCK_CONST_METHOD2(createTransform, PeakTransform_sptr(const std::string&, const std::string&));
};

  /*------------------------------------------------------------
  Mock Peak Overlay View
  ------------------------------------------------------------*/
  class MockPeakOverlayView : public PeakOverlayView
  {
  public:
    MOCK_METHOD1(setPlaneDistance, void(const double&));
    MOCK_METHOD0(updateView, void());
    MOCK_METHOD1(setSlicePoint, void(const double&));
    MOCK_METHOD0(hideView, void());
    MOCK_METHOD0(showView, void());
    MOCK_METHOD1(movePosition, void(PeakTransform_sptr));
    ~MockPeakOverlayView(){}
  };

  /*------------------------------------------------------------
  Mock Widget Factory.
  ------------------------------------------------------------*/
  class MockPeakOverlayFactory : public PeakOverlayViewFactory
  {
  public:
    MOCK_CONST_METHOD1(createView, boost::shared_ptr<PeakOverlayView>(const Mantid::Kernel::V3D&));
    MOCK_METHOD1(setRadius, void(const double&));
    MOCK_METHOD2(setZRange, void(const double&, const double&));
    MOCK_CONST_METHOD0(getPlotXLabel, std::string());
    MOCK_CONST_METHOD0(getPlotYLabel, std::string());
    MOCK_METHOD0(updateView, void());
  };
  
  
  /*------------------------------------------------------------
  Mock IPeak
  ------------------------------------------------------------*/
  class MockIPeak : public Mantid::API::IPeak 
  {
  public:
    MOCK_METHOD1(setInstrument,
      void(Geometry::Instrument_const_sptr inst));
    MOCK_CONST_METHOD0(getDetectorID,
      int());
    MOCK_METHOD1(setDetectorID,
      void(int m_DetectorID));
    MOCK_CONST_METHOD0(getDetector,
      Geometry::IDetector_const_sptr());
    MOCK_CONST_METHOD0(getInstrument,
      Geometry::Instrument_const_sptr());
    MOCK_CONST_METHOD0(getRunNumber,
      int());
    MOCK_METHOD1(setRunNumber,
      void(int m_RunNumber));
    MOCK_CONST_METHOD0(getMonitorCount,
      double());
    MOCK_METHOD1(setMonitorCount,
      void(double m_MonitorCount));
    MOCK_CONST_METHOD0(getH,
      double());
    MOCK_CONST_METHOD0(getK,
      double());
    MOCK_CONST_METHOD0(getL,
      double());
    MOCK_CONST_METHOD0(getHKL,
      Mantid::Kernel::V3D());
    MOCK_METHOD1(setH,
      void(double m_H));
    MOCK_METHOD1(setK,
      void(double m_K));
    MOCK_METHOD1(setL,
      void(double m_L));
    MOCK_METHOD3(setHKL,
      void(double H, double K, double L));
    MOCK_METHOD1(setHKL,
      void(Mantid::Kernel::V3D HKL));
    MOCK_CONST_METHOD0(getQLabFrame,
      Mantid::Kernel::V3D());
    MOCK_CONST_METHOD0(getQSampleFrame,
      Mantid::Kernel::V3D());
    MOCK_METHOD0(findDetector,
      bool());
    MOCK_METHOD2(setQSampleFrame,
      void(Mantid::Kernel::V3D QSampleFrame, double detectorDistance));
    MOCK_METHOD2(setQLabFrame,
      void(Mantid::Kernel::V3D QLabFrame, double detectorDistance));
    MOCK_METHOD1(setWavelength,
      void(double wavelength));
    MOCK_CONST_METHOD0(getWavelength,
      double());
    MOCK_CONST_METHOD0(getScattering,
      double());
    MOCK_CONST_METHOD0(getDSpacing,
      double());
    MOCK_CONST_METHOD0(getTOF,
      double());
    MOCK_CONST_METHOD0(getInitialEnergy,
      double());
    MOCK_CONST_METHOD0(getFinalEnergy,
      double());
    MOCK_METHOD1(setInitialEnergy,
      void(double m_InitialEnergy));
    MOCK_METHOD1(setFinalEnergy,
      void(double m_FinalEnergy));
    MOCK_CONST_METHOD0(getIntensity,
      double());
    MOCK_CONST_METHOD0(getSigmaIntensity,
      double());
    MOCK_METHOD1(setIntensity,
      void(double m_Intensity));
    MOCK_METHOD1(setSigmaIntensity,
      void(double m_SigmaIntensity));
    MOCK_CONST_METHOD0(getBinCount,
      double());
    MOCK_METHOD1(setBinCount,
      void(double m_BinCount));
    MOCK_CONST_METHOD0(getGoniometerMatrix,
      Mantid::Kernel::Matrix<double>());
    MOCK_METHOD1(setGoniometerMatrix,
      void(Mantid::Kernel::Matrix<double> m_GoniometerMatrix));
    MOCK_CONST_METHOD0(getBankName,
      std::string());
    MOCK_CONST_METHOD0(getRow,
      int());
    MOCK_CONST_METHOD0(getCol,
      int());
    MOCK_CONST_METHOD0(getDetPos,
      Mantid::Kernel::V3D());
    MOCK_CONST_METHOD0(getL1,
      double());
    MOCK_CONST_METHOD0(getL2,
      double());
  };

  /*------------------------------------------------------------
  Mock MDGeometry
  ------------------------------------------------------------*/
  class MockMDGeometry : public Mantid::API::MDGeometry
  {
  public:
    MOCK_CONST_METHOD0(getNumDims, size_t());
    MOCK_CONST_METHOD1(getDimension, boost::shared_ptr<const Mantid::Geometry::IMDDimension>(size_t));
    virtual ~MockMDGeometry() {}
  };

  /*------------------------------------------------------------
  Mock IMDDimension
  ------------------------------------------------------------*/
  class MockIMDDimension : public Mantid::Geometry::IMDDimension 
  {
  public:
    MOCK_CONST_METHOD0(getName,
      std::string());
    MOCK_CONST_METHOD0(getUnits,
      std::string());
    MOCK_CONST_METHOD0(getDimensionId,
      std::string());
    MOCK_CONST_METHOD0(getMaximum,
      coord_t());
    MOCK_CONST_METHOD0(getMinimum,
      coord_t());
    MOCK_CONST_METHOD0(getNBins,
      size_t());
    MOCK_CONST_METHOD0(toXMLString,
      std::string());
    MOCK_CONST_METHOD0(getIsIntegrated,
      bool());
    MOCK_CONST_METHOD1(getX,
      coord_t(size_t ind));
    MOCK_METHOD3(setRange,
      void(size_t nBins, coord_t min, coord_t max));
  };

}

#endif