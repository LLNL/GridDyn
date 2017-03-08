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

#include "readElement.h"
#include "elementReaderTemplates.hpp"
#include "relays/gridRelay.h"
#include "gridBus.h"
#include "loadModels/zipLoad.h"
#include "generators/gridDynGenerator.h"
#include "gridArea.h"

using namespace readerConfig;
static const IgnoreListType relayIgnoreElements {
  "area", "sink", "source", "target"
};
static const std::string relayComponentName = "relay";

// "aP" is the XML element passed from the reader
gridRelay * readRelayElement (std::shared_ptr<readerElement> &element, readerInfo &ri, coreObject *searchObject)
{

  auto riScope = ri.newScope ();

  // check the rest of the elements

  //boiler plate code to setup the object from references or new object
  //check for the area field
  coreObject *keyObject = searchObject;
  gridRelay *relay = nullptr;
  searchObject = updateSearchObject<gridPrimary> (element, ri, searchObject);
  if (!(dynamic_cast<gridArea *> (searchObject)))
    {
      if (searchObject)
        {
          searchObject = searchObject->getRoot();
        }
    }


  std::string name = getElementField (element, "type", defMatchType);
  if (name.empty ())
    {     //if the relay is a subobject of specific type of object then adjust the relay to match
      std::string valType = convertToLowerCase (element->getName ());
      if (valType == relayComponentName)
        {
          name = getElementField (element, "ref", defMatchType);
          if (name.empty ())
            {
              //no type information so generate default relay of a specific type
              if (dynamic_cast<gridBus *> (keyObject))
                {
                  relay = static_cast<gridRelay *> (coreObjectFactory::instance ()->createObject (relayComponentName, "bus"));
                }
              else if (dynamic_cast<zipLoad *> (keyObject))
                {
                  relay = static_cast<gridRelay *> (coreObjectFactory::instance ()->createObject (relayComponentName, "load"));
                }
              else if (dynamic_cast<gridDynGenerator *> (keyObject))
                {
                  relay = static_cast<gridRelay *> (coreObjectFactory::instance ()->createObject (relayComponentName, "gen"));
                }
            }
        }
    }
  relay = ElementReaderSetup (element, relay, relayComponentName, ri, searchObject);

  coreObject *targetObj = nullptr;
  std::string objname = getElementField (element, "target", defMatchType);
  if (!objname.empty ())
    {
      objname = ri.checkDefines (objname);
      targetObj = locateObject (objname, searchObject);
      if (targetObj == nullptr)
        {
          WARNPRINT (READER_WARN_IMPORTANT, "Unable to locate target object " << objname);
        }
    }

  if (targetObj)
    {
      relay->setSource (targetObj);
      relay->setSink (targetObj);
    }
  else
    {

      objname = getElementField (element, "source", defMatchType);
      if (objname.empty ())
        {
          targetObj = keyObject;
        }
      else if (searchObject)
        {
          objname = ri.checkDefines (objname);
          targetObj = locateObject (objname, searchObject);
        }
      if (targetObj)
        {
          relay->setSource (targetObj);
        }

      objname = getElementField (element, "sink", defMatchType);
      if (objname.empty ())
        {
          targetObj = keyObject;
        }
      else if (searchObject)
        {
          objname = ri.checkDefines (objname);
          targetObj = locateObject (objname, searchObject);
        }
      if (targetObj)
        {
          relay->setSink (targetObj);
        }
    }

  loadElementInformation (relay, element, relayComponentName, ri, relayIgnoreElements);

  LEVELPRINT (READER_NORMAL_PRINT, "loaded relay " << relay->getName ());

  ri.closeScope (riScope);
  return relay;
}
