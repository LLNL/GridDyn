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

#include "readElement.h"
#include "readerHelper.h"
#include "elementReaderTemplates.hpp"
#include "gridDynFileInput.h"
#include "readerElement.h"



#include <cstdio>
#include <iterator>
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
#include "gridBus.h"
#include "gridArea.h"
#include "linkModels/gridLink.h"
#include "loadModels/gridLoad.h"
#include "generators/gridDynGenerator.h"
#include "relays/gridRelay.h"

static const std::map < std::string, std::function < gridCoreObject *(std::shared_ptr<readerElement> &, readerInfo *)>> loadFunctionMap
{
  /* *INDENT-OFF* */
	  {"genmodel", [](std::shared_ptr<readerElement> &cd, readerInfo *ri) {return ElementReader(cd, (gridDynGenModel *)(nullptr), "genmodel", ri,nullptr); }},
	  {"exciter", [](std::shared_ptr<readerElement> &cd, readerInfo *ri) {return ElementReader(cd, (gridDynExciter *)(nullptr), "exciter", ri, nullptr); }},
	  {"governor", [](std::shared_ptr<readerElement> &cd, readerInfo *ri) {return ElementReader(cd, (gridDynGovernor *)(nullptr), "governor", ri, nullptr); }},
	  {"pss", [](std::shared_ptr<readerElement> &cd, readerInfo *ri) {return ElementReader(cd, (gridDynPSS *)(nullptr), "pss", ri, nullptr); }},
	  {"source", [](std::shared_ptr<readerElement> &cd, readerInfo *ri) {return ElementReader(cd, (gridSource *)(nullptr), "source", ri, nullptr); }},
	  {"controlblock", [](std::shared_ptr<readerElement> &cd, readerInfo *ri) {return ElementReader(cd, (basicBlock *)(nullptr), "controlblock", ri, nullptr); }},
	  {"generator", [](std::shared_ptr<readerElement> &cd, readerInfo *ri) {return ElementReader(cd, (gridDynGenerator *)(nullptr), "generator", ri, nullptr); }},
	  {"load", [](std::shared_ptr<readerElement> &cd, readerInfo *ri) {return ElementReader(cd, (gridLoad *)(nullptr), "load", ri, nullptr); }},
	  {"bus", [](std::shared_ptr<readerElement> &cd, readerInfo *ri) {return readBusElement(cd, ri, nullptr); }},
	  {"relay", [](std::shared_ptr<readerElement> &cd, readerInfo *ri) {return readRelayElement(cd, ri, nullptr); }},
	  {"area", [](std::shared_ptr<readerElement> &cd, readerInfo *ri) {return readAreaElement(cd, ri, nullptr); }},
	  {"link", [](std::shared_ptr<readerElement> &cd, readerInfo *ri) {return readLinkElement(cd, ri, nullptr, false); }},
	  {"scheduler", [](std::shared_ptr<readerElement> &cd, readerInfo *ri) {return ElementReader(cd, (scheduler *)(nullptr), "scheduler", ri, nullptr); }},
	  { "agc", [](std::shared_ptr<readerElement> &cd, readerInfo *ri) {return ElementReader(cd, (AGControl *)(nullptr), "agc", ri, nullptr); }},
	  { "econ", [](std::shared_ptr<readerElement> &cd, readerInfo *ri) {return readEconElement(cd, ri, nullptr); } },
	  { "reservedispatcher", [](std::shared_ptr<readerElement> &cd, readerInfo *ri) {return ElementReader(cd, (reserveDispatcher *)(nullptr), "reserveDispatcher", ri, nullptr); }},
  /* *INDENT-ON* */
};

void readLibraryElement (std::shared_ptr<readerElement> &element, readerInfo *ri)
{

  auto riScope = ri->newScope ();
  //readerInfo xm2;
  std::string baseName = element->getName ();
  element->bookmark ();

  loadDefines (element, ri);
  loadDirectories (element, ri);
  //loop through the other children
  element->moveToFirstChild ();

  while (element->isValid ())
    {
      gridCoreObject  *obj = nullptr;
      std::string fname = convertToLowerCase (element->getName ());
      //std::cout<<"library model :"<<fname<<":\n";
      if ((fname == "define") || (fname == "recorder") || (fname == "event"))
        {
        }
      else
        {
          auto obname = ri->objectNameTranslate (fname);
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
          bool found = ri->addLibraryObject (obj, pf);
          if (found)
            {
              LEVELPRINT (READER_VERBOSE_PRINT, "adding " << fname << " " << obj->getName () << " to Library");
            }
          else
            {
              WARNPRINT (READER_WARN_IMPORTANT, "Duplicate library objects: ignoring second object " << obj->getName ());
              condDelete (obj,nullptr);
            }

        }
      element->moveToNextSibling ();
    }

  element->restore ();
  assert (element->getName () == baseName);
  ri->closeScope (riScope);
}

static const std::string defineString ("define");

void loadDefines (std::shared_ptr<readerElement> &element, readerInfo *ri)
{
  if (element->hasElement (defineString) == false)
    {
      return;
    }
  std::string def;
  std::string rep;
  bool locked = false;
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
          continue;
        }

      if (element->hasAttribute ("locked"))
        {
          auto lockstr = element->getAttributeText ("locked");
          locked = ((lockstr == "true")||(lockstr == "1")) ? true : false;
        }
      else
        {
          locked = false;
        }
      auto kcheck = ri->checkDefines (rep);
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
          ri->addLockedDefinition (def, rep);
        }
      else
        {
          ri->addDefinition (def, rep);
        }


      element->moveToNextSibling (defineString);                                 // next define
    }
  element->moveToParent ();

}

static const std::string directoryString ("directory");

void loadDirectories (std::shared_ptr<readerElement> &element, readerInfo *ri)
{
  //loop through all directory elements
  if (!element->hasElement (directoryString))
    {
      return;
    }
  element->moveToFirstChild (directoryString);
  while (element->isValid ())
    {
      std::string dfld;
      if (element->hasAttribute ("value"))
        {
          dfld = element->getAttributeText ("value");
        }
      else
        {
          dfld = element->getText ();
        }
      ri->addDirectory (dfld);
      element->moveToNextSibling (directoryString);
    }
  element->moveToParent ();
}

static const std::string customString ("custom");
void loadCustomSections (std::shared_ptr<readerElement> &element, readerInfo *ri)
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
      ri->addCustomElement (name, element, nargs);
      element->moveToNextSibling (directoryString);
    }
  element->moveToParent ();
}

static const std::string translateString ("translate");
void loadTranslations (std::shared_ptr<readerElement> &element, readerInfo *ri)
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

      auto kcheck = ri->objectNameTranslate (component);
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
              ri->addTranslate (def, component);
            }
        }
      else
        {
          if (def.empty ())
            {
              ri->addTranslateType (component, type);
            }
          else if (component.empty ())
            {
              ri->addTranslateType (def, type);
            }
          else
            {
              ri->addTranslate (def, component, type);
            }

        }

      element->moveToNextSibling (translateString);
    }
  element->moveToParent ();
}

