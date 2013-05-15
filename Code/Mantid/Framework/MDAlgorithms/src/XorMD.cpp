/*WIKI*

Perform the Xor (exclusive-or) boolean operation on two MDHistoWorkspaces.
The xor operation is performed element-by-element.
A signal of 0.0 means "false" and any non-zero signal is "true".

*WIKI*/
/*WIKI_USAGE*
 C = A ^ B
 A ^= B

See [[MDHistoWorkspace#Boolean_Operations|this page]] for examples on using boolean operations.

*WIKI_USAGE*/

#include "MantidMDAlgorithms/XorMD.h"
#include "MantidKernel/System.h"

using namespace Mantid::Kernel;
using namespace Mantid::API;

namespace Mantid
{
namespace MDAlgorithms
{

  // Register the algorithm into the AlgorithmFactory
  DECLARE_ALGORITHM(XorMD)
  
  //----------------------------------------------------------------------------------------------
  /** Constructor
   */
  XorMD::XorMD()
  {  }
    
  //----------------------------------------------------------------------------------------------
  /** Destructor
   */
  XorMD::~XorMD()
  {  }
  
  //----------------------------------------------------------------------------------------------
  /// Algorithm's name for identification. @see Algorithm::name
  const std::string XorMD::name() const { return "XorMD";};
  
  /// Algorithm's version for identification. @see Algorithm::version
  int XorMD::version() const { return 1;};
  
  //----------------------------------------------------------------------------------------------
  /// Run the algorithm with a MDHisotWorkspace as output and operand
  void XorMD::execHistoHisto(Mantid::MDEvents::MDHistoWorkspace_sptr out, Mantid::MDEvents::MDHistoWorkspace_const_sptr operand)
  {
    out->operator ^=(*operand);
  }


} // namespace Mantid
} // namespace MDAlgorithms
