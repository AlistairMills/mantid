#ifndef MDEVENTS_MDWSDESCRIPTION_TEST_H
#define MDEVENTS_MDWSDESCRIPTION_TEST_H

#include "MantidMDEvents/MDWSDescription.h"
#include "MantidTestHelpers/WorkspaceCreationHelper.h"
#include "MantidKernel/Exception.h"
#include <cxxtest/TestSuite.h>

using namespace Mantid;
using namespace Mantid::MDEvents;

class MDWSDescriptionTest : public CxxTest::TestSuite
{
    Mantid::API::MatrixWorkspace_sptr ws2D;
public:
  void testBuildFromMatrixWS2D()
  {
    MDWSDescription WSD;
    TSM_ASSERT("Initial preprocessed detector state should be NULL ",WSD.getDetectors()==NULL);
    // dimensions (min-max) have not been set
    TS_ASSERT_THROWS(WSD.buildFromMatrixWS(ws2D,"|Q|","Direct"),std::invalid_argument);
    std::vector<double> dimMin(2,-1);
    std::vector<double> dimMax(2, 1);
    WSD.setMinMax(dimMin,dimMax);   
    TS_ASSERT_THROWS_NOTHING(WSD.buildFromMatrixWS(ws2D,"|Q|","Direct"));
    TS_ASSERT_EQUALS(2,WSD.nDimensions());

    TSM_ASSERT("initiating from new matix workspace should nullify preprocessed detectors pointer",WSD.getDetectors()==NULL);

  }
  void testBuildFromMatrixWS4D()
  {
    MDWSDescription WSD;
    std::vector<double> dimMin(4,-10);
    std::vector<double> dimMax(4, 20);
    WSD.setMinMax(dimMin,dimMax);   
    std::vector<std::string> PropNamews(2,"Ei");
    PropNamews[1]="P";
    // no property named "P" is attached to workspace
    TS_ASSERT_THROWS(WSD.buildFromMatrixWS(ws2D,"|Q|","Direct",PropNamews),Kernel::Exception::NotFoundError);

    // H is attached
    PropNamews[1]="H";
    TS_ASSERT_THROWS_NOTHING(WSD.buildFromMatrixWS(ws2D,"|Q|","Indirect",PropNamews));
    TS_ASSERT_EQUALS(4,WSD.nDimensions());

    TSM_ASSERT("initiating from new matix workspace should nullify preprocessed detectors pointer",WSD.getDetectors()==NULL);
  }
  void testGetWS4DimIDFine()
  {
    Mantid::API::MatrixWorkspace_sptr ws2D =WorkspaceCreationHelper::createProcessedWorkspaceWithCylComplexInstrument(4,10,true);
    ws2D->mutableRun().addProperty("Ei",12.,"meV",true);

    MDEvents::MDWSDescription TWS;
    std::vector<double> min(4,-10),max(4,10);
    TWS.setMinMax(min,max);

    std::vector<std::string> other_dim_names;
 

    TS_ASSERT_THROWS_NOTHING(TWS.buildFromMatrixWS(ws2D,"Q3D","Direct",other_dim_names));

    TSM_ASSERT_EQUALS("Inelastic workspace will produce 4 dimensions",4,TWS.nDimensions());
    std::vector<std::string> dim_units = TWS.getDimUnits();
    TSM_ASSERT_EQUALS("Last dimension of Inelastic transformation should be DeltaE","DeltaE",dim_units[3]);
    TSM_ASSERT_EQUALS("Alg ID would be: ","Q3D",TWS.AlgID);
    TSM_ASSERT("detector infromation should be present in the workspace ",!TWS.isDetInfoLost());

    TS_ASSERT_THROWS_NOTHING(TWS.buildFromMatrixWS(ws2D,TWS.AlgID,"Indirect",other_dim_names));

    TSM_ASSERT("initiating from new matix workspace should nullify preprocessed detectors pointer",TWS.getDetectors()==NULL);
    //std::vector<std::string> dimID= TWS.getDefaultDimIDQ3D(1);
    //for(size_t i=0;i<4;i++)
    //{
    //    TS_ASSERT_EQUALS(dimID[i],TWS.dimIDs[i]);
    //    TS_ASSERT_EQUALS(dimID[i],TWS.dimNames[i]);
    //}

   }
  

MDWSDescriptionTest()
{
     ws2D =WorkspaceCreationHelper::createProcessedWorkspaceWithCylComplexInstrument(4,10,true);
    // rotate the crystal by twenty degrees back;
     ws2D->mutableRun().mutableGoniometer().setRotationAngle(0,20);
     // add workspace energy
     ws2D->mutableRun().addProperty("Ei",13.,"meV",true);
     // ADD time series property
     ws2D->mutableRun().addProperty("H",10.,"Gs");


}
};


#endif