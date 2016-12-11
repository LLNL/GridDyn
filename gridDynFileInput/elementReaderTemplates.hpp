/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil;  eval: (c-set-offset 'innamespace 0); -*- */
/*
* LLNS Copyright Start
* Copyright (c) 2014, Lawrence Livermore National Security
* This work was performed under the auspices of the U.S. Department
* of Energy by Lawrence Livermore National Laboratory in part under
* Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
* Produced at the Lawrence Livermore National Laboratory.
* All rights reserved.
* For details, see the LICENSE file.
* LLNS Copyright End
*/

#ifndef ELEMENTREADERTEMPLATES_H_

#define ELEMENTREADERTEMPLATES_H_

#include "objectFactory.h"
#include "gridDynFileInput.h"
#include "objectInterpreter.h"
#include "readerHelper.h"
#include "readerElement.h"
#include "readElement.h"
#include "stringOps.h"
#include "core/gridDynExceptions.h"

const IgnoreListType emptyIgnoreList {};

template <class X>
void loadParentInfo (std::shared_ptr<readerElement> &element, X *mobj, readerInfo *ri, coreObject *parentObject)
{
  coreObject *newParentObject = getParent (element, ri, parentObject, parentSearchComponent (mobj));
  if (newParentObject)
    {
      if (mobj->getParent () == nullptr)
        {
          addToParent (mobj, newParentObject);
        }
      else if (mobj->getParent ()->getID () != newParentObject->getID ())
        {
          WARNPRINT (READER_WARN_IMPORTANT, "Parent " << newParentObject->getName () << " specified for " << mobj->getName () << " even though it already has a parent");
        }
    }

  if (mobj->getParent () == nullptr)
    {

      if (parentObject)
        {
          addToParent (mobj, parentObject);
        }
      else
        {
          //set the base power to the system default (usually used for library objects
          mobj->set ("basepower", ri->base);
        }
    }
}

template<class X>
coreObject * updateSearchObject (std::shared_ptr<readerElement> &element, readerInfo *ri, coreObject *parentObject)
{
  coreObject *alternateObject = getParent (element, ri, parentObject, parentSearchComponent (static_cast<X*>(nullptr)));

  return (alternateObject) ? alternateObject : parentObject;
}

const stringVec NumandIndexNames {
  "number","index"
};

const stringVec typeandRetype{ "type","retype" };

template<class X>
X* locateObjectFromElement (std::shared_ptr<readerElement> &element, const std::string &componentName, readerInfo *ri, coreObject *searchObject)
{
  using namespace readerConfig;
  coreObject *obj;
  //try to locate the object if it exists already
  std::string ename = getElementField (element, "name", defMatchType);
  if (!ename.empty ())
    {
      //check if the object can be found in the current search object
      ename = ri->checkDefines (ename);
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
X* buildObject (std::shared_ptr<readerElement> &element, X* mobj, const std::string &componentName, readerInfo *ri, coreObject *searchObject)
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
          valType = ri->checkDefines (valType);
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
          valType = ri->checkDefines (valType);
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
              mobj->clone (rtObj);
              searchObject->remove (mobj);
              mobj->setParent (nullptr);
              condDelete (mobj, mobj);                                               //do a conditional delete here just in case
              searchObject->add (rtObj);                                    //add the new object back into the searchObject
              mobj = rtObj;
            }
        }
    }
  
  if (!preexist)
  {//check for library references if the object didn't exist before
	  std::string ename = getElementField(element, "ref", defMatchType);
	  if (!ename.empty())
	  {
		  ename = ri->checkDefines(ename);
		  auto obj = ri->makeLibraryObject(ename, mobj);
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
				  condDelete(obj, obj);
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
X* ElementReaderSetup (std::shared_ptr<readerElement> &element, X* mobj, const std::string &componentName, readerInfo *ri, coreObject *searchObject)
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
X* ElementReader (std::shared_ptr<readerElement> &element, X* mobj, const std::string &componentName, readerInfo *ri, coreObject *searchObject)
{
  using namespace readerConfig;
  auto riScope = ri->newScope ();

  mobj = ElementReaderSetup (element, mobj, componentName, ri, searchObject);

  loadElementInformation (mobj, element, componentName, ri, emptyIgnoreList);

  ri->closeScope (riScope);
  return mobj;
}

template<class X>
void setAttributes (std::shared_ptr<X> obj, std::shared_ptr<readerElement> &element, const std::string &componentName, readerInfo *ri, const IgnoreListType &ignoreList)
{
  using namespace readerConfig;
  auto att = element->getFirstAttribute ();

  while (att.isValid ())
    {
	  std::string fname = convertToLowerCase (att.getName ());

      auto ifind = ignoreList.find (fname);
      if (ifind != ignoreList.end ())
        {
          att = element->getNextAttribute ();
          continue;
        }
	  try
	  {
		  if (fname.find("file") != std::string::npos)
		  {
			  std::string strVal = att.getText();
			  ri->checkFileParam(strVal);
			  LEVELPRINT(READER_VERBOSE_PRINT, componentName << ": setting " << fname << " to " << strVal);
			  obj->set(fname, strVal);
		  }
		  else
		  {
			  double val = att.getValue();
			  if (val != kNullVal)
			  {
				  LEVELPRINT(READER_VERBOSE_PRINT, componentName << ": setting " << fname << " to " << val);
				  obj->set(fname, val);
			  }
			  else
			  {
				  std::string strVal = att.getText();
				  gridParameter po(fname, strVal);
				  paramStringProcess(&po, ri);
				  if (po.stringType == true)
				  {
					  obj->set(po.field, po.strVal);
					  LEVELPRINT(READER_VERBOSE_PRINT, componentName << ": setting " << fname << " to " << strVal);
				  }
				  else
				  {
					  obj->set(po.field, po.value);
					  LEVELPRINT(READER_VERBOSE_PRINT, componentName << ": setting " << fname << " to " << po.value);
				  }
			  }
		  }
	  }
	  catch (const unrecognizedParameter &)
	  {
		  WARNPRINT(READER_WARN_ALL, "unknown " << componentName << " parameter " << fname);
	  }
	  catch (const invalidParameterValue &)
	  {
		  WARNPRINT(READER_WARN_ALL, "value for parameter " << fname << " (" << att.getText() << ") is invalid");
	  }
	 
      att = element->getNextAttribute ();
    }

}

template<class X>
void setParams (std::shared_ptr<X> obj, std::shared_ptr<readerElement> &element, const std::string &typeName, readerInfo *ri, const IgnoreListType &ignoreList)
{
  using namespace readerConfig;

  gridParameter param;

  element->moveToFirstChild ();
  while (element->isValid ())
    {
      std::string fname = convertToLowerCase (element->getName ());
      auto ifind = ignoreList.find (fname);
      if (ifind != ignoreList.end ())
        {
          element->moveToNextSibling ();
          continue;
        }


      getElementParam (element, &param);
      if (param.valid)
        {
		  try
		  {
			  if (param.stringType == true)
			  {
				  if (param.field.find("file") != std::string::npos)
				  {
					  ri->checkFileParam(param.strVal);
					  LEVELPRINT(READER_VERBOSE_PRINT, typeName << ":setting " << obj->getName() << " file to " << param.strVal);
					  obj->set(param.field, param.strVal);
				  }
				  else
				  {

					  paramStringProcess(&param, ri);
					  if (param.stringType == true)
					  {
						  LEVELPRINT(READER_VERBOSE_PRINT, typeName << ":setting " << obj->getName() << " " << param.field << " to " << param.strVal);
						  obj->set(param.field, param.strVal);
					  }
					  else
					  {
						  LEVELPRINT(READER_VERBOSE_PRINT, typeName << ":setting " << obj->getName() << " " << param.field << " to " << param.value);
						  obj->set(param.field, param.value);
					  }
				  }
			  }
			  else
			  {
				  LEVELPRINT(READER_VERBOSE_PRINT, typeName << ":setting " << obj->getName() << " " << param.field << " to " << param.value);
				  obj->set(param.field, param.value);
			  }
		  }
		  catch (const unrecognizedParameter &)
		  {
			  WARNPRINT(READER_WARN_ALL, "unknown " << typeName << " parameter " << param.field);
		  }
		  catch (const invalidParameterValue &)
		  {
			  if (param.stringType == true)
			  {
				  WARNPRINT(READER_WARN_ALL, "value for " << typeName << " parameter " << param.field << " (" << param.strVal << ") is invalid");
			  }
			  else
			  {
				  WARNPRINT(READER_WARN_ALL, "value for solver parameter " << param.field << " (" << param.value << ") is invalid");
			  }
		  }
        }
      element->moveToNextSibling ();
    }
  element->moveToParent ();
}


#endif
