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


#include "elementReaderTemplates.hpp"
#include "gridDynFileInput.h"
#include "readElement.h"
#include <cstdio>
#include <functional>
#include <iterator>
#include <map>

// A bunch of includes to load these kinds of objects
#include "controllers/AGControl.h"
#include "controllers/reserveDispatcher.h"
#include "controllers/scheduler.h"
#include "generators/gridDynGenerator.h"
#include "gridArea.h"
#include "gridBus.h"
#include "linkModels/gridLink.h"
#include "loadModels/zipLoad.h"
#include "relays/gridRelay.h"
#include "sourceModels/gridSource.h"
#include "submodels/gridControlBlocks.h"
#include "submodels/gridDynExciter.h"
#include "submodels/gridDynGenModel.h"
#include "submodels/gridDynGovernor.h"
#include "submodels/gridDynPSS.h"

using namespace readerConfig;

#define READERSIGNATURE [](std::shared_ptr<readerElement> & cd, readerInfo & ri, coreObject * parent)
static const std::
  map<std::string, std::function<coreObject *(std::shared_ptr<readerElement> &, readerInfo &, coreObject *parent)>>
    loadFunctionMap{
      /* *INDENT-OFF* */
      {"genmodel", READERSIGNATURE{return ElementReader (cd, (gridDynGenModel *)(nullptr), "genmodel", ri, parent);
}
}
, {"exciter", READERSIGNATURE{return ElementReader (cd, (gridDynExciter *)(nullptr), "exciter", ri, parent);
}
}
, {"governor", READERSIGNATURE{return ElementReader (cd, (gridDynGovernor *)(nullptr), "governor", ri, parent);
}
}
, {"pss", READERSIGNATURE{return ElementReader (cd, (gridDynPSS *)(nullptr), "pss", ri, parent);
}
}
, {"source", READERSIGNATURE{return ElementReader (cd, (gridSource *)(nullptr), "source", ri, parent);
}
}
, {"scheduler", READERSIGNATURE{return ElementReader (cd, (scheduler *)(nullptr), "scheduler", ri, parent);
}
}
, {"agc", READERSIGNATURE{return ElementReader (cd, (AGControl *)(nullptr), "agc", ri, parent);
}
}
, {"reservedispatcher",
   READERSIGNATURE{return ElementReader (cd, (reserveDispatcher *)(nullptr), "reservedispatcher", ri, parent);
}
}
, {"controlblock", READERSIGNATURE{return ElementReader (cd, (basicBlock *)(nullptr), "controlblock", ri, parent);
}
}
, {"generator", READERSIGNATURE{return ElementReader (cd, (gridDynGenerator *)(nullptr), "generator", ri, parent);
}
}
, {"load", READERSIGNATURE{return ElementReader (cd, (gridLoad *)(nullptr), "load", ri, parent);
}
}
, {"bus", READERSIGNATURE{return readBusElement (cd, ri, parent);
}
}
, {"relay", READERSIGNATURE{return readRelayElement (cd, ri, parent);
}
}
, {"area", READERSIGNATURE{return readAreaElement (cd, ri, parent);
}
}
, {"link", READERSIGNATURE{return readLinkElement (cd, ri, parent, false);
}
}
, {"econ", READERSIGNATURE{readEconElement (cd, ri, parent);
return parent;
}
}
, {"array", READERSIGNATURE{readArrayElement (cd, ri, parent);
return parent;
}
}
,
{
    "if", READERSIGNATURE
    {
        loadConditionElement (cd, ri, parent);
        return parent;
    }
}
/* *INDENT-ON* */
}
;

static const IgnoreListType customIgnore{"args", "arg1", "arg2", "arg3", "arg4", "arg5",
                                         "arg6", "arg7", "arg8", "arg9", "arg0"};

void loadSubObjects (std::shared_ptr<readerElement> &element, readerInfo &ri, coreObject *parentObject)
{
    // read areas first to set them up for other things to call
    if (element->hasElement ("area"))
    {
        element->moveToFirstChild ("area");
        while (element->isValid ())
        {
            readAreaElement (element, ri, parentObject);
            element->moveToNextSibling ("area");  // next area
        }
        element->moveToParent ();
    }


    // then load the buses as the nodes
    if (element->hasElement ("bus"))
    {
        element->moveToFirstChild ("bus");
        while (element->isValid ())
        {
            readBusElement (element, ri, parentObject);
            element->moveToNextSibling ("bus");  // next bus
        }
        element->moveToParent ();
    }


    element->moveToFirstChild ();
    while (element->isValid ())
    {
        auto fname = convertToLowerCase (element->getName ());
        if ((fname == "bus") || (fname == "area"))
        {
            element->moveToNextSibling ();
            continue;
        }

        if (fname == "local")  // shortcut to do more loading on the parent object most useful in loops to add
                               // stacked parameters and imports
        {
            loadElementInformation (parentObject, element, fname, ri, emptyIgnoreList);
        }
        else
        {
            // std::cout<<"library model :"<<fname<<":\n";
            auto obname = ri.objectNameTranslate (fname);
            if (obname == "collector")
            {
                loadCollectorElement (element, parentObject, ri);
            }
            // event
            else if (obname == "event")
            {
                loadEventElement (element, parentObject, ri);
            }
            else
            {
                auto rval = loadFunctionMap.find (obname);
                if (rval != loadFunctionMap.end ())
                {
                    coreObject *obj = rval->second (element, ri, parentObject);
                    if ((obj->isRoot ()) && (obj != parentObject))
                    {
                        WARNPRINT (READER_WARN_IMPORTANT, obj->getName () << " not owned by any other object");
                    }
                }
                else if (ri.isCustomElement (obname))
                {
                    auto customElementPair = ri.getCustomElement (obname);
                    auto scopeID = ri.newScope ();
                    loadDefines (element, ri);
                    char argVal = '1';
                    std::string argName = "arg";
                    for (int argNum = 1; argNum <= customElementPair.second; ++argVal, ++argNum)
                    {
                        argName.push_back (argVal);
                        auto av = getElementField (element, argName);
                        if (!av.empty ())
                        {
                            ri.addDefinition (argName, av);
                        }
                        else
                        {
                            av = getElementField (customElementPair.first, argName);
                            if (!av.empty ())
                            {
                                ri.addDefinition (argName, av);
                            }
                            else
                            {
                                WARNPRINT (READER_WARN_IMPORTANT,
                                           "custom element " << argName << " not specified");
                            }
                        }
                        argName.pop_back ();
                    }
                    loadElementInformation (parentObject, customElementPair.first, obname, ri, customIgnore);
                    ri.closeScope (scopeID);
                }
            }
        }
        element->moveToNextSibling ();
    }
    element->moveToParent ();
}
