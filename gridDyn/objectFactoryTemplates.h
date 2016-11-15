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

#ifndef GD_OBJECT_FACTORY_TEMPLATES_H_
#define GD_OBJECT_FACTORY_TEMPLATES_H_

#include "objectFactory.h"

/** @brief template class for object ownership*/
template <class Ntype>
class gridObjectHolder : public gridCoreObject
{
  static_assert (std::is_base_of<gridCoreObject, Ntype>::value, "holder object must have gridCoreObject as base");
private:
  std::vector<Ntype> objArray;
  count_t next = 0;
  count_t objCount = 0;
public:
  explicit gridObjectHolder (count_t objs) : gridCoreObject ("holder_#"),objArray (objs), objCount (objs)
  {
    for (auto &so : objArray)
      {
        so.setOwner (nullptr, this);
      }
  }
  ~gridObjectHolder ()
  {
    for (auto &so : objArray)
      {
        if (so.getParent ())
          {
            so.getParent ()->remove (&so);
          }
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

  gridCoreObject * getRoot () const
  {
    if (parent)
      {
        return parent;
      }
    //loop down from the most recent used to get the most recently established root object
    for (int pp = static_cast<int> (next - 1); pp >= 0; --pp)
      {
        if (objArray[pp].getParent ())
          {
            return objArray[pp].getParent ()->find ("root");
          }
      }

    return nullptr;
  }

};

/** @brief template class for object construction */
template <class Ntype>
class typeFactory : public objectFactory
{
  static_assert (std::is_base_of<gridCoreObject, Ntype>::value, "factory class must have gridCoreObject as base");
private:
  std::shared_ptr<gridObjectHolder<Ntype>> obptr = nullptr;
  bool useBlock = false;
  count_t targetprepped = 0;
public:
  typeFactory (const std::string &componentName, const std::string &typeName) : objectFactory (componentName, typeName)
  {
    auto tF = coreObjectFactory::instance ()->getFactory (componentName);
    tF->registerFactory (typeName, this);
  }

  typeFactory (const std::string &componentName, const stringVec &typeNames) : objectFactory (componentName, typeNames)
  {

    auto tF = coreObjectFactory::instance ()->getFactory (componentName);
    for (auto tname : typeNames)
      {
        tF->registerFactory (tname, this);
      }
  }
  typeFactory (const std::string &componentName, const stringVec &typeNames, const std::string &defType) : objectFactory (componentName, typeNames)
  {
    auto tF = coreObjectFactory::instance ()->getFactory (componentName);
    for (auto tname : typeNames)
      {
        tF->registerFactory (tname, this);
      }
    tF->setDefault (defType);
  }

  gridCoreObject * makeObject () override
  {
    gridCoreObject *ret = makeTypeObject ();
    return ret;
  }

  gridCoreObject * makeObject (const std::string &objName) override
  {
    gridCoreObject *ret = makeTypeObject (objName);
    return ret;
  }

  virtual Ntype * makeTypeObject (const std::string &objName = "")
  {
    Ntype *ret = nullptr;
    if (useBlock)
      {
        ret = obptr->getNext ();
        if (!ret)
          {                                   //means the block was used up
            if (targetprepped > 0)                                  //if we are scheduled for more make a new objectHolder
              {
                auto root = obptr->getRoot ();
                if (root)
                  {
                    if (obptr->getParent () != root)
                      {
                        root->addsp (obptr);
                      }
                  }
                obptr = std::make_shared<gridObjectHolder<Ntype>> (targetprepped);
                if (!obptr)
                  {
                    root->log (root, print_level::warning, "unable to create container object");
                    useBlock = false;
                  }
                else
                  {
                    if (root)
                      {
                        root->addsp (obptr);
                      }
                    ret = obptr->getNext ();
                    useBlock = true;
                  }
                targetprepped = 0;
              }
            else
              {
                useBlock = false;
                obptr = nullptr;
              }
          }
        if (ret)
          {
            if (!objName.empty ())
              {
                ret->setName (objName);
              }
          }
      }
    if (ret == nullptr)              //if we fail on all counts just make a new object
      {
        if (objName.empty ())
          {
            ret = new Ntype ();
          }
        else
          {
            ret = new Ntype (objName);
          }
      }
    return ret;
  }

  virtual void prepObjects (count_t count, gridCoreObject *obj) override
  {
    auto root = obj->find ("root");
    useBlock = true;
    if ((obptr) && (root))
      {
        if (obptr->getParent () != root)
          {
            root->addsp (obptr);
          }
      }
    if (remainingPrepped () < count)
      {
        if ((obptr) && (obptr->remaining () > 0))
          {
            targetprepped = count - obptr->remaining ();
          }
        else
          {
            obptr = std::make_shared<gridObjectHolder<Ntype>> (targetprepped);
            if (root)
              {
                if (!obptr)
                  {
                    root->log (root, print_level::warning, "unable to create container object");
                    useBlock = false;
                  }
                else
                  {
                    root->addsp (obptr);
                  }
              }
            else
              {
                useBlock = false;
              }
          }
      }

  }
  virtual count_t remainingPrepped () const override
  {
    return (obptr) ? (obptr->remaining () + targetprepped) : 0;
  }

  virtual std::shared_ptr<gridCoreObject> getHolder () const
  {
    return obptr;
  }
};

/** @brief class for templating inherited object factories to cascade correctly*/
template <class Ntype, class Btype>
class childTypeFactory : public typeFactory<Btype>
{
  static_assert (std::is_base_of<gridCoreObject, Btype>::value, "factory class must have gridCoreObject as base");
  static_assert (std::is_base_of<Btype, Ntype>::value, "factory class types must have parent child relationship");
private:
  std::shared_ptr<gridObjectHolder<Ntype>> obptr;
  bool useBlock = false;
  count_t targetprepped = 0;
public:
  childTypeFactory (const std::string &componentName, const std::string &typeName) : typeFactory<Btype> (componentName, typeName)
  {
  }

  childTypeFactory (const std::string &componentName, const stringVec &typeNames) : typeFactory<Btype> (componentName, typeNames)
  {

  }
  childTypeFactory (const std::string &componentName, const stringVec &typeNames, const std::string &defType) : typeFactory<Btype> (componentName, typeNames, defType)
  {

  }
  gridCoreObject * makeObject () override
  {
    gridCoreObject *ret = makeTypeObject ();

    return ret;
  }

  gridCoreObject * makeObject (const std::string &objName) override
  {
    gridCoreObject *ret = makeTypeObject (objName);

    return ret;
  }

  Btype * makeTypeObject (const std::string &objName = "") override         //done this way to make sure calling makeTypeObject on the parent works in the correct polymorphic call
  {
    Ntype *ret = nullptr;
    if (useBlock)
      {
        ret = obptr->getNext ();
        if (!ret)
          {                                           //means the block was used up
            if (targetprepped > 0)                                  //if we are scheduled for more make a new objectHolder
              {
                auto root = obptr->getRoot ();
                if (root)
                  {
                    if (obptr->getParent () != root)
                      {
                        root->addsp (obptr);
                      }
                  }
                obptr = std::make_shared<gridObjectHolder<Ntype>> (targetprepped);
                if (!obptr)
                  {
                    root->log (root, print_level::warning, "unable to create container object");
                    useBlock = false;
                  }
                else
                  {
                    if (root)
                      {
                        root->addsp (obptr);
                      }
                    ret = obptr->getNext ();
                    useBlock = true;
                  }
                targetprepped = 0;
              }
            else
              {
                useBlock = false;
                obptr = nullptr;
              }
          }
        if (ret)
          {
            if (!objName.empty ())
              {
                ret->setName (objName);
              }
          }
      }
    if (ret == nullptr)                    //if we fail on all counts just make a new object
      {
        if (objName.empty ())
          {
            ret = new Ntype ();
          }
        else
          {
            ret = new Ntype (objName);
          }

      }
    return ret;
  }

  virtual void prepObjects (count_t count, gridCoreObject *obj) override
  {

    auto root = obj->find ("root");
    useBlock = true;
    if ((obptr) && (root))
      {
        if (obptr->getParent () != root)
          {
            root->addsp (obptr);
          }
      }
    if (remainingPrepped () < count)
      {
        if ((obptr) && (obptr->remaining () > 0))
          {
            targetprepped = count - obptr->remaining ();
          }
        else
          {
            obptr = std::make_shared<gridObjectHolder<Ntype>> (targetprepped);
            if (root)
              {
                if (!obptr)
                  {
                    root->log (root, print_level::warning, "unable to create container object");
                    useBlock = false;
                  }
                else
                  {
                    root->addsp (obptr);
                  }
              }
            else
              {
                useBlock = false;
              }
          }
      }
  }
  virtual count_t remainingPrepped () const override
  {
    return (obptr) ? (obptr->remaining () + targetprepped) : 0;
  }
  virtual std::shared_ptr<gridCoreObject> getHolder () const override
  {
    return obptr;
  }
};

template <class Ntype, class argType>
class typeFactoryArg : public objectFactory
{
  static_assert (std::is_base_of<gridCoreObject, Ntype>::value, "factory class must have gridCoreObject as base");
  static_assert (!std::is_same<argType, std::string>::value, "arg type cannot be a std::string");
public:
  argType arg;
  typeFactoryArg (const std::string &componentName, const std::string &typeName, const argType iArg) : objectFactory (componentName, typeName), arg (iArg)
  {
    auto tF = coreObjectFactory::instance ()->getFactory (componentName);
    tF->registerFactory (typeName, this);
  }

  typeFactoryArg (const std::string &componentName, const stringVec &typeNames, const argType iArg) : objectFactory (componentName, typeNames), arg (iArg)
  {

    auto tF = coreObjectFactory::instance ()->getFactory (componentName);
    for (auto tname : typeNames)
      {
        tF->registerFactory (tname, this);
      }
  }

  gridCoreObject * makeObject () override
  {
    return static_cast<gridCoreObject *> (new Ntype (arg));
  }

  gridCoreObject * makeObject (const std::string &objName) override
  {
    return static_cast<gridCoreObject *> (new Ntype (arg,objName));
  }

  Ntype * makeTypeObject (const std::string &objName = "")
  {
    if (objName.empty ())
      {
        return(new Ntype (arg));
      }
    else
      {
        return (new Ntype (arg, objName));
      }

  }
};


/**
* @brief template for cloneBase using an object Factory to make new objects instead of new
* @param[in] object of class A to be cloned
* @param[in] pointer of an object to clone to or a null pointer if a new object needs to be created
@param[in] factory making the new object
* @return pointer to the cloned object
*/
template<class A, class B>
A * cloneBaseFactory (const A *bobj, gridCoreObject *obj, objectFactory *cfact)
{
  static_assert (std::is_base_of<B, A>::value, "classes A and B must have parent child relationship");
  static_assert (std::is_base_of<gridCoreObject, B>::value, "classes must be inherited from gridCoreObject");
  static_assert (std::is_base_of<gridCoreObject, A>::value, "classes must be inherited from gridCoreObject");
  A *nobj;
  if (obj == nullptr)
    {
      nobj = static_cast<A *> (cfact->makeObject (bobj->getName()));
    }
  else
    {
      nobj = dynamic_cast<A *> (obj);
      if (nobj == nullptr)
        {
          //if we can't cast the pointer clone at the next lower level
          bobj->B::clone (obj);
          return nullptr;
        }
    }
  bobj->B::clone (nobj);
  return nobj;
}
#endif

