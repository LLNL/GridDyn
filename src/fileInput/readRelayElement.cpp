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

#include "elementReaderTemplates.hpp"
#include "Generator.h"
#include "Area.h"
#include "gridBus.h"
#include "loads/zipLoad.h"
#include "readElement.h"
#include "Relay.h"

namespace griddyn
{
using namespace readerConfig;
static const IgnoreListType relayIgnoreElements{"area", "sink", "source", "target"};
static const std::string relayComponentName = "relay";

// "aP" is the XML element passed from the reader
Relay *readRelayElement (std::shared_ptr<readerElement> &element, readerInfo &ri, coreObject *searchObject)
{
    auto riScope = ri.newScope ();

    // check the rest of the elements

    // boiler plate code to setup the object from references or new object
    // check for the area field
    coreObject *keyObject = searchObject;
    Relay *relay = nullptr;
    searchObject = updateSearchObject<gridPrimary> (element, ri, searchObject);
    if (dynamic_cast<Area *> (searchObject) == nullptr)
    {
        if (searchObject != nullptr)
        {
            searchObject = searchObject->getRoot ();
        }
    }

    std::string name = getElementField (element, "type", defMatchType);
    if (name.empty ())
    {  // if the relay is a subobject of specific type of object then adjust the relay to match
        std::string valType = convertToLowerCase (element->getName ());
        if (valType == relayComponentName)
        {
            name = getElementField (element, "ref", defMatchType);
            if (name.empty ())
            {
                // no type information so generate default relay of a specific type
                if (dynamic_cast<gridBus *> (keyObject) != nullptr)
                {
                    relay = static_cast<Relay *> (
                      coreObjectFactory::instance ()->createObject (relayComponentName, "bus"));
                }
                else if (dynamic_cast<zipLoad *> (keyObject) != nullptr)
                {
                    relay = static_cast<Relay *> (
                      coreObjectFactory::instance ()->createObject (relayComponentName, "load"));
                }
                else if (dynamic_cast<Generator *> (keyObject) != nullptr)
                {
                    relay = static_cast<Relay *> (
                      coreObjectFactory::instance ()->createObject (relayComponentName, "gen"));
                }
            }
        }
    }
    relay = ElementReaderSetup (element, relay, relayComponentName, ri, searchObject);

    coreObject *targetObj = nullptr;
    std::string objName = getElementField (element, "target", defMatchType);
    if (!objName.empty ())
    {
        objName = ri.checkDefines (objName);
        targetObj = locateObject (objName, searchObject);
        if (targetObj == nullptr)
        {
            WARNPRINT (READER_WARN_IMPORTANT, "Unable to locate target object " << objName);
        }
    }

	if (targetObj != nullptr)
    {
        relay->setSource (targetObj);
        relay->setSink (targetObj);
    }
    else
    {
        objName = getElementField (element, "source", defMatchType);
        if (objName.empty ())
        {
            targetObj = keyObject;
        }
        else if (searchObject != nullptr)
        {
            objName = ri.checkDefines (objName);
            targetObj = locateObject (objName, searchObject);
        }
        if (targetObj != nullptr)
        {
            relay->setSource (targetObj);
        }

        objName = getElementField (element, "sink", defMatchType);
        if (objName.empty ())
        {
            targetObj = keyObject;
        }
        else if (searchObject != nullptr)
        {
            objName = ri.checkDefines (objName);
            targetObj = locateObject (objName, searchObject);
        }
        if (targetObj != nullptr)
        {
            relay->setSink (targetObj);
        }
    }

    loadElementInformation (relay, element, relayComponentName, ri, relayIgnoreElements);

    LEVELPRINT (READER_NORMAL_PRINT, "loaded relay " << relay->getName ());

    ri.closeScope (riScope);
    return relay;
}

}//namespace griddyn