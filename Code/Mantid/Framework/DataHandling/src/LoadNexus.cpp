/*WIKI* 


The algorithm LoadNexus will read the given Nexus file and try to identify its type so that it can be read into a workspace.
The file name can be an absolute or relative path and should have the extension
.nxs or .nx5.
Currently only Nexus Muon Version 1 files are recognised, but this will be extended as other types are supported such as [[LoadNexusProcessed]].

If the file contains data for more than one period, a separate workspace will be generated for each.
After the first period the workspace names will have "_2", "_3", and so on, appended to the given workspace name.
For single period data, the optional parameters can be used to control which spectra are loaded into the workspace.
If spectrum_min and spectrum_max are given, then only that range to data will be loaded.
If a spectrum_list is given than those values will be loaded.

*WIKI*/
// LoadNexus
// @author Freddie Akeroyd, STFC ISIS Faility
// @author Ronald Fowler, e_Science  - updated to be wrapper to either LoadMuonNeuxs or LoadNexusProcessed
// Dropped the upper case X from Algorithm name (still in filenames)
//----------------------------------------------------------------------
// Includes
//----------------------------------------------------------------------
#include "MantidDataHandling/LoadNexus.h"
#include "MantidDataObjects/Workspace2D.h"
#include "MantidNexus/NexusClasses.h"
#include "MantidNexus/NexusFileIO.h"
#include "MantidKernel/ArrayProperty.h"
#include "MantidAPI/FileProperty.h"
#include "MantidAPI/WorkspaceGroup.h"
#include "MantidKernel/BoundedValidator.h"

#include <cmath>
#include <boost/shared_ptr.hpp>

namespace Mantid
{
  namespace DataHandling
  {

    // Register the algorithm into the algorithm factory
    DECLARE_ALGORITHM(LoadNexus)
    
    /// Sets documentation strings for this algorithm
    void LoadNexus::initDocs()
    {
      this->setWikiSummary("The LoadNexus algorithm will try to identify the type of Nexus file given to it and invoke the appropriate algorithm to read the data and populate the named workspace. ");
      this->setOptionalMessage("The LoadNexus algorithm will try to identify the type of Nexus file given to it and invoke the appropriate algorithm to read the data and populate the named workspace.");
    }
    

    using namespace Kernel;
    using namespace API;
    using namespace DataObjects;

    /// Empty default constructor
    LoadNexus::LoadNexus() : Algorithm(), m_filename()
    {}

    /** Initialisation method.
     *
     */
    void LoadNexus::init()
    {
      // Declare required input parameters for all Child Algorithms
      std::vector<std::string> exts;
      exts.push_back(".nxs");
      exts.push_back(".nx5");
      exts.push_back(".xml");
      exts.push_back(".n*");
      declareProperty(new FileProperty("Filename", "", FileProperty::Load, exts),
        "The name of the Nexus file to read, as a full or relative path." );      

      declareProperty(new WorkspaceProperty<Workspace>("OutputWorkspace","",Direction::Output), 
                      "The name of the workspace to be created as the output of the algorithm.  A workspace of this name will be created and stored in the Analysis Data Service. For multiperiod files, one workspace will be generated for each period.");

      // Declare optional input parameters
      auto mustBePositive = boost::make_shared<BoundedValidator<int> >();
      mustBePositive->setLower(0);
      declareProperty("SpectrumMin",1, mustBePositive,
        "Number of first spectrum to read, only for single period data.");
      declareProperty("SpectrumMax", Mantid::EMPTY_INT(), mustBePositive,
                      "Number of last spectrum to read, only for single period data.");
      declareProperty(new ArrayProperty<int>("SpectrumList"),
                      "List of spectrum numbers to read, only for single period data.");

      declareProperty("EntryNumber",0, mustBePositive, 
        "The particular entry number to read (default: Load all workspaces and creates a workspace group)" );
    }

    /** Executes the algorithm. Reading in the file and creating and populating
     *  the output workspace
     *
     *  @throw runtime_error Thrown if algorithm cannot execute
     */
    void LoadNexus::exec()
    {
      // Retrieve the filename and output workspace name from the properties
      m_filename = getPropertyValue("Filename");
      m_workspace = getPropertyValue("OutputWorkspace");

      // Test the given filename to see if it contains the field "analysis" with value "muonTD"
      // within the first NXentry.
      // If so, assume it is a Muon Nexus file (version 1) and pass to the LoadMuonNexus algorithm
      // Otherwise try LoadIsisNexus.

      // FIXME: The following variable isn't used, but the above comments
      // imply it should be.
      //std::string dataName="analysis";

      std::string muonTD="muonTD", pulsedTD="pulsedTD";
      std::vector<std::string> entryName,definition;
      int count= Mantid::NeXus::getNexusEntryTypes(m_filename,entryName,definition);
      if(count<=-1)
      {
        g_log.error("Error reading file " + m_filename);
        throw Exception::FileError("Unable to read data in File:" , m_filename);
      }
      else if(count==0)
      {
        g_log.error("Error no entries found in " + m_filename);
        throw Exception::FileError("Error no entries found in " , m_filename);
      }
      if( definition[0]==muonTD || definition[0]==pulsedTD)
      {
        runLoadMuonNexus();
      }
      else if( entryName[0]=="mantid_workspace_1" )
      {
        runLoadNexusProcessed();
      }
      else if( entryName[0]=="raw_data_1" )
      {
        runLoadIsisNexus();
      }
      else
      {
    	Mantid::NeXus::NXRoot root(m_filename);
    	Mantid::NeXus::NXEntry entry = root.openEntry(root.groups().front().nxname);
        try
        {
        	Mantid::NeXus::NXChar nxc = entry.openNXChar("instrument/SNSdetector_calibration_id");
        }
        catch(...)
        {
          g_log.error("File " + m_filename + " is a currently unsupported type of NeXus file");
          throw Exception::FileError("Unable to read File:" , m_filename);
        }
        runLoadSNSNexus();
      }
      return;
    }

    void LoadNexus::runLoadMuonNexus()
    {
      IAlgorithm_sptr loadMuonNexus = createChildAlgorithm("LoadMuonNexus",0.,1.);
      // Pass through the same input filename
      loadMuonNexus->setPropertyValue("Filename",m_filename);
      // Set the workspace property
      std::string outputWorkspace="OutputWorkspace";
      loadMuonNexus->setPropertyValue(outputWorkspace,m_workspace);
      //Get the array passed in the spectrum_list, if an empty array was passed use the default 
      std::vector<int> specList = getProperty("SpectrumList");
      if ( !specList.empty() )
        loadMuonNexus->setPropertyValue("SpectrumList",getPropertyValue("SpectrumList"));
      //
      int specMax = getProperty("SpectrumMax");
      if( specMax != Mantid::EMPTY_INT() )
      {
        loadMuonNexus->setPropertyValue("SpectrumMax",getPropertyValue("SpectrumMax"));
        loadMuonNexus->setPropertyValue("SpectrumMin",getPropertyValue("SpectrumMin"));
      }
      loadMuonNexus->setPropertyValue("EntryNumber",getPropertyValue("EntryNumber"));

      // Now execute the Child Algorithm. Catch and log any error, but don't stop.
      // try
      // {
      loadMuonNexus->execute();
      // }
      // catch (std::runtime_error&)
      // {
      //   g_log.error("Unable to successfully run LoadMuonNexus Child Algorithm");
      //  }
      if ( ! loadMuonNexus->isExecuted() ) g_log.error("Unable to successfully run LoadMuonNexus2 Child Algorithm");
      // Get pointer to the workspace created
      //  m_localWorkspace=loadMuonNexus->getProperty(outputWorkspace); 
      //  setProperty(outputWorkspace,boost::dynamic_pointer_cast<Workspace>(m_localWorkspace));
      Workspace_sptr localWorkspace=loadMuonNexus->getProperty(outputWorkspace); 
      setProperty(outputWorkspace,boost::dynamic_pointer_cast<Workspace>(localWorkspace));
      //
      // copy pointers to any new output workspaces created by alg LoadMuonNexus to alg LoadNexus
      // Loop through names of form "OutputWorkspace<n>" where <n> is integer from 2 upwards
      // until name not found
      //

      int period=0;
      bool noError=true;
      while(noError)
      {
        std::stringstream suffix;
        //period++;
        suffix << (period+1);
        std::string opWS = outputWorkspace + "_"+suffix.str();
        std::string WSName = m_workspace + "_" + suffix.str();
        try
        {
          Workspace_sptr localWorkspace = loadMuonNexus->getProperty(opWS);
          declareProperty(new WorkspaceProperty<Workspace>(opWS,WSName,Direction::Output));
          setProperty<Workspace_sptr>(opWS, localWorkspace);
          period++;
        }
        catch (Exception::NotFoundError&)
        {
          noError=false;
        }
      }

    }

    void LoadNexus::runLoadNexusProcessed()
    {
      IAlgorithm_sptr loadNexusPro = createChildAlgorithm("LoadNexusProcessed",0.,1.);
      // Pass through the same input filename
      loadNexusPro->setPropertyValue("Filename",m_filename);
      // Set the workspace property
      loadNexusPro->setPropertyValue("OutputWorkspace",m_workspace);

	  loadNexusPro->setPropertyValue("SpectrumMin",getPropertyValue("SpectrumMin"));
	  loadNexusPro->setPropertyValue("SpectrumMax",getPropertyValue("SpectrumMax"));
	  loadNexusPro->setPropertyValue("SpectrumList",getPropertyValue("SpectrumList"));

	
      /* !!! The spectrum min/max/list properties are currently missing from LoadNexus
             so don't pass them through here, just print a warning !!! */

      //Get the array passed in the spectrum_list, if an empty array was passed use the default 
      //std::vector<int> specList = getProperty("SpectrumList");
      //if ( !specList.empty() )
      //{
      //  g_log.warning("SpectrumList property ignored - it is not implemented in LoadNexusProcessed.");
      //  //loadNexusPro->setProperty("SpectrumList",specList);
      //}
      //int specMin = getProperty("SpectrumMin");
      //int specMax = getProperty("SpectrumMax");
      //if ( specMax != Mantid::EMPTY_INT() || specMin != 0 )
      //{
      //  g_log.warning("SpectrumMin/Max properties ignored - they are not implemented in LoadNexusProcessed.");
      //  //loadNexusPro->setProperty("SpectrumMax",specMin);
      //  //loadNexusPro->setProperty("SpectrumMin",specMax);
      //}

      loadNexusPro->setPropertyValue("EntryNumber",getPropertyValue("EntryNumber"));
      // Now execute the Child Algorithm. Catch and log any error, but don't stop.
      loadNexusPro->execute();
      if ( ! loadNexusPro->isExecuted() ) g_log.error("Unable to successfully run LoadNexusProcessed Child Algorithm");
      // Get pointer to the workspace created
      Workspace_sptr localworkspace=loadNexusPro->getProperty("OutputWorkspace");
      setProperty<Workspace_sptr>("OutputWorkspace",localworkspace); 
      //
      // copy pointers to any new output workspaces created by alg LoadNexusProcessed to alg LoadNexus
      // Loop through names of form "OutputWorkspace<n>" where <n> is integer from 2 upwards
      // until name not found.
      // At moment do not expect LoadNexusProcessed to return multiperiod data.
      //
      int period=0;
      bool noError=true;
      while(noError)
      {
        std::stringstream suffix;
        suffix << (period+1);
        std::string opWS = "OutputWorkspace_"+ suffix.str();
        std::string WSName = m_workspace + "_" + suffix.str();
        try
        {
          Workspace_sptr localWorkspace = loadNexusPro->getProperty(opWS); 
          declareProperty(new WorkspaceProperty<Workspace>(opWS,WSName,Direction::Output));
          setProperty<Workspace_sptr>(opWS, localWorkspace);
          period++;
        }
        catch (Exception::NotFoundError&)
        {
          noError=false;
        }
      }
    }

    void LoadNexus::runLoadIsisNexus()
    {
      IAlgorithm_sptr loadNexusPro = createChildAlgorithm("LoadISISNexus",0.,1.);
      // Pass through the same input filename
      loadNexusPro->setPropertyValue("Filename",m_filename);
      // Set the workspace property
      std::string outputWorkspace="OutputWorkspace";
      loadNexusPro->setPropertyValue(outputWorkspace,m_workspace);
      //Get the array passed in the spectrum_list, if an empty array was passed use the default 
      std::vector<int> specList = getProperty("SpectrumList");
      if ( !specList.empty() )
        loadNexusPro->setPropertyValue("SpectrumList",getPropertyValue("SpectrumList"));
      //
      int specMax = getProperty("SpectrumMax");
      if ( specMax != Mantid::EMPTY_INT() )
      {
        loadNexusPro->setPropertyValue("SpectrumMax",getPropertyValue("SpectrumMax"));
        loadNexusPro->setPropertyValue("SpectrumMin",getPropertyValue("SpectrumMin"));
      }
      loadNexusPro->setPropertyValue("EntryNumber",getPropertyValue("EntryNumber"));

      loadNexusPro->execute();

      if ( ! loadNexusPro->isExecuted() ) g_log.error("Unable to successfully run LoadISISNexus Child Algorithm");
      // Get pointer to the workspace created
      Workspace_sptr localWorkspace=loadNexusPro->getProperty(outputWorkspace); 
      setProperty(outputWorkspace,boost::dynamic_pointer_cast<Workspace>(localWorkspace));
      //
      // copy pointers to any new output workspaces created by alg LoadNexusProcessed to alg LoadNexus
      // Loop through names of form "OutputWorkspace<n>" where <n> is integer from 2 upwards
      // until name not found.
      //
      int period=0;
      bool noError=true;
      while(noError)
      {
        std::stringstream suffix;
        suffix << (period+1);
        std::string opWS = outputWorkspace + "_"+suffix.str();
        std::string WSName = m_workspace + "_" + suffix.str();
        try
        {
          Workspace_sptr localWorkspace = loadNexusPro->getProperty(opWS); 
          declareProperty(new WorkspaceProperty<Workspace>(opWS,WSName,Direction::Output));
          setProperty<Workspace_sptr>(opWS,localWorkspace);
          period++;
        }
        catch (Exception::NotFoundError&)
        {
          noError=false;
        }
      }
    }

    void LoadNexus::runLoadSNSNexus()
    {
      IAlgorithm_sptr loadNexusPro = createChildAlgorithm("LoadSNSNexus",0.,1.);
      // Pass through the same input filename
      loadNexusPro->setPropertyValue("Filename",m_filename);
      // Set the workspace property
      std::string outputWorkspace="OutputWorkspace";
      loadNexusPro->setPropertyValue(outputWorkspace,m_workspace);
      //Get the array passed in the spectrum_list, if an empty array was passed use the default 
      std::vector<int> specList = getProperty("SpectrumList");
      if ( !specList.empty() )
        loadNexusPro->setPropertyValue("SpectrumList",getPropertyValue("SpectrumList"));
      //
      int specMax = getProperty("SpectrumMax");
      if ( specMax != Mantid::EMPTY_INT() )
      {
        loadNexusPro->setPropertyValue("SpectrumMax",getPropertyValue("SpectrumMax"));
        loadNexusPro->setPropertyValue("SpectrumMin",getPropertyValue("SpectrumMin"));
      }

      // Now execute the Child Algorithm. Catch and log any error, but don't stop.
      try
      {
        loadNexusPro->execute();
      }
      catch (std::runtime_error&)
      {
        g_log.error("Unable to successfully run LoadSNSNexus Child Algorithm");
      }
      if ( ! loadNexusPro->isExecuted() ) g_log.error("Unable to successfully run LoadSNSNexus Child Algorithm");
      // Get pointer to the workspace created
      Workspace_sptr localWorkspace = loadNexusPro->getProperty(outputWorkspace); 
      setProperty(outputWorkspace,localWorkspace);
      //
      // copy pointers to any new output workspaces created by alg LoadNexusProcessed to alg LoadNexus
      // Loop through names of form "OutputWorkspace<n>" where <n> is integer from 2 upwards
      // until name not found.
      //

      WorkspaceGroup_sptr wsGroup = boost::dynamic_pointer_cast<WorkspaceGroup>(localWorkspace);
      if (wsGroup)
      {
        const std::vector<std::string> wsNames = wsGroup->getNames();
        for(size_t i=0;i<wsNames.size();i++)
        {
          std::stringstream suffix;
          suffix << '_' << (i+1);
          std::string opWS = outputWorkspace + suffix.str();
          std::string WSName = m_workspace + suffix.str();
          Workspace_sptr localWorkspace = loadNexusPro->getProperty(opWS); 
          declareProperty(new WorkspaceProperty<Workspace>(opWS,WSName,Direction::Output));
          setProperty(opWS,localWorkspace);
        }
      }
    }

  } // namespace DataHandling
} // namespace Mantid
