/*WIKI*

This algorithm loads a Sassena output file into a group workspace.
It will create a workspace for each scattering intensity and one workspace for the Q-values

*WIKI*/

#include "MantidDataHandling/LoadSassena.h"
#include "MantidAPI/LoadAlgorithmFactory.h"
#include "MantidAPI/FileProperty.h"
#include "MantidDataObjects/Workspace2D.h"
#include "MantidKernel/Exception.h"
//#include <hdf5.h>
#include <hdf5_hl.h>

namespace Mantid
{
namespace DataHandling
{

// Register the algorithm into the AlgorithmFactory
DECLARE_ALGORITHM(LoadSassena)
//register the algorithm into LoadAlgorithmFactory
DECLARE_LOADALGORITHM(LoadSassena)

/// Sets documentation strings for this algorithm
void LoadSassena::initDocs()
{
  this->setWikiSummary("This algorithm loads a Sassena output file into a group workspace.");
  this->setOptionalMessage(" Algorithm to load an NXSPE file into a group workspace.");
  this->setWikiDescription("This algorithm loads a Sassena output file into a group workspace. It will create a workspace for each scattering intensity and one workspace for the Q-values");
}

/**
 * Do a quick file type check by looking at the first 100 bytes of the file
 *  @param filePath :: path of the file including name.
 *  @param nread :: no.of bytes read
 *  @param header :: The first 100 bytes of the file as a union
 *  @return true if the given file is of type which can be loaded by this algorithm
 */
bool LoadSassena::quickFileCheck(const std::string& filePath,size_t nread, const file_header& header)
{
  std::string ext = this->extension(filePath);
  // If the extension is h5 then give it a go
  if( ext.compare("h5") == 0 ) return true;

  // If not then let's see if it is a HDF file by checking for the magic cookie
  if ( nread >= sizeof(int32_t) && (ntohl(header.four_bytes) == g_hdf_cookie) ) return true;

  return false;
}

/**
 * Checks the file by opening it and reading few lines
 *  @param filePath :: name of the file inluding its path
 *  @return an integer value how much this algorithm can load the file
 */
int LoadSassena::fileCheck(const std::string &filePath)
{
  int confidence(0);
  hid_t h5file = H5Fopen(filePath.c_str(),H5F_ACC_RDONLY,H5P_DEFAULT);
  if (H5LTfind_attribute(h5file,"sassena_version")==1)
    confidence = 99;
  //else
  //log.debug("no version attribute found");
  return confidence;
}

/**
 * Register a workspace in the Analysis Data Service and add it to the groupWorkspace
 * @param gws pointer to WorkspaceGroup being filled
 * @param wsName name of workspace to be added and registered
 * @param ws pointer to workspace to be added and registered
 * @param description string summarizing workspace contents
 */
void LoadSassena::registerWorkspace( API::WorkspaceGroup_sptr gws, const std::string wsName, DataObjects::Workspace2D_sptr ws, const std::string &description )
{
  this->declareProperty(new API::WorkspaceProperty<DataObjects::Workspace2D>(wsName,wsName,Kernel::Direction::Output), description);
  this->setProperty(wsName,ws);
  gws->add(wsName); // Add the named workspace to the group
}

/**
 * Read dataset dimensionality
 * @param h5file file identifier
 * @param setName string name of dataset
 * @param buffer storing dimensionality
 */
void LoadSassena::dataSetInfo( const hid_t& h5file, const std::string setName, hsize_t* dims)
{
  H5T_class_t class_id;
  size_t type_size;
  if( H5LTget_dataset_info( h5file, setName.c_str(), dims, &class_id, &type_size ) < 0 )
  {
    this->g_log.error("Unable to read "+setName+" dataset info");
    throw Kernel::Exception::FileError("Unable to read "+setName+" dataset info:" , m_filename);
  }
}

/**
 * Read the dataset
 * @param h5file file identifier
 * @param setName string name of dataset
 * @param buffer storing dataset
 */
void LoadSassena::dataSetDouble( const hid_t& h5file, const std::string setName, double *buf )
{
  if( H5LTread_dataset_double(h5file,setName.c_str(),buf) < 0 )
  {
    this->g_log.error("Cannot read "+setName+" dataset");
    throw Kernel::Exception::FileError("Unable to read "+setName+" dataset:" , m_filename);
  }
}

/**
 * load vectors onto a Workspace2D with 3 bins (the three components of the vectors)
 * dataX for the origin of the vector (assumed (0,0,0) )
 * dataY for the tip of the vector
 * dataE is assumed (0,0,0), no errors
 * @param h5file file identifier
 * @param gws pointer to WorkspaceGroup being filled
 */
const MantidVec LoadSassena::loadQvectors(const hid_t& h5file, API::WorkspaceGroup_sptr gws)
{
  const std::string gwsName = this->getPropertyValue("OutputWorkspace");
  std::string setName("qvectors");

  hsize_t dims[3];
  this->dataSetInfo(h5file, setName, dims);
  int nq = static_cast<int>( dims[0] ); //number of q-vectors
  double* buf = new double[nq*3];
  this->dataSetDouble(h5file,"qvectors",buf);

  DataObjects::Workspace2D_sptr ws = boost::dynamic_pointer_cast<DataObjects::Workspace2D>(API::WorkspaceFactory::Instance().create("Workspace2D", nq, 3, 3));
  std::string wsName = gwsName + std::string("_") + setName;
  ws->setTitle(wsName);

  MantidVec qvmod; //store the modulus of the vector
  double* curr = buf;
  for(int iq=0; iq<nq; iq++){
    MantidVec& Y = ws->dataY(iq);
    Y.assign(curr,curr+3);
    qvmod.push_back( sqrt( curr[0]*curr[0] + curr[1]*curr[1] + curr[2]*curr[2] ) );
    curr += 3;
  }
  delete buf;
  this->registerWorkspace(gws,wsName,ws, "X-axis: origin of Q-vectors; Y-axis: tip of Q-vectors");
  return qvmod;
}

/**
 * Create workspace to store the structure factor.
 * First spectrum is the real part, second spectrum is the imaginary part
 * X values are the modulus of the Q-vectors
 * @param h5file file identifier
 * @param gws pointer to WorkspaceGroup being filled
 * @param setName string name of dataset
 * @param qvmod vector of Q-vectors' moduli
 */
void LoadSassena::loadFQ(const hid_t& h5file, API::WorkspaceGroup_sptr gws, const std::string setName, const MantidVec &qvmod){
  const std::string gwsName = this->getPropertyValue("OutputWorkspace");
  int nq = static_cast<int>( qvmod.size() ); //number of q-vectors

  DataObjects::Workspace2D_sptr ws = boost::dynamic_pointer_cast<DataObjects::Workspace2D>(API::WorkspaceFactory::Instance().create("Workspace2D", 2, nq, nq));
  const std::string wsName = gwsName + std::string("_") + setName;
  ws->setTitle(wsName);

  double* buf = new double[nq*2];
  this->dataSetDouble(h5file,setName,buf);
  MantidVec& re = ws->dataY(0); // store the real part
  ws->dataX(0) = qvmod;  //X-axis values are the modulus of the q vector
  MantidVec& im = ws->dataY(1); // store the imaginary part
  ws->dataX(1) = qvmod;
  double* curr = buf;
  for(int iq=0; iq<nq; iq++){
    re[iq]=curr[0];
    im[iq]=curr[1];
    curr += 2;
  }
  delete buf;
  this->registerWorkspace(gws,wsName,ws, "X-axis: Q-vector modulus; Y-axis: intermediate structure factor");
}

/**
 * Create one workspace to hold the real part and another to hold the imaginary part
 * Y-values are structure factor for each Q-value
 * X-values are time bins
 * @param h5file file identifier
 * @param gws pointer to WorkspaceGroup being filled
 * @param setName string name of dataset
 * @param qvmod vector of Q-vectors' moduli
 */
void LoadSassena::loadFQT(const hid_t& h5file, API::WorkspaceGroup_sptr gws, const std::string setName, const MantidVec &qvmod)
{
  const std::string gwsName = this->getPropertyValue("OutputWorkspace");
  int nq = static_cast<int>( qvmod.size() ); //number of q-vectors
  double dt = 1.0; //time unit increment;
  hsize_t dims[3];
  this->dataSetInfo(h5file, setName, dims);
  int nt = static_cast<int>( dims[1] ); //number of time points
  double* buf = new double[nq*nt*2];
  this->dataSetDouble(h5file,setName,buf);

  DataObjects::Workspace2D_sptr wsRe = boost::dynamic_pointer_cast<DataObjects::Workspace2D>(API::WorkspaceFactory::Instance().create("Workspace2D", nq, nt, nt));
  const std::string wsReName = gwsName + std::string("_") + setName + std::string(".Re");
  wsRe->setTitle(wsReName);

  DataObjects::Workspace2D_sptr wsIm = boost::dynamic_pointer_cast<DataObjects::Workspace2D>(API::WorkspaceFactory::Instance().create("Workspace2D", nq, nt, nt));
  const std::string wsImName = gwsName + std::string("_") + setName + std::string(".Im");
  wsIm->setTitle(wsImName);

  double* curr = buf;
  for(int iq=0; iq<nq; iq++)
  {
    MantidVec& reX = wsRe->dataX(iq);
    MantidVec& imX = wsIm->dataX(iq);
    MantidVec& reY = wsRe->dataY(iq);
    MantidVec& imY = wsIm->dataY(iq);
    for(int it=0; it<nt; it++)
    {
      reX[it] = it*dt;  // time point for the real part
      reY[it] = *curr;  // real part of the intermediate structure factor
      curr ++;
      imX[it] = it*dt;
      imY[it] = *curr;
      curr ++;
    }
  }
  delete buf;
  this->registerWorkspace(gws,wsReName,wsRe, "X-axis: time; Y-axis: real part of intermediate structure factor");
  this->registerWorkspace(gws,wsImName,wsIm, "X-axis: time; Y-axis: imaginary part of intermediate structure factor");
}

/**
 * Initialise the algorithm. Declare properties which can be set before execution (input) or
 * read from after the execution (output).
 */
void LoadSassena::init()
{
  std::vector<std::string> exts; // Specify file extensions which can be associated with an output Sassena file
  exts.push_back(".h5");
  exts.push_back(".hd5");

  // Declare the Filename algorithm property. Mandatory. Sets the path to the file to load.
  this->declareProperty(new API::FileProperty("Filename", "", API::FileProperty::Load, exts),"A Sassena file");
  // Declare the OutputWorkspace property
  this->declareProperty(new API::WorkspaceProperty<API::WorkspaceGroup>("OutputWorkspace","",Kernel::Direction::Output), "The name of the group workspace to be created.");
}

/**
 * Execute the algorithm.
 */
void LoadSassena::exec()
{
  const std::string gwsName = this->getPropertyValue("OutputWorkspace");
  API::WorkspaceGroup_sptr gws(new API::WorkspaceGroup);

  //populate m_validSets
  int nvalidSets = 4;
  const char* validSets[] = { "fq", "fq0", "fq2", "fqt"};
  for(int iSet=0; iSet<nvalidSets; iSet++) this->m_validSets.push_back( validSets[iSet] );

  //open the HDF5 file for reading
  m_filename = this->getPropertyValue("Filename");
  hid_t h5file = H5Fopen(m_filename.c_str(),H5F_ACC_RDONLY,H5P_DEFAULT);
  if( h5file < 0)
  {
    this->g_log.error("Cannot open "+m_filename);
    throw Kernel::Exception::FileError("Unable to open:" , m_filename);
  }

  //find out the sassena version used
  char cversion[16];
  if ( H5LTget_attribute_string( h5file, "/", "sassena_version", cversion ) < 0 )
  {
    this->g_log.error("Cannot read version string");
    throw Kernel::Exception::FileError("Unable to read version string:" , m_filename);
  }
  const std::string version(cversion);
  //determine which loader protocol to use based on the version
  //to be done at a later time, maybe implement a Version class

  const MantidVec qvmod = this->loadQvectors( h5file, gws );
  //iterate over the valid sets
  std::string setName;
  for(std::vector<std::string>::const_iterator it=this->m_validSets.begin(); it!=this->m_validSets.end(); ++it){
    setName = *it;
    if(H5LTfind_dataset(h5file,setName.c_str())==1)
    {
      if(setName == "fq" || setName == "fq0" || setName == "fq2")
        this->loadFQ( h5file, gws, setName, qvmod );
      else if(setName == "fqt")
        this->loadFQT( h5file, gws, setName, qvmod );
    }
    else
      this->g_log.information("Dataset "+setName+" not present in file");
  }// end of iterate over the valid sets

  this->setProperty( "OutputWorkspace", gws ); //register the groupWorkspace in the analysis data service

} // end of LoadSassena::exec()


} // endof namespace DataHandling
} // endof namespace Mantid