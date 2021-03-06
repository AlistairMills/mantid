#include "MantidGeometry/Instrument/ParameterMap.h"
#include "MantidGeometry/Objects/BoundingBox.h"
#include "MantidGeometry/IDetector.h"
#include "MantidGeometry/Instrument/NearestNeighbours.h"
#include "MantidKernel/MultiThreaded.h"
#include "MantidGeometry/Instrument.h"
#include <cstring>
#include <boost/algorithm/string.hpp>


namespace Mantid
{
  namespace Geometry
  {
    using Kernel::V3D;
    using Kernel::Quat;

    namespace
    {
      // names of common parameter types
      const std::string POS_PARAM_NAME="pos";
      const std::string POSX_PARAM_NAME="x";
      const std::string POSY_PARAM_NAME="y";
      const std::string POSZ_PARAM_NAME="z";

      const std::string ROT_PARAM_NAME="rot";
      const std::string ROTX_PARAM_NAME="rotx";
      const std::string ROTY_PARAM_NAME="roty";
      const std::string ROTZ_PARAM_NAME="rotz";

      const std::string DOUBLE_PARAM_NAME="double";
      const std::string INT_PARAM_NAME="int";
      const std::string BOOL_PARAM_NAME="bool";
      const std::string STRING_PARAM_NAME="string";
      const std::string V3D_PARAM_NAME="V3D";
      const std::string QUAT_PARAM_NAME="Quat";

      // static logger reference
      Kernel::Logger g_log("ParameterMap");
    }
    //--------------------------------------------------------------------------
    // Public method
    //--------------------------------------------------------------------------
    /**
     * Default constructor
     */
    ParameterMap::ParameterMap()
      : m_map()
    {}

    /**
    * Return string to be inserted into the parameter map
    */
    // Position
    const std::string & ParameterMap::pos()
    {
      return POS_PARAM_NAME;
    }

    const std::string & ParameterMap::posx()
    {
      return POSX_PARAM_NAME;
    }

    const std::string & ParameterMap::posy()
    {
      return POSY_PARAM_NAME;
    }

    const std::string & ParameterMap::posz()
    {
      return POSZ_PARAM_NAME;
    }

    // Rotation
    const std::string & ParameterMap::rot()
    {
      return ROT_PARAM_NAME;
    }

    const std::string & ParameterMap::rotx()
    {
      return ROTX_PARAM_NAME;
    }

    const std::string & ParameterMap::roty()
    {
      return ROTY_PARAM_NAME;
    }

    const std::string & ParameterMap::rotz()
    {
      return ROTZ_PARAM_NAME;
    }

    // Other types
    const std::string & ParameterMap::pDouble()
    {
      return DOUBLE_PARAM_NAME;
    }

    const std::string & ParameterMap::pInt()
    {
      return INT_PARAM_NAME;
    }

    const std::string & ParameterMap::pBool()
    {
      return BOOL_PARAM_NAME;
    }

    const std::string & ParameterMap::pString()
    {
      return STRING_PARAM_NAME;
    }

    const std::string & ParameterMap::pV3D()
    {
      return V3D_PARAM_NAME;
    }

    const std::string & ParameterMap::pQuat()
    {
      return QUAT_PARAM_NAME;
    }

    /**
     * Compares the values in this object with that given for inequality
     * The order of the values in the map is not important.
     * @param rhs A reference to a ParameterMap object to compare it to
     * @return true if the objects are considered not equal, false otherwise
     */
    bool ParameterMap::operator!=(const ParameterMap & rhs) const
    { 
      return !(this->operator==(rhs)); 
    }
    
    /**
     * Compares the values in this object with that given for equality
     * The order of the values in the map is not important.
     * @param rhs A reference to a ParameterMap object to compare it to
     * @return true if the objects are considered equal, false otherwise
     */
    bool ParameterMap::operator==(const ParameterMap & rhs) const
    {
      if(this == &rhs) return true; // True for the same object
      
      // Quick size check
      if(this->size() != rhs.size()) return false;
      
      // The map is unordered and the key is only valid at runtime. The
      // asString method turns the ComponentIDs to full-qualified name identifiers
      // so we will use the same approach to compare them

      pmap_cit thisEnd = this->m_map.end();
      pmap_cit rhsEnd = rhs.m_map.end();
      for(pmap_cit thisIt = this->m_map.begin(); thisIt != thisEnd; ++thisIt)
      {
        const IComponent * comp = static_cast<IComponent*>(thisIt->first);
        const std::string fullName = comp->getFullName();
        const auto & param = thisIt->second;
        bool match(false);
        for(pmap_cit rhsIt = rhs.m_map.begin(); rhsIt != rhsEnd; ++rhsIt)
        {
          const IComponent * rhsComp = static_cast<IComponent*>(rhsIt->first);
          const std::string rhsFullName = rhsComp->getFullName();
          if(fullName == rhsFullName && (*param) == (*rhsIt->second))
          {
            match = true;
            break;
          }
        }
        if(!match) return false;
      }
      return true;
    }

    /**
     * Clear any parameters with the given name
     * @param name :: The name of the parameter
     */
    void ParameterMap::clearParametersByName(const std::string & name)
    {
      // Key is component ID so have to search through whole lot
      for( pmap_it itr = m_map.begin(); itr != m_map.end();)
      {
        if(itr->second->name() == name)
        {
          m_map.erase(itr++);
        }
        else
        {
          ++itr;
        }
      }
      // Check if the caches need invalidating
      if( name == pos() || name == rot() ) clearPositionSensitiveCaches();
    }

    /**
     * Clear any parameters with the given name for a specified component
     * @param name :: The name of the parameter
     * @param comp :: The component to clear parameters from
     */
    void ParameterMap::clearParametersByName(const std::string & name, const IComponent* comp)
    {
      if( !m_map.empty() )
      {
        const ComponentID id = comp->getComponentID();
        pmap_it it_found = m_map.find(id);
        if (it_found != m_map.end())
        {
          if(it_found->second->name() == name)
          {
            m_map.erase(it_found++);
          }
          else
          {
            ++it_found;
          }
        }

        // Check if the caches need invalidating
        if( name == pos() || name == rot() ) clearPositionSensitiveCaches();
      }
    }
    
    /**
     * Add a value into the map
     * @param type :: A string denoting the type, e.g. double, string, fitting
     * @param comp :: A pointer to the component that this parameter is attached to
     * @param name :: The name of the parameter
     * @param value :: The parameter's value
     */
    void ParameterMap::add(const std::string& type,const IComponent* comp,const std::string& name, 
                           const std::string& value)
    {
      PARALLEL_CRITICAL(parameter_add)
      {
        bool created(false);
        boost::shared_ptr<Parameter> param = retrieveParameter(created, type, comp, name);
        param->fromString(value);
        if( created )
        {
          m_map.insert(std::make_pair(comp->getComponentID(),param));
        }
      }
    }

    /** Create or adjust "pos" parameter for a component
     * Assumed that name either equals "x", "y" or "z" otherwise this 
     * method will not add or modify "pos" parameter
     * @param comp :: Component
     * @param name :: name of the parameter
     * @param value :: value
     */
    void ParameterMap::addPositionCoordinate(const IComponent* comp, const std::string& name, 
                                             const double value)
    {
      Parameter_sptr param = get(comp,pos());
      V3D position;
      if (param)
      {
        // so "pos" already defined
        position = param->value<V3D>();
      }
      else
      {
        // so "pos" is not defined - therefore get position from component
        position = comp->getPos();
      }

      // adjust position

      if ( name.compare(posx())==0 )
        position.setX(value);
      else if ( name.compare(posy())==0 )
        position.setY(value);
      else if ( name.compare(posz())==0 )
        position.setZ(value);
      else
      {
        g_log.warning() << "addPositionCoordinate() called with unrecognised coordinate symbol: " << name;
        return;
      }

      //clear the position cache
      clearPositionSensitiveCaches();
      // finally add or update "pos" parameter
      if (param)
        param->set(position);
      else
        addV3D(comp, pos(), position);
    }

    /** Create or adjust "rot" parameter for a component
     * Assumed that name either equals "rotx", "roty" or "rotz" otherwise this 
     * method will not add/modify "rot" parameter
     * @param comp :: Component
     * @param name :: Parameter name
     * @param deg :: Parameter value in degrees
    */
    void ParameterMap::addRotationParam(const IComponent* comp,const std::string& name, const double deg)
    {
      Parameter_sptr param = get(comp,rot());
      Quat quat;

      Parameter_sptr paramRotX = get(comp,rotx());
      Parameter_sptr paramRotY = get(comp,roty());
      Parameter_sptr paramRotZ = get(comp,rotz());
      double rotX, rotY, rotZ;

      if ( paramRotX )
        rotX = paramRotX->value<double>();
      else
        rotX = 0.0;

      if ( paramRotY )
        rotY = paramRotY->value<double>();
      else
        rotY = 0.0;

      if ( paramRotZ )
        rotZ = paramRotZ->value<double>();
      else
        rotZ = 0.0;
        

      // adjust rotation

      if ( name.compare(rotx())==0 )
      {
        if (paramRotX)
          paramRotX->set(deg);
        else
          addDouble(comp, rotx(), deg);

        quat = Quat(deg,V3D(1,0,0))*Quat(rotY,V3D(0,1,0))*Quat(rotZ,V3D(0,0,1));
      }
      else if ( name.compare(roty())==0 )
      {
        if (paramRotY)
          paramRotY->set(deg);
        else
          addDouble(comp, roty(), deg);

        quat = Quat(rotX,V3D(1,0,0))*Quat(deg,V3D(0,1,0))*Quat(rotZ,V3D(0,0,1));
      }
      else if ( name.compare(rotz())==0 )
      {
        if (paramRotZ)
          paramRotZ->set(deg);
        else
          addDouble(comp, rotz(), deg);

        quat = Quat(rotX,V3D(1,0,0))*Quat(rotY,V3D(0,1,0))*Quat(deg,V3D(0,0,1));
      }
      else
      {
        g_log.warning() << "addRotationParam() called with unrecognised coordinate symbol: " << name;
        return;
      }

      //clear the position cache
      clearPositionSensitiveCaches();

      // finally add or update "pos" parameter
      if (param)
        param->set(quat);
      else
        addQuat(comp, rot(), quat);
    }

    /**  
     * Adds a double value to the parameter map.
     * @param comp :: Component to which the new parameter is related
     * @param name :: Name for the new parameter
     * @param value :: Parameter value as a string
    */
    void ParameterMap::addDouble(const IComponent* comp,const std::string& name, const std::string& value)
    {
      add(pDouble(),comp,name,value);
    }

    /**
     * Adds a double value to the parameter map.
     * @param comp :: Component to which the new parameter is related
     * @param name :: Name for the new parameter
     * @param value :: Parameter value as a double
    */
    void ParameterMap::addDouble(const IComponent* comp,const std::string& name, double value)
    {
      add(pDouble(),comp,name,value);
    }

    /**  
     * Adds an int value to the parameter map.
     * @param comp :: Component to which the new parameter is related
     * @param name :: Name for the new parameter
     * @param value :: Parameter value as a string
     */
    void ParameterMap::addInt(const IComponent* comp,const std::string& name, const std::string& value)
    {
      add(pInt(),comp,name,value);
    }

    /**
     * Adds an int value to the parameter map.
     * @param comp :: Component to which the new parameter is related
     * @param name :: Name for the new parameter
     * @param value :: Parameter value as an int
     */
    void ParameterMap::addInt(const IComponent* comp,const std::string& name, int value)
    {
      add(pInt(),comp,name,value);
    }

    /**  
     * Adds a bool value to the parameter map.
     * @param comp :: Component to which the new parameter is related
     * @param name :: Name for the new parameter
     * @param value :: Parameter value as a string
     */
    void ParameterMap::addBool(const IComponent* comp,const std::string& name, const std::string& value)
    {
      add(pBool(),comp,name,value);
    }
    /**
     * Adds a bool value to the parameter map.
     * @param comp :: Component to which the new parameter is related
     * @param name :: Name for the new parameter
     * @param value :: Parameter value as a bool
     */
    void ParameterMap::addBool(const IComponent* comp,const std::string& name, bool value)
    {
      add(pBool(),comp,name,value);
    }

    /**  
     * Adds a std::string value to the parameter map.
     * @param comp :: Component to which the new parameter is related
     * @param name :: Name for the new parameter
     * @param value :: Parameter value
    */
    void ParameterMap::addString(const IComponent* comp,const std::string& name, const std::string& value)
    {
      add<std::string>(pString(),comp,name,value);
    }

    /**  
     * Adds a V3D value to the parameter map.
     * @param comp :: Component to which the new parameter is related
     * @param name :: Name for the new parameter
     * @param value :: Parameter value as a string
     */
    void ParameterMap::addV3D(const IComponent* comp,const std::string& name, const std::string& value)
    {
      add(pV3D(),comp,name,value);
      clearPositionSensitiveCaches();
    }

    /**
     * Adds a V3D value to the parameter map.
     * @param comp :: Component to which the new parameter is related
     * @param name :: Name for the new parameter
     * @param value :: Parameter value as a V3D
    */
    void ParameterMap::addV3D(const IComponent* comp,const std::string& name, const V3D& value)
    {
      add(pV3D(),comp,name,value);
      clearPositionSensitiveCaches();
    }

    /**  
     * Adds a Quat value to the parameter map.
     * @param comp :: Component to which the new parameter is related
     * @param name :: Name for the new parameter
     * @param value :: Parameter value as a Quat
    */
    void ParameterMap::addQuat(const IComponent* comp,const std::string& name, const Quat& value)
    {
      add(pQuat(),comp,name,value);
      clearPositionSensitiveCaches();
    }

    /**
     * Does the named parameter exist for the given component and given type
     * @param comp :: The component to be searched
     * @param name :: The name of the parameter
     * @param type :: The type of the component as a string
     * @returns A boolean indicating if the map contains the named parameter. If the type is given then
     * this must also match
     */
    bool ParameterMap::contains(const IComponent* comp, const std::string & name,
                                const std::string & type) const
    {
      if( m_map.empty() ) return false;
      return contains(comp, name.c_str(), type.c_str());
    }

    /**
     * Avoids having to instantiate temporary std::string in method below when called with a string directly
     * @param comp :: The component to be searched as a c-string
     * @param name :: The name of the parameter
     * @return A boolean indicating if the map contains the named parameter.
     */
    bool ParameterMap::contains(const IComponent* comp, const char * name, const char *type) const
    {
      if( m_map.empty() ) return false;
      const ComponentID id = comp->getComponentID();
      std::pair<pmap_cit,pmap_cit> components = m_map.equal_range(id);
      bool anytype = (strlen(type) == 0);
      for( pmap_cit itr = components.first; itr != components.second; ++itr )
      {
        boost::shared_ptr<Parameter> param = itr->second;
        if( boost::iequals(param->name(),name) && (anytype || param->type() == type) )
        {
          return true;
        }
      }
      return false;
    }

    /**
     * @param comp A pointer to a component
     * @param parameter A Parameter object
     * @return true if the combination exists in the map, false otherwise
     */
    bool ParameterMap::contains(const IComponent* comp, const Parameter & parameter) const
    {
      if( m_map.empty() || !comp) return false;

      const ComponentID id = comp->getComponentID();
      pmap_cit it_found = m_map.find(id);
      if(it_found != m_map.end())
      {
        pmap_cit itr = m_map.lower_bound(id);
        pmap_cit itr_end = m_map.upper_bound(id);
        for(; itr != itr_end; ++itr)
        {
          const Parameter_sptr & param = itr->second;
          if(*param == parameter) return true;
        }
        return false;
      }
      else 
        return false;
    }

    /** Return a named parameter of a given type
     * @param comp :: Component to which parameter is related
     * @param name :: Parameter name
     * @param type :: An optional type string
     * @returns The named parameter of the given type if it exists or a NULL shared pointer if not
     */
    Parameter_sptr ParameterMap::get(const IComponent* comp, const std::string& name,
                                     const std::string & type) const
    {
      return get(comp, name.c_str(), type.c_str());
    }


    /** Return a named parameter of a given type. Avoids allocating std::string temporaries
     * @param comp :: Component to which parameter is related
     * @param name :: Parameter name
     * @param type :: An optional type string
     * @returns The named parameter of the given type if it exists or a NULL shared pointer if not
     */
    boost::shared_ptr<Parameter> ParameterMap::get(const IComponent* comp,
                                                   const char *name, const char * type) const
    {
      Parameter_sptr result;
      if(!comp) return result;
      const bool anytype = (strlen(type) == 0);
      PARALLEL_CRITICAL(ParameterMap_get)
      {
        if( !m_map.empty() )
        {
          const ComponentID id = comp->getComponentID();
          pmap_cit it_found = m_map.find(id);
          if (it_found != m_map.end())
          {
            pmap_cit itr = m_map.lower_bound(id);
            pmap_cit itr_end = m_map.upper_bound(id);
            for( ; itr != itr_end; ++itr )
            {
              Parameter_sptr param = itr->second;
              if( boost::iequals(param->nameAsCString(), name) && (anytype || param->type() == type) )
              {
                result = param;
                break;
              }
            }
          }
        }
      }
      return result;
    }

     /** Look for a parameter in the given component by the type of the parameter.
     * @param comp :: Component to which parameter is related
     * @param type :: Parameter type
     * @returns The typed parameter if it exists or a NULL shared pointer if not
     */
    Parameter_sptr ParameterMap::getByType(const IComponent* comp, const std::string& type) const
    {
      Parameter_sptr result = Parameter_sptr();
      PARALLEL_CRITICAL(ParameterMap_get)
      {
        if( !m_map.empty() )
        {
          const ComponentID id = comp->getComponentID();
          pmap_cit it_found = m_map.find(id);
          if(it_found != m_map.end() )
          {
             if (it_found->first)
             {
                pmap_cit itr = m_map.lower_bound(id);
                pmap_cit itr_end = m_map.upper_bound(id);
                for( ; itr != itr_end; ++itr )
                {
                    Parameter_sptr param = itr->second;
                    if( boost::iequals(param->type(), type) )
                    {
                        result = param;
                        break;
                    }
                }
             } // found->firdst
          } // it_found != m_map.end()
        } //!m_map.empty()
      } // PARALLEL_CRITICAL(ParameterMap_get)
      return result;
    }

     /** Looks recursively upwards in the component tree for the first instance of a component with a matching type.
     * @param comp :: The component to start the search with
     * @param type :: Parameter type
     * @returns the first matching parameter.
     */
    Parameter_sptr ParameterMap::getRecursiveByType(const IComponent* comp, const std::string& type) const
    {
      boost::shared_ptr<const IComponent> compInFocus(comp,NoDeleting());
      while( compInFocus != NULL )
      {
        Parameter_sptr param = getByType(compInFocus.get(), type);
        if (param)
        {
          return param;
        }
        compInFocus = compInFocus->getParent();
      }
      //Nothing was found!
      return Parameter_sptr();
    }


    /** 
     * Find a parameter by name, recursively going up the component tree
     * to higher parents.
     * @param comp :: The component to start the search with
     * @param name :: Parameter name
     * @param type :: An optional type string
     * @returns the first matching parameter.
     */
    Parameter_sptr ParameterMap::getRecursive(const IComponent* comp,const std::string& name, 
                                              const std::string & type) const
    {
      return getRecursive(comp, name.c_str(), type.c_str());
    }

    /** 
     * Find a parameter by name, recursively going up the component tree
     * to higher parents.
     * @param comp :: The component to start the search with
     * @param name :: Parameter name
     * @param type :: An optional type string
     * @returns the first matching parameter.
     */
    Parameter_sptr ParameterMap::getRecursive(const IComponent* comp, const char* name, 
                                              const char * type) const
    {
      Parameter_sptr result = this->get(comp->getComponentID(), name, type);
      if(result) return result;

      auto parent = comp->getParent();
      while(parent)
      {
        result = this->get(parent->getComponentID(), name, type);
        if(result) return result;
        parent = parent->getParent();
      }
      return result;
    }

    /**  
     * Return the value of a parameter as a string
     * @param comp :: Component to which parameter is related
     * @param name :: Parameter name
     * @return string representation of the parameter
     */
    std::string ParameterMap::getString(const IComponent* comp, const std::string& name) const
    {
      Parameter_sptr param = get(comp,name);
      if (!param) return "";
      return param->asString();
    }

    /** 
     * Returns a set with all the parameter names for the given component
     * @param comp :: A pointer to the component of interest
     * @returns A set of names of parameters for the given component
     */
    std::set<std::string> ParameterMap::names(const IComponent* comp)const
    {
      std::set<std::string> paramNames;
      const ComponentID id = comp->getComponentID();
      pmap_cit it_found = m_map.find(id);
      if (it_found == m_map.end())
      {
        return paramNames;
      }

      pmap_cit it_begin = m_map.lower_bound(id);
      pmap_cit it_end = m_map.upper_bound(id);

      for(pmap_cit it = it_begin; it != it_end; ++it)
      {
        paramNames.insert(it->second->name());
      }

      return paramNames;
    } 
    
    /**
     * Return a string representation of the parameter map. The format is either:
     * |detID:id-value;param-type;param-name;param-value| for a detector or
     * |comp-name;param-type;param-name;param-value| for other components.
     * @returns A string containing the contents of the parameter map.
     */
    std::string ParameterMap::asString()const
    {
      std::stringstream out;
      for(pmap_cit it=m_map.begin();it!=m_map.end();it++)
      {
        boost::shared_ptr<Parameter> p = it->second;
        if (p && it->first)
        {
          const IComponent* comp = (const IComponent*)(it->first);
          const IDetector* det = dynamic_cast<const IDetector*>(comp);
          if (det)
          {
            out << "detID:"<<det->getID();
          }
          else
          {
            out << comp->getFullName();  // Use full path name to ensure unambiguous naming
          }
          out << ';' << p->type()<< ';' << p->name() << ';' << p->asString() << '|';
        }
      }
      return out.str();
    }

    /**
     * Clears the location, rotation & bounding box caches
     */
    void ParameterMap::clearPositionSensitiveCaches()
    {
      m_cacheLocMap.clear();
      m_cacheRotMap.clear();
      m_boundingBoxMap.clear();
    }
 
    ///Sets a cached location on the location cache
    /// @param comp :: The Component to set the location of
    /// @param location :: The location 
    void ParameterMap::setCachedLocation(const IComponent* comp, const V3D& location) const
    {
      // Call to setCachedLocation is a write so not thread-safe
      PARALLEL_CRITICAL(positionCache)
      {
        m_cacheLocMap.setCache(comp->getComponentID(),location);
      }
    }

    ///Attempts to retreive a location from the location cache
    /// @param comp :: The Component to find the location of
    /// @param location :: If the location is found it's value will be set here
    /// @returns true if the location is in the map, otherwise false
    bool ParameterMap::getCachedLocation(const IComponent* comp, V3D& location) const
    {
      bool inMap(false);
      PARALLEL_CRITICAL(positionCache)
      {
        inMap = m_cacheLocMap.getCache(comp->getComponentID(),location);
      }
      return inMap;
    }

    ///Sets a cached rotation on the rotation cache
    /// @param comp :: The Component to set the rotation of
    /// @param rotation :: The rotation as a quaternion 
    void ParameterMap::setCachedRotation(const IComponent* comp, const Quat& rotation) const
    {
      // Call to setCachedRotation is a write so not thread-safe
      PARALLEL_CRITICAL(rotationCache)
      {
        m_cacheRotMap.setCache(comp->getComponentID(),rotation);
      }
    }

    ///Attempts to retreive a rotation from the rotation cache
    /// @param comp :: The Component to find the rotation of
    /// @param rotation :: If the rotation is found it's value will be set here
    /// @returns true if the rotation is in the map, otherwise false
    bool ParameterMap::getCachedRotation(const IComponent* comp, Quat& rotation) const
    {
      bool inMap(false);
      PARALLEL_CRITICAL(rotationCache)
      {
        inMap = m_cacheRotMap.getCache(comp->getComponentID(),rotation);
      }
      return inMap;
    }

    ///Sets a cached bounding box
    /// @param comp :: The Component to set the rotation of
    /// @param box :: A reference to the bounding box
    void ParameterMap::setCachedBoundingBox(const IComponent *comp, const BoundingBox & box) const
    {
      // Call to setCachedRotation is a write so not thread-safe
      PARALLEL_CRITICAL(boundingBoxCache)
      {
        m_boundingBoxMap.setCache(comp->getComponentID(), box);
      }
    }

    ///Attempts to retreive a bounding box from the cache
    /// @param comp :: The Component to find the bounding box of
    /// @param box :: If the bounding box is found it's value will be set here
    /// @returns true if the bounding is in the map, otherwise false
    bool ParameterMap::getCachedBoundingBox(const IComponent *comp, BoundingBox & box) const
    {
      return m_boundingBoxMap.getCache(comp->getComponentID(),box);
    }

    /**
     * Copy pairs (oldComp->id,Parameter) to the m_map
     * assigning the new newComp->id
     * @param oldComp :: Old component
     * @param newComp :: New component
     * @param oldPMap :: Old map corresponding to the Old component
     */
    void ParameterMap::copyFromParameterMap(const IComponent* oldComp,
                                            const IComponent* newComp, const ParameterMap *oldPMap) 
    {

      std::set<std::string> oldParameterNames = oldPMap->names(oldComp);

      for(auto it = oldParameterNames.begin(); it != oldParameterNames.end(); ++it)
      {
        Parameter_sptr thisParameter = oldPMap->get(oldComp,*it);
        // Insert the fecthed parameter in the m_map
        m_map.insert(std::make_pair(newComp->getComponentID(),thisParameter));
      }
    }
    
    //--------------------------------------------------------------------------------------------
    /** Save the object to an open NeXus file.
     * @param file :: open NeXus file
     * @param group :: name of the group to create
     */
    void ParameterMap::saveNexus(::NeXus::File * file, const std::string & group) const
    {
      file->makeGroup(group, "NXnote", true);
      file->putAttr("version", 1);
      file->writeData("author", "");
      file->writeData("date", Kernel::DateAndTime::getCurrentTime().toISO8601String());
      file->writeData("description", "A string representation of the parameter map. The format is either: |detID:id-value;param-type;param-name;param-value| for a detector or  |comp-name;param-type;param-name;param-value| for other components.");
      file->writeData("type", "text/plain");
      std::string s = this->asString();
      file->writeData("data", s);
      file->closeGroup();
    }

    //--------------------------------------------------------------------------
    // Private methods
    //--------------------------------------------------------------------------
    /**
     *  Retrieve a parameter by either creating a new one of getting an existing one
     * @param[out] created Set to true if the named parameter was newly created, false otherwise
     * @param type :: A string denoting the type, e.g. double, string, fitting
     * @param comp :: A pointer to the component that this parameter is attached to
     * @param name :: The name of the parameter
     */
    Parameter_sptr ParameterMap::retrieveParameter(bool & created, const std::string & type, 
                                                   const IComponent* comp, const std::string & name)
    {
      boost::shared_ptr<Parameter> param;
      if( this->contains(comp, name) )
      {
        param = this->get(comp, name);
        if( param->type() != type )
        {
          throw std::runtime_error("ParameterMap::add - Type mismatch on parameter replacement");
        }
        created = false;
      }
      else
      {
        // Create a new one
        param = ParameterFactory::create(type,name);
        created = true;
      }
      return param;
    }

  } // Namespace Geometry
} // Namespace Mantid

