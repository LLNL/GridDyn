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
#include "readerHelper.h"
#include "elementReaderTemplates.hpp"
#include "gridDynFileInput.h"
#include "readerElement.h"


#include <map>
#include <functional>
#include <cassert>

using namespace readerConfig;
// "aP" is the XML element passed from the reader
using namespace std::placeholders;

//A bunch of includes to load these kinds of objects
#include "submodels/gridDynGenModel.h"
#include "submodels/gridDynExciter.h"
#include "submodels/gridDynGovernor.h"
#include "submodels/gridDynPSS.h"
#include "submodels/gridControlBlocks.h"
#include "controllers/AGControl.h"
#include "controllers/scheduler.h"
#include "controllers/reserveDispatcher.h"
#include "sourceModels/gridSource.h"
#include "loadModels/zipLoad.h"
#include "generators/gridDynGenerator.h"
#include "gridBus.h"
#include "relays/gridRelay.h"
#include "gridArea.h"
#include "linkModels/gridLink.h"

#define READSIGNATURE [](std::shared_ptr<readerElement> &cd, readerInfo &ri)

static const std::map < std::string, std::function < coreObject *(std::shared_ptr<readerElement> &, readerInfo &)>> loadFunctionMap
{
  /* *INDENT-OFF* */
	  {"genmodel", READSIGNATURE{return ElementReader(cd, (gridDynGenModel *)(nullptr), "genmodel", ri,nullptr); }},
	  {"exciter", READSIGNATURE{return ElementReader(cd, (gridDynExciter *)(nullptr), "exciter", ri, nullptr); }},
	  {"governor", READSIGNATURE{return ElementReader(cd, (gridDynGovernor *)(nullptr), "governor", ri, nullptr); }},
	  {"pss", READSIGNATURE{return ElementReader(cd, (gridDynPSS *)(nullptr), "pss", ri, nullptr); }},
	  {"source", READSIGNATURE{return ElementReader(cd, (gridSource *)(nullptr), "source", ri, nullptr); }},
	  {"controlblock",READSIGNATURE{return ElementReader(cd, (basicBlock *)(nullptr), "controlblock", ri, nullptr); }},
	  {"generator", READSIGNATURE{return ElementReader(cd, (gridDynGenerator *)(nullptr), "generator", ri, nullptr); }},
	  {"load", READSIGNATURE{return ElementReader(cd, (gridLoad *)(nullptr), "load", ri, nullptr); }},
	  {"bus", READSIGNATURE{return readBusElement(cd, ri, nullptr); }},
	  {"relay", READSIGNATURE{return readRelayElement(cd, ri, nullptr); }},
	  {"area", READSIGNATURE{return readAreaElement(cd, ri, nullptr); }},
	  {"link", READSIGNATURE{return readLinkElement(cd, ri, nullptr, false); }},
	  {"scheduler", READSIGNATURE{return ElementReader(cd, (scheduler *)(nullptr), "scheduler", ri, nullptr); }},
	  { "agc", READSIGNATURE{return ElementReader(cd, (AGControl *)(nullptr), "agc", ri, nullptr); }},
	  { "econ", READSIGNATURE{return readEconElement(cd, ri, nullptr); } },
	  { "reservedispatcher", READSIGNATURE{return ElementReader(cd, (reserveDispatcher *)(nullptr), "reserveDispatcher", ri, nullptr); }},
  /* *INDENT-ON* */
};



void readLibraryElement (std::shared_ptr<readerElement> &element, readerInfo &ri)
{

  auto riScope = ri.newScope ();
  //readerInfo xm2;
  std::string baseName = element->getName ();
  element->bookmark ();

  loadDefines (element, ri);
  loadDirectories (element, ri);
  //loop through the other children
  element->moveToFirstChild ();

  while (element->isValid ())
    {
      coreObject  *obj = nullptr;
      std::string fname = convertToLowerCase (element->getName ());
      //std::cout<<"library model :"<<fname<<":\n";
      if ((fname == "define") || (fname == "recorder") || (fname == "event"))
        {
        }
      else
        {
          auto obname = ri.objectNameTranslate (fname);
          auto rval = loadFunctionMap.find (obname);
          if (rval != loadFunctionMap.end ())
            {
              std::string bname = element->getName ();
              obj = rval->second (element, ri);
              assert (bname == element->getName ());
            }
          else
            {
              WARNPRINT (READER_WARN_IMPORTANT, "Unrecognized object type " << fname << " in library");
            }
        }
      if (obj)
        {
          std::vector<gridParameter> pf;
          bool found = ri.addLibraryObject (obj, pf);
          if (found)
            {
              LEVELPRINT (READER_VERBOSE_PRINT, "adding " << fname << " " << obj->getName () << " to Library");
            }
          else
            {
              WARNPRINT (READER_WARN_IMPORTANT, "Duplicate library objects: ignoring second object " << obj->getName ());
			  removeReference(obj);
            }

        }
      element->moveToNextSibling ();
    }

  element->restore ();
  assert (element->getName () == baseName);
  ri.closeScope (riScope);
}

static const std::string defineString ("define");

void loadDefines (std::shared_ptr<readerElement> &element, readerInfo &ri)
{
  if (element->hasElement (defineString) == false)
    {
      return;
    }
  std::string def;
  std::string rep;
  
  //loop through all define elements
  element->moveToFirstChild (defineString);
  while (element->isValid ())
    {
      if (element->hasAttribute ("name"))
        {
          def = element->getAttributeText ("name");
        }
      else if (element->hasAttribute ("string"))
        {
          def = element->getAttributeText ("string");
        }
      else
        {
          WARNPRINT (READER_WARN_ALL, "define element with no " "name" " or " "string" " attribute");
          element->moveToNextSibling (defineString);                                     // next define
          continue;
        }
      if (element->hasAttribute ("value"))
        {
          rep = element->getAttributeText ("value");
        }
      else if (element->hasAttribute ("replacement"))
        {
          rep = element->getAttributeText ("replacement");
        }
      else
        {
          rep = element->getText ();
        }
	  bool locked = false;
      if (element->hasAttribute ("locked"))
        {
          auto lockstr = element->getAttributeText ("locked");
          locked = ((lockstr == "true")||(lockstr == "1")) ? true : false;
        }

      auto kcheck = ri.checkDefines (rep);
      if (def == kcheck)
        {
          WARNPRINT (READER_WARN_ALL, "illegal recursive definition " << def << " name and value are equivalent");
          element->moveToNextSibling ("define");                                     // next define
          continue;
        }
      //check for overloading
      if (element->hasAttribute ("eval"))
        {
          double val = interpretString (rep, ri);
          if (std::isnormal (val))
            {
              if (std::abs (trunc (val) - val) < 1e-9)
                {
                  rep = std::to_string (static_cast<int> (val));
                }
              else
                {
                  rep = std::to_string (val);
                }
            }
        }

      if (locked)
        {
          ri.addLockedDefinition (def, rep);
        }
      else
        {
          ri.addDefinition (def, rep);
        }


      element->moveToNextSibling (defineString);                                 // next define
    }
  element->moveToParent ();

}

static const std::string directoryString ("directory");

void loadDirectories (std::shared_ptr<readerElement> &element, readerInfo &ri)
{
  //loop through all directory elements
  if (!element->hasElement (directoryString))
    {
      return;
    }
  element->moveToFirstChild (directoryString);
  while (element->isValid ())
    {
	  std::string dfld = (element->hasAttribute("value")) ? element->getAttributeText("value") : element->getText();
      
      ri.addDirectory (dfld);
      element->moveToNextSibling (directoryString);
    }
  element->moveToParent ();
}

static const std::string customString ("custom");
void loadCustomSections (std::shared_ptr<readerElement> &element, readerInfo &ri)
{
  if (!element->hasElement (customString))
    {
      return;
    }
  element->moveToFirstChild (customString);
  while (element->isValid ())
    {
      auto name = getElementField (element, "name");
      if (name.empty ())
        {
          WARNPRINT (READER_WARN_ALL, "name not specified for custom object");
          element->moveToNextSibling (customString);
          continue;
        }
      auto args = element->getAttributeValue ("args");
      int nargs = static_cast<int> (args);
      if (args == kNullVal)
        {
          nargs = 0;
        }
      ri.addCustomElement (name, element, nargs);
      element->moveToNextSibling (directoryString);
    }
  element->moveToParent ();
}

static const std::string translateString ("translate");
void loadTranslations (std::shared_ptr<readerElement> &element, readerInfo &ri)
{

  if (!element->hasElement (translateString))
    {
      return;
    }
  //loop through all define elements
  element->moveToFirstChild (translateString);
  while (element->isValid ())
    {
      std::string def;
      if (element->hasAttribute ("name"))
        {
          def = element->getAttributeText ("name");
        }
      else if (element->hasAttribute ("string"))
        {
          def = element->getAttributeText ("string");
        }


      std::string component = element->getAttributeText ("component");

      if ((def.empty ()) && (component.empty ()))
        {
          WARNPRINT (READER_WARN_ALL, "neither name nor component specified in translation");
          element->moveToNextSibling (translateString);
          continue;
        }

      auto kcheck = ri.objectNameTranslate (component);
      if (def == kcheck)
        {
          WARNPRINT (READER_WARN_ALL, "illegal recursive object name translation " << def << " name and value are equivalent");
          element->moveToNextSibling (translateString);
          continue;
        }

      std::string type = element->getAttributeText ("type");

      if (type.empty ())
        {
          if ((def.empty ()) && (component.empty ()))
            {
              WARNPRINT (READER_WARN_ALL, "both name and component must be specified with no type definition");
              element->moveToNextSibling (translateString);
              continue;
            }
          else
            {
              ri.addTranslate (def, component);
            }
        }
      else
        {
          if (def.empty ())
            {
              ri.addTranslateType (component, type);
            }
          else if (component.empty ())
            {
              ri.addTranslateType (def, type);
            }
          else
            {
              ri.addTranslate (def, component, type);
            }

        }

      element->moveToNextSibling (translateString);
    }
  element->moveToParent ();
}

