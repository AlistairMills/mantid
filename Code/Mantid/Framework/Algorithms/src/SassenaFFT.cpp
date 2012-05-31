/*WIKI*

The SassenaFFT algorithm performs the discrete Fourier transform on the intermediate scattering factor, F(q,t), resulting from loading a Sassena input file.

The input data is mirrored into negative times, as F(q,t) is expected to be invariant under time reversal.
 */

//----------------------------------------------------------------------
// Includes
//----------------------------------------------------------------------
#include "MantidAlgorithms/SassenaFFT.h"
#include "MantidAPI/FileProperty.h"
#include "MantidAPI/WorkspaceGroup.h"
#include "MantidDataObjects/Workspace2D.h"
#include "MantidAPI/Algorithm.h"

namespace Mantid
{
namespace Algorithms
{

// Register the class into the algorithm factory
DECLARE_ALGORITHM(SassenaFFT);

/// Sets Documentation strings for this algorithm
void SassenaFFT::initDocs()
{
  this->setWikiSummary("Performs complex Fast Fourier Transform of intermediate scattering function");
  this->setOptionalMessage("Performs complex Fast Fourier Transform of intermediate scattering function");
  this->setWikiDescription("Performs complex Fast Fourier Transform of intermediate scattering function");
}

/**
 * Initialise the algorithm. Declare properties which can be set before execution (input) or
 * read from after the execution (output).
 */
void SassenaFFT::init()
{
  this->declareProperty(new API::WorkspaceProperty<API::WorkspaceGroup>("InputWorkspace","",Kernel::Direction::Input), "The name of the input group workspace");
}

/// Execute the algorithm
void SassenaFFT::exec()
{
  const std::string gwsName = this->getPropertyValue("InputWorkspace");
  API::WorkspaceGroup_sptr gws = this->getProperty("InputWorkspace");

  const std::string ftqReName = gwsName + "_fqt.re";
  const std::string ftqImName = gwsName + "_fqt.re";

  // Make sure the intermediate structure factor is there
  if(!gws->contains(ftqReName) )
  {
    const std::string errMessg = "workspace "+gwsName+" does not contain an intermediate structure factor";
    this->g_log.error(errMessg);
    throw Kernel::Exception::NotFoundError("group workspace does not contain",ftqReName);
  }

  // Retrieve the real and imaginary parts of the intermediate scattering function
  DataObjects::Workspace2D_const_sptr fqtRe = boost::dynamic_pointer_cast<DataObjects::Workspace2D>( gws->getItem( ftqReName ) );
  DataObjects::Workspace2D_const_sptr fqtIm = boost::dynamic_pointer_cast<DataObjects::Workspace2D>( gws->getItem( ftqImName ) );

  // Calculate the FFT for all spectra, retaining only the real part since F(q,-t) = F*(q,t)
  const std::string sqwName = gwsName + "_sqw";
  API::Algorithm_sptr fft = this->createSubAlgorithm("ExtractFFTSpectrum");
  fft->setProperty<DataObjects::Workspace2D_sptr>("InputWorkspace", fqtRe);
  fft->setProperty<DataObjects::Workspace2D_sptr>("InputImagWorkspace", fqtIm);
  fft->setProperty<int>("FFTPart",3);
  fft->setPropertyValue("OutputWorkspace", sqwName );
  fft->executeAsSubAlg();

  // Add to group workspace
  gws->add( sqwName );
}

} // namespacce Mantid
} // namespace Algorithms