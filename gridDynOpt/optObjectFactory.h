/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil;  eval: (c-set-offset 'innamespace 0); -*- */
/*
 * LLNS Copyright Start
 * Copyright (c) 2016, Lawrence Livermore National Security
 * This work was performed under the auspices of the U.S. Department
 * of Energy by Lawrence Livermore National Laboratory in part under
 * Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
 * Produced at the Lawrence Livermore National Laboratory.
 * All rights reserved.
 * For details, see the LICENSE file.
 * LLNS Copyright End
*/

#ifndef GD_OPT_OBJECT_FACTORY_H_
#define GD_OPT_OBJECT_FACTORY_H_

#include "gridCore.h"
#include "gridOptObjects.h"
#include <map>
#include <vector>
#include <memory>

#include <type_traits>


// class definitions for the object factories that can create the objects
//cFactory is a virtual base class for object Construction functions
class optFactory
{
public:
  std::string  name;
  int m_level = 0;
  optFactory (const std::string & /*componentName*/, const std::string objName, int level = 0) : name (objName),m_level (level)
  {

  }
  optFactory (const stringVec & /*componentName*/, const std::string objName, int level = 0) : name (objName),m_level (level)
  {
  }
  virtual gridOptObject * makeObject (gridCoreObject *obj) = 0;
  virtual gridOptObject * makeObject () = 0;
  virtual void prepObjects (count_t /*count*/, gridCoreObject * /*obj*/)
  {
  }
  virtual count_t remainingPrepped () const
  {
    return 0;
  }
  virtual bool testObject (gridCoreObject *)
  {
    return true;
  }
};

typedef std::map<std::string, optFactory *> optMap;

class optComponentFactory
{
public:
  std::string  name;
  optComponentFactory ()
  {
  }
  optComponentFactory (const std::string typeName);
  ~optComponentFactory ();
  stringVec getObjNames ();
  gridOptObject * makeObject (gridCoreObject *obj);
  gridOptObject * makeObject (const std::string &objType);
  gridOptObject * makeObject ();
  void registerFactory (optFactory *optFac);
  bool isValidObject (const std::string &objName);
  optFactory * getFactory (const std::string &objName);
protected:
  optMap m_factoryMap;
  std::vector<optFactory *> m_factoryList;
};

//create a high level object factory for the coreObject class
typedef std::map<std::string, std::shared_ptr<optComponentFactory>> optfMap;

class coreOptObjectFactory
{
public:
  /** public destructor
  * Destructor must be public to work with shared_ptr
  */
  ~coreOptObjectFactory ()
  {
  }
  static std::shared_ptr<coreOptObjectFactory> instance ();
  void registerFactory (const std::string name, std::shared_ptr<optComponentFactory> tf);
  void registerFactory (std::shared_ptr<optComponentFactory> tf);
  stringVec getFactoryNames ();
  stringVec getObjNames (const std::string &typeName);
  gridOptObject * createObject (const std::string &optComponet, const std::string &objName);
  gridOptObject * createObject (const std::string &optComponent, gridCoreObject *obj);
  gridOptObject * createObject (gridCoreObject *obj);
  gridOptObject * createObject (const std::string &objName);
  std::shared_ptr<optComponentFactory> getFactory (const std::string &optComponent);
  bool isValidType (const std::string &obComponent);
  bool isValidObject (const std::string &optComponent, const std::string &objName);
  void setDefaultType (const std::string defComponent);
  void prepObjects (const std::string &optComponent, const std::string &optName, count_t numObjects, gridCoreObject *baseObj);
  void prepObjects (const std::string &objName, count_t numObjects, gridCoreObject *baseObj);
private:
  coreOptObjectFactory ()
  {
  }

  optfMap m_factoryMap;
  std::string m_defaultType;

};

/*** template class for opt object ownership*/
template <class Ntype, class gdType>
class gridOptObjectHolder : public gridCoreObject
{
  static_assert (std::is_base_of<gridOptObject, Ntype>::value, "opt class must have a base class of gridOptObject");
  static_assert (std::is_base_of<gridCoreObject, gdType>::value, "gridDyn class must have base class type of gridCoreObject");
private:
  std::vector<Ntype> objArray;
  count_t next = 0;
  count_t objCount = 0;
public:
  gridOptObjectHolder (count_t objs) : objArray (objs), objCount (objs)
  {
    for (auto &so : objArray)
      {
        so.setOwner (nullptr,this);
      }
  }
  Ntype * getNext ()
  {
    Ntype *obj = nullptr;
    if (next < objCount)
      {
        obj = &(objArray[next]);
        ++next;
      }
    return obj;
  }

  count_t remaining () const
  {
    return objCount - next;
  }


};



//opt factory is a template class that inherits from cFactory to actually to the construction of a specific object
template <class Ntype, class gdType>
class optObjectFactory : public optFactory
{
  static_assert (std::is_base_of<gridOptObject, Ntype>::value, "opt class must have a base class of gridOptObject");
  static_assert (std::is_base_of<gridCoreObject, gdType>::value, "gridDyn class must have base class type of gridCoreObject");
private:
  bool useBlock = false;
  gridOptObjectHolder<Ntype, gdType> *gOOH = nullptr;
public:
  optObjectFactory (const std::string &componentName, const std::string objName, int level = 0,bool makeDefault = false) : optFactory (componentName, objName,level)
  {

    auto coof = coreOptObjectFactory::instance ();
    auto fac = coof->getFactory (componentName);
    fac->registerFactory (this);
    if (makeDefault)
      {
        coof->setDefaultType (componentName);
      }

  }

  optObjectFactory (const stringVec &componentNames, const std::string objName, int level = 0, bool makeDefault = false) : optFactory (componentNames[0],objName, level)
  {
    auto coof = coreOptObjectFactory::instance ();
    for (auto &tname : componentNames)
      {
        coof->getFactory (tname)->registerFactory ( this);
      }
    if (makeDefault)
      {
        coof->setDefaultType (componentNames[0]);
      }
  }

  bool testObject (gridCoreObject *obj) override
  {
    if (dynamic_cast<gdType *> (obj))
      {
        return true;
      }
    else
      {
        return false;
      }
  }

  gridOptObject * makeObject (gridCoreObject *obj) override
  {
    gridOptObject *ret = nullptr;
    if (useBlock)
      {
        ret = gOOH->getNext ();
        if (ret == nullptr)
          {                     //means the block was used up
            useBlock = false;
            gOOH = nullptr;
          }
        else
          {
            ret->add (obj);
          }
      }
    if (ret == nullptr)
      {
        ret = new Ntype (obj);
      }
    return ret;
  }

  gridOptObject * makeObject () override
  {
    gridOptObject *ret = nullptr;
    if (useBlock)
      {
        ret = gOOH->getNext ();
        if (ret == nullptr)
          {                     //means the block was used up
            useBlock = false;
            gOOH = nullptr;
          }
      }
    if (ret == nullptr)
      {
        ret = new Ntype ();
      }
    return ret;
  }

  Ntype * makeTypeObject (gridCoreObject *obj)
  {
    Ntype *ret = nullptr;
    if (useBlock)
      {
        ret = gOOH->getNext ();
        if (ret == nullptr)
          {                     //means the block was used up
            useBlock = false;
            gOOH = nullptr;
          }
        else
          {
            ret->add (obj);
          }
      }
    if (ret == nullptr)
      {
        ret = new Ntype (obj);
      }
    return ret;
  }

  virtual void prepObjects (count_t count, gridCoreObject *obj) override
  {
    auto root = obj->find ("root");
    gOOH = new gridOptObjectHolder<Ntype,gdType> (count);
    root->add (gOOH);
    useBlock = true;

  }
  virtual count_t remainingPrepped () const override
  {
    return (gOOH) ? (gOOH->remaining ()) : 0;
  }
};








#endif
