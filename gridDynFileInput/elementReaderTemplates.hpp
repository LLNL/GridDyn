/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil;  eval: (c-set-offset 'innamespace 0); -*- */
/*
* LLNS Copyright Start
* Copyright (c) 2017, Lawrence Livermore National Security
* This work was performed under the auspices of the U.S. Department
* of Energy by Lawrence Livermore National Laboratory in part under
* Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
* Produced at the Lawrence Livermore National Laboratory.
* All rights reserved.
* For details, see the LICENSE file.
* LLNS Copyright End
*/
#pragma once
#ifndef ELEMENT_READER_TEMPLATES_H_
#define ELEMENT_READER_TEMPLATES_H_

#include "core/objectFactory.h"
#include "gridDynFileInput.h"
#include "core/objectInterpreter.h"
#include "readerHelper.h"
#include "formatInterpreters/readerElement.h"
#include "readElement.h"
#include "utilities/stringOps.h"
#include "core/coreExceptions.h"

const IgnoreListType emptyIgnoreList {};

template <class X>
void loadParentInfo (std::shared_ptr<readerElement> &element, X *mobj, readerInfo &ri, coreObject *parentObject)
{
  coreObject *newParentObject = getParent (element, ri, parentObject, parentSearchComponent (mobj));
  if (newParentObject)
    {
      if (mobj->isRoot())
        {
          addToParent (mobj, newParentObject);
        }
      else if (!isSameObject(mobj->getParent (),newParentObject))
        {
          WARNPRINT (READER_WARN_IMPORTANT, "Parent " << newParentObject->getName () << " specified for " << mobj->getName () << " even though it already has a parent");
        }
    }

  if (mobj->isRoot())
    {

      if (parentObject)
        {
          addToParent (mobj, parentObject);
        }
      else
        {
          //set the base power to the system default (usually used for library objects
          mobj->set ("basepower", ri.base);
        }
    }
}

template<class X>
coreObject * updateSearchObject (std::shared_ptr<readerElement> &element, readerInfo &ri, coreObject *parentObject)
{
  coreObject *alternateObject = getParent (element, ri, parentObject, parentSearchComponent (static_cast<X*>(nullptr)));

  return (alternateObject) ? alternateObject : parentObject;
}

const stringVec NumandIndexNames {
  "number","index"
};

const stringVec typeandRetype{ "type","retype" };

template<class X>
X* locateObjectFromElement (std::shared_ptr<readerElement> &element, const std::string &componentName, readerInfo &ri, coreObject *searchObject)
{
  using namespace readerConfig;
  coreObject *obj;
  //try to locate the object if it exists already
  std::string ename = getElementField (element, "name", defMatchType);
  if (!ename.empty ())
    {
      //check if the object can be found in the current search object
      ename = ri.checkDefines (ename);
      if ((obj = searchObject->find (ename)) != nullptr)
        {
          return dynamic_cast<X *> (obj);
        }
    }

  ename = getElementFieldOptions (element, NumandIndexNames, defMatchType);
  if (!ename.empty ())
    {
      //check if the object can be found in the current search object
      double val = interpretString (ename, ri);
      auto index = static_cast<index_t> (val);
      if ((obj = searchObject->getSubObject (componentName, index)) != nullptr)
        {
          return dynamic_cast<X *> (obj);
        }
    }

  ename = getElementField (element, "id", defMatchType);
  if (!ename.empty())
  {
    //check if the object can be found in the current search object
    double val = interpretString (ename, ri);
    auto index = static_cast<index_t> (val);
    if ((obj = searchObject->findByUserID (componentName, index)) != nullptr)
      {
        return dynamic_cast<X *> (obj);
      }
  }
  return nullptr;
}

template<class X>
X* buildObject (std::shared_ptr<readerElement> &element, X* mobj, const std::string &componentName, readerInfo &ri, coreObject *searchObject)
{
  using namespace readerConfig;
  auto cof = coreObjectFactory::instance ();
  auto tf = cof->getFactory (componentName);
  auto objectName = getObjectName (element, ri);
  bool preexist = (mobj != nullptr);
  if (mobj == nullptr)
    {
      //check if type is explicit
      std::string valType = getElementFieldOptions (element, typeandRetype, defMatchType);
      if (!valType.empty ())
        {
          valType = ri.checkDefines (valType);
          makeLowerCase (valType);
          if (!tf->isValidType (valType))
            {
              WARNPRINT (READER_WARN_IMPORTANT, "unknown " << componentName << " type (" << valType << ") reverting to default type ");
            }
          if (objectName.empty ())
            {
              mobj = dynamic_cast<X *> (tf->makeObject (valType));
            }
          else
            {
              mobj = dynamic_cast<X *> (tf->makeObject (valType,objectName));
            }
          if (mobj == nullptr)
            {
              WARNPRINT (READER_WARN_IMPORTANT, "unknown " << componentName << " type " << valType);
            }
        }
      else
        {                 //check if there was a type in the element name
          valType = element->getName ();
          if (valType != componentName)
            {
              if (objectName.empty ())
                {
                  mobj = dynamic_cast<X *> (tf->makeObject (valType));
                }
              else
                {
                  mobj = dynamic_cast<X *> (tf->makeObject (valType,objectName));
                }
              if (mobj == nullptr)
                {
                  WARNPRINT (READER_WARN_IMPORTANT, "unknown " << componentName << " type " << valType);
                }
            }
        }
    }
  else
    {
      //if we need to do a type override

      std::string valType = getElementField (element, "retype", defMatchType);
      if (!valType.empty ())
        {
          valType = ri.checkDefines (valType);
          makeLowerCase (valType);
          if (!tf->isValidType (valType))
            {
              WARNPRINT (READER_WARN_IMPORTANT, "unknown " << componentName << " retype reverting to default type" << valType);
            }
          X* rtObj = dynamic_cast<X *> (tf->makeObject (valType, mobj->getName ()));

          if (rtObj == nullptr)
            {
              WARNPRINT (READER_WARN_IMPORTANT, "unknown " << componentName << " retype " << valType);
            }
          else
            {
			  // the reference count in the object needs to be incremented so we are in control of the object
			  //deletion,  the remove call will (most likely) call delete but in case it doesn't we still need
			  //access to it here to ensure it gets deleted by the call to remove ref,  otherwise we either run the risk of 
			  //a memory leak or dereferencing an invalid pointer, so we just claim ownership for a short time to alleviate the issue.
              mobj->clone (rtObj);
			  mobj->addOwningReference();
              searchObject->remove (mobj);
              removeReference (mobj, searchObject);                       //decrement the reference again
              searchObject->add (rtObj);                                    //add the new object back into the searchObject
              mobj = rtObj;  //move the pointer to the new object
            }
        }
    }
  
  if (!preexist)
  {//check for library references if the object didn't exist before
	  std::string ename = getElementField(element, "ref", defMatchType);
	  if (!ename.empty())
	  {
		  ename = ri.checkDefines(ename);
		  auto obj = ri.makeLibraryObject(ename, mobj);
		  if (obj == nullptr)
		  {
			  WARNPRINT(READER_WARN_IMPORTANT, "unable to locate reference object " << ename << " in library");
		  }
		  else
		  {
			  mobj = dynamic_cast<X *> (obj);
			  if (mobj == nullptr)
			  {
				  WARNPRINT(READER_WARN_IMPORTANT, "Invalid reference object " << ename << ": wrong type");
				  removeReference(obj);
			  }
			  else
			  {
				  if (!objectName.empty())
				  {
					  mobj->setName(objectName);
				  }
			  }
		  }
	  }
  }
 
  //make the default object of componentName
  if (mobj == nullptr)
    {
      if (objectName.empty ())
        {
          mobj = dynamic_cast<X *> (tf->makeObject ());
        }
      else
        {
          mobj = dynamic_cast<X *> (tf->makeObject ("",objectName));
        }
      if (mobj == nullptr)
        {
          WARNPRINT (READER_WARN_IMPORTANT, "Unable to create object " << componentName);
          return nullptr;
        }
    }
  return mobj;
}

template<class X>
X* ElementReaderSetup (std::shared_ptr<readerElement> &element, X* mobj, const std::string &componentName, readerInfo &ri, coreObject *searchObject)
{
  using namespace readerConfig;

  loadDefines (element, ri);
  loadDirectories (element, ri);
  searchObject = updateSearchObject<X> (element, ri, searchObject);
  if ((mobj == nullptr) && (searchObject))
    {
      mobj = locateObjectFromElement<X> (element, componentName, ri, searchObject);
    }

  mobj = buildObject (element, mobj, componentName, ri, searchObject);

  setIndex (element, mobj,ri);

  loadParentInfo (element, mobj, ri,searchObject);
  //locate a parent if any

  return mobj;
}

template<class X>
X* ElementReader (std::shared_ptr<readerElement> &element, X* mobj, const std::string &componentName, readerInfo &ri, coreObject *searchObject)
{
  using namespace readerConfig;
  auto riScope = ri.newScope ();

  mobj = ElementReaderSetup (element, mobj, componentName, ri, searchObject);

  loadElementInformation (mobj, element, componentName, ri, emptyIgnoreList);

  ri.closeScope (riScope);
  return mobj;
}



#endif
