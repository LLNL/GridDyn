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

#include "gridDyn.h"
#include "readerHelper.h"
#include "elementReaderTemplates.hpp"

using namespace readerConfig;


void loadDefaultObjectTranslations (readerInfo &ri);

static const IgnoreListType simIgnoreFields {
  "version", "basepower"
};

bool isMasterObject (const coreObject *searchObject, const gridSimulation *gs);

static const std::string libstring("library");
// read XML file
//coreObject * readSimXMLFile(const std::string &filename, coreObject *gco, const std::string  prefix, readerInfo *ri) const
gridSimulation * readSimulationElement (std::shared_ptr<readerElement> &element, readerInfo &ri, coreObject *searchObject, gridSimulation *gs)
{
  // pointers
  bool masterObject = isMasterObject (searchObject, gs);

  auto riScope = ri.newScope ();

  loadDefines (element, ri);
  loadDirectories (element, ri);
  if (masterObject)
    {
      loadDefaultObjectTranslations (ri);
    }
  loadTranslations (element, ri);
  loadCustomSections (element, ri);
  gridSimulation *simulation = ElementReaderSetup (element, gs, "simulation", ri, searchObject);

  //
  
  //load the simulation name and id
  std::string ename = getElementField(element, "name", defMatchType);
  if (!ename.empty())
  {
	  simulation->setName(ename);
  }
  setIndex(element, simulation, ri);
  // load any other attributes
  objSetAttributes (simulation, element, simulation->getName (), ri, simIgnoreFields);

  if (masterObject)
    {
      std::string pname = getElementField (element, "basepower", defMatchType);
      if (!pname.empty ())
        {
          double val = interpretString (pname, ri);
          ri.base = val;
          simulation->set ("basepower", val);
        }
    }
  //load the libraries
  if (element->hasElement (libstring))
    {
      element->moveToFirstChild (libstring);
      while (element->isValid ())
        {
          readLibraryElement (element, ri);
          element->moveToNextSibling (libstring);
        }
      element->moveToParent ();
    }


  readImports (element, ri, simulation, false);


  //load all other objects besides bus and area
  loadSubObjects (element, ri, simulation);


  paramLoopElement (simulation,element, simulation->getName (), ri, simIgnoreFields);

  //read imports marked final
  readImports (element, ri, simulation, true);

  element->moveToFirstChild ("solver");
  while (element->isValid ())
    {
      loadSolverElement (element, ri, dynamic_cast<gridDynSimulation*> (simulation));
      element->moveToNextSibling ("solver");
    }
  element->moveToParent ();

  if (masterObject)
    {
      int busCount = simulation->getInt ("totalbuscount");
      int linkCount = simulation->getInt ("totallinkcount");

      LEVELPRINT (READER_NORMAL_PRINT, "loaded Power simulation " << simulation->getName ());
      LEVELPRINT (READER_SUMMARY_PRINT, "Summary: " << busCount << " buses Loaded ");
      LEVELPRINT (READER_SUMMARY_PRINT, "Summary: " << linkCount << " links Loaded ");
      if (ri.collectors.size () > 0)
        {
          LEVELPRINT (READER_SUMMARY_PRINT, "Summary: " << ri.collectors.size () << " collectors Loaded ");
        }
      if (ri.events.size () > 0)
        {
          LEVELPRINT (READER_SUMMARY_PRINT, "Summary: " << ri.events.size () << " events Loaded ");
        }
      for (auto col : ri.collectors)
        {
          simulation->add (col);
        }
      //add the events
      simulation->add (ri.events);
    }

  ri.closeScope (riScope);

  return simulation;
}

void loadDefaultObjectTranslations (readerInfo &ri)
{
  ri.addTranslate ("fuse", "relay");
  ri.addTranslate ("breaker", "relay");
  ri.addTranslate ("sensor", "relay");
  ri.addTranslate ("control", "relay");
  ri.addTranslate ("block", "controlblock");
  ri.addTranslate ("model", "genmodel");
  ri.addTranslate ("gen", "generator");
  ri.addTranslate ("transformer", "link");
  ri.addTranslate ("line", "link");
  ri.addTranslate ("tie", "link");
  ri.addTranslate ("subsystem", "link");
  ri.addTranslate ("busmodify", "bus");
  ri.addTranslate ("areamodify", "area");
  ri.addTranslate ("linkmodify", "link");
  ri.addTranslate ("gov", "governor");
  ri.addTranslate("recorder", "collector");
  ri.addTranslate("player", "event");
}


bool isMasterObject (const coreObject *searchObject, const gridSimulation *gs)
{
  if (searchObject)
    {
      if (gs)
        {
		  return(isSameObject(searchObject,gs));
        }
      return false;
    }
  return (gs) ? (gs->isRoot()) : true;  //The last true could be either if both are null
   
}
