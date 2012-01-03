/*WIKI* 


The algorithm will apply the exponential function (i.e. <math>e^y</math>) to the data and associated errors from a workspaces.
The units of the workspace are not updated, so the user must take care in the use of such output workspaces.


== Usage ==
'''Python'''
 Exponential("input","output")



*WIKI*/
//----------------------------------------------------------------------
// Includes
//----------------------------------------------------------------------
#include "MantidAlgorithms/Exponential.h"
#include <math.h>

using namespace Mantid::API;
using namespace Mantid::Kernel;

namespace Mantid
{
  namespace Algorithms
  {
    // Register the class into the algorithm factory
    DECLARE_ALGORITHM(Exponential)
    
    Exponential::Exponential(): UnaryOperation()
    {
      this->useHistogram=true;
    }
    /// Sets documentation strings for this algorithm
    void Exponential::initDocs()
    {
      this->setWikiSummary("The Exponential algorithm will transform the signal values ''y'' into <math>e^y</math>. The corresponding error values will be updated using <math>E_{new}=E_{old}.e^y</math>, assuming errors are Gaussian and small compared to the signal. ");
      this->setOptionalMessage("The Exponential algorithm will transform the signal values 'y' into <math>e^y</math>. The corresponding error values will be updated using <math>E_{new}=E_{old}.e^y</math>, assuming errors are Gaussian and small compared to the signal.");
    }
    

    void Exponential::performUnaryOperation(const double XIn, const double YIn, const double EIn, double& YOut, double& EOut)
    {
      (void) XIn; //Avoid compiler warning
      // Multiply the data and error by the correction factor
      YOut = exp(YIn);
      EOut = EIn*YOut;
    }

    /*
    void Exponential::setOutputUnits(const API::MatrixWorkspace_const_sptr lhs,API::MatrixWorkspace_sptr out)
    {
      // If Y has not units, then the output will be dimensionless, but not a distribution
      if ( lhs->YUnit().empty() )
      {
        out->setYUnit("");
        out->isDistribution(false); // might be, maybe not?
      }
      // Else units are questionable...
      else
      {
        out->setYUnit("exp(" + lhs->YUnit()+ ")");
      }
    }
    */

  } // namespace Algorithms
} // namespace Mantid
