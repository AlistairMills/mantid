// SetScalingPSD
// @author Ronald Fowler
//----------------------------------------------------------------------
// Includes
//----------------------------------------------------------------------
#include "MantidAlgorithms/SetScalingPSD.h"
#include "MantidDataObjects/Workspace2D.h"
#include "MantidKernel/ArrayProperty.h"
#include "MantidAPI/WorkspaceValidators.h"
#include <cmath>
#include <fstream>


namespace Mantid
{
namespace Algorithms
{

  // Register the algorithm into the algorithm factory
  DECLARE_ALGORITHM(SetScalingPSD)

  using namespace Kernel;
  using namespace API;
  using namespace Geometry;
  using namespace DataObjects;

  Logger& SetScalingPSD::g_log = Logger::get("SetScalingPSD");

  /// Empty default constructor
  SetScalingPSD::SetScalingPSD() :
      Algorithm()
  {
  }

  /** Initialisation method.
   *
   */
  void SetScalingPSD::init()
  {
    // Declare required input parameters for algorithm
    std::vector<std::string> exts;
    exts.push_back("sca");
    exts.push_back("raw");
    declareProperty("ScalingFilename","",new FileValidator(exts));
    //declareProperty(new WorkspaceProperty<>("Workspace","",Kernel::Direction::InOut,wsValidator));
    declareProperty(new WorkspaceProperty<>("Workspace","",Direction::InOut));
  }

  /** Executes the algorithm.
   *
   *  @throw runtime_error Thrown if algorithm cannot execute
   */
  void SetScalingPSD::exec()
  {
    // Retrieve the filename and output workspace name from the properties
    m_filename = getPropertyValue("ScalingFilename");
    //m_workspace = getPropertyValue("Workspace");
    m_workspace = getProperty("Workspace");
    std::vector<Geometry::V3D> truepos;
    processScalingFile(m_filename,truepos);
    //calculateDetectorShifts(truepos);
    
    return;
  }

  bool SetScalingPSD::processScalingFile(const std::string& scalingFile, std::vector<Geometry::V3D>& truepos)
  {
      // Read the scaling information from a file (e.g. merlin_detector.sca)
      // This is really corrected positions as (r,theta,phi) for each detector
      // Compare these with the instrument values to determine the change in position and the scaling
      // which may be necessary for each pixel if in a tube.
      // movePos is used to updated positions
      std::map<int,Geometry::V3D> posMap;
      std::map<int,Geometry::V3D>::iterator it;
      std::map<int,double> scaleMap;
      std::map<int,double>::iterator its;

      IInstrument_const_sptr instrument = m_workspace->getInstrument();
      if(scalingFile.find(".sca") || scalingFile.find(".SCA"))
      {
          std::ifstream sFile(scalingFile.c_str());
          if (!sFile)
          {
              g_log.error() << "Unable to open scaling file " << scalingFile << std::endl;
              return false;
          }
          std::string str;
          getline(sFile,str); // skip header line should be <filename> generated by <prog>
          int detectorCount;
          getline(sFile,str); // get detector count line
          std::istringstream istr(str);
          istr >> detectorCount;
          if(detectorCount<1)
          {
              g_log.error("Bad detector count in scaling file");
              throw std::runtime_error("Bad detector count in scaling file"); 
          }
          truepos.reserve(detectorCount);
          getline(sFile,str); // skip title line
          int detIdLast=-10;
          Geometry:: V3D truPosLast,detPosLast;
          while(getline(sFile,str))
          {
              if (str.empty() || str[0] == '#') continue;
              std::istringstream istr(str);
              int detIndex,code;
              double l2,theta,phi,offset;
              istr >> detIndex >> offset >> l2 >> code >> theta >> phi;
              Geometry::V3D truPos;
              // use abs as correction file has -ve l2 for first few detectors
              truPos.spherical(fabs(l2),theta,phi);
              truepos.push_back(truPos);
              //Geometry::IDetector_const_sptr det = m_workspace->getDetector(detIndex);
              Geometry::IDetector_const_sptr det = instrument->getDetector(detIndex);
              Geometry::V3D detPos = det->getPos();
              Geometry::V3D shift=truPos-detPos;
              double scale=1.0;
              if(detIdLast==detIndex-1 && detIndex>100) // merlin monitors are <100, dets >100
              {
                  Geometry::V3D diffI=detPos-detPosLast;
                  Geometry::V3D diffT=truPos-truPosLast;
                  scale=diffT.norm()/diffI.norm();
                  Geometry::V3D scaleDir=diffT/diffT.norm();
                  double tol=1e-4;
                  int mDir=abs(scaleDir.masterDir(tol));
                  scaleMap[detIndex]=scale;
                  its=scaleMap.find(detIndex-1);
                  if(its==scaleMap.end())
                     scaleMap[detIndex-1]=scale;
                  else
                     its->second=0.5*(its->second+scale);
                  //std::cout << detIndex << scale << scaleDir << std::endl;
              }
              detIdLast=detIndex;
              detPosLast=detPos;
              truPosLast=truPos;
              posMap[detIndex]=shift;
              //runMoveInstrumentComp(detIndex,shift);
          }
      }
      else if(scalingFile.find(".raw") || scalingFile.find(".RAW"))
      {
          // read from raw TODO
      }
      movePos( m_workspace, posMap, scaleMap);
      return true;
  }

void SetScalingPSD::runMoveInstrumentComp(const int& detIndex, const Geometry::V3D& shift)
{
   // This was used initially to move each detector using MoveInstrumentComponent alg
   // but is too slow on a large instrument like Merlin.
   IAlgorithm_sptr moveInstruComp = createSubAlgorithm("MoveInstrumentComponent");
   moveInstruComp->setProperty<MatrixWorkspace_sptr>("Workspace",m_workspace); //?
   std::ostringstream tmpstmd,tmpstmx,tmpstmy,tmpstmz;
   tmpstmd << detIndex;
   moveInstruComp->setPropertyValue("DetectorID", tmpstmd.str());
   tmpstmx << shift[0];
   moveInstruComp->setPropertyValue("X", tmpstmx.str());
   tmpstmy << shift[1];
   moveInstruComp->setPropertyValue("Y", tmpstmy.str());
   tmpstmz << shift[2];
   moveInstruComp->setPropertyValue("Z", tmpstmz.str());

   // Now execute the sub-algorithm. Catch and log any error, but don't stop.
   try
   {
      moveInstruComp->execute();
   }
   catch (std::runtime_error& err)
   {
      g_log.information("Unable to successfully run moveIntrumentComp sub-algorithm");
   }
}


void SetScalingPSD::movePos(MatrixWorkspace_sptr& WS, std::map<int,Geometry::V3D>& posMap,
                            std::map<int,double>& scaleMap)
{
  // Move all the detectors to their actual positions, as stored in posMap
  std::map<int,Geometry::V3D>::iterator iter = posMap.begin();
  boost::shared_ptr<IInstrument> inst = WS->getInstrument();
  boost::shared_ptr<IComponent> comp;

  // Want to get all the detectors to move, but don't want to do this one at a time
  // since the search (based on MoveInstrument findBy methods) is going to be too slow
  // Hence findAll gets a vector of IComponent for all detectors, as m_vectDet.
  m_vectDet.reserve(posMap.size());
  findAll(inst);

  // loop over detector (IComps)
  for(size_t id=0;id<m_vectDet.size();id++)
  {
      V3D Pos,shift;// New relative position
      comp = m_vectDet[id];
      boost::shared_ptr<IDetector> det = boost::dynamic_pointer_cast<IDetector>(comp);
      int idet=0;
      if (det) idet=det->getID();

      iter=posMap.find(idet); // check if we have a shift
      if(iter==posMap.end()) continue;
      shift=iter->second;
      // First set it to the new absolute position (code from MoveInstrument)
      Pos = comp->getPos() + shift;
    
      // Then find the corresponding relative position
      boost::shared_ptr<const IComponent> parent = comp->getParent();
      if (parent)
      {
          Pos -= parent->getPos();
          Quat rot = parent->getRelativeRot();
          rot.inverse();
          rot.rotate(Pos);
      }
    
      //Need to get the address to the base instrument component
      boost::shared_ptr<Geometry::ParameterMap> pmap = WS->InstrumentParameters();
      ParametrizedComponent* pcomp = dynamic_cast<ParametrizedComponent*>(comp.get());
      const IComponent* baseComp;
      if (pcomp)
      {
          baseComp = pcomp->base();
      }
      else
      {
          baseComp = comp.get();
      }
    
      // Set "pos" instrument parameter. 
      Parameter_sptr par = pmap->get(baseComp,"pos");
      if (par) par->set(Pos);
      else
          pmap->addV3D(baseComp,"pos",Pos);

      // Set the "sca" instrument parameter
      std::map<int,double>::iterator it=scaleMap.find(idet);
      ObjComponent* baseObjComp = dynamic_cast<ObjComponent*>(comp.get());
      if(it!=scaleMap.end())
      {
          par = pmap->get(baseComp,"sca");
          if (par) par->set(V3D(1.0,it->second,1.0));
          else
              pmap->addV3D(baseComp,"sca",V3D(1.0,it->second,1.0));
      }
      //
      // scale the object, for now just in the IObjComponent, not in the pmap
      //
      //if(it!=scaleMap.end())
      //{
      //   baseObjComp->setScaleFactor(1.0,it->second,1.0);
      //}
  }

  return;
}

void SetScalingPSD::findAll(boost::shared_ptr<IComponent> comp)
/// find all detectors in the comp and push the IComp pointers onto m_vectDet
{
    boost::shared_ptr<IDetector> det = boost::dynamic_pointer_cast<IDetector>(comp);
    if (det)
    {
       m_vectDet.push_back(comp);
       return;
    }
    boost::shared_ptr<ICompAssembly> asmb = boost::dynamic_pointer_cast<ICompAssembly>(comp);
    if (asmb)
        for(int i=0;i<asmb->nelements();i++)
        {
            findAll((*asmb)[i]);
        }
    return;
}


} // namespace Algorithm
} // namespace Mantid
