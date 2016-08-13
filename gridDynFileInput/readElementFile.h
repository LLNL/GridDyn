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

#ifndef READ_ELEMENT_FILE_H_
#define READ_ELEMENT_FILE_H_

#include "readElement.h"
#include "readerElement.h"
#include "gridCore.h"
#include "simulation/gridSimulation.h"
#include "boost/filesystem.hpp"

#include <type_traits>
#include <memory>
void readConfigurationFields (std::shared_ptr<readerElement> &sim, readerInfo *ri);

template<class RX>
gridCoreObject * loadElementFile (gridCoreObject *parentObject, const std::string &filename, readerInfo *ri)
{
  using namespace readerConfig;
  static_assert (std::is_base_of<readerElement, RX>::value, "classes must be inherited from gridCoreObject");
  // pointers

  gridCoreObject *gco = nullptr;
  bool rootSimFile = true;
  if (parentObject != nullptr)
    {
      auto rootObj = parentObject->find ("root");
      if (rootObj->getID () == parentObject->getID ())
        {
          rootSimFile = true;
          gco = parentObject;

        }
      else
        {
          rootSimFile = false;
        }
    }
  else
    {
      //set the warn count to 0 for the readerConfig namespace
      warnCount = 0;
    }

  bool rmxmi = false;
  readerInfo::scopeID riScope = 0;
  if (ri == nullptr)
    {
      ri = new readerInfo;
      rmxmi = true;
    }
  else
    {
      riScope = ri->newScope ();
    }

  boost::filesystem::path mainPath (filename);

  ri->addDirectory (boost::filesystem::current_path ().string ());
  ri->addDirectory (mainPath.parent_path ().string ());


  // read xml file from location
  LEVELPRINT (READER_SUMMARY_PRINT, "loading file " << filename);

  auto doc = std::make_shared<RX> (filename);

  if (!doc->isValid ())
    {
      WARNPRINT (READER_WARN_ALL, "Unable to open File" << filename);
      if (dynamic_cast<gridSimulation *> (parentObject))
        {
          dynamic_cast<gridSimulation *> (parentObject)->setErrorCode (GS_INVALID_FILE_ERROR);
        }
      return nullptr;
    }
  auto sim = std::static_pointer_cast<readerElement> (doc);
  if (rootSimFile)
    {
      readConfigurationFields (sim, ri);
    }
  gco = (rootSimFile) ? readSimulationElement (sim, ri, nullptr, static_cast<gridSimulation *> (gco)) : parentObject;
  std::string name = sim->getName ();
  if (!rootSimFile)
    {
      loadElementInformation (gco, sim, "import", ri, { "version" });
    }
  if (rmxmi)
    {
      delete ri;
    }
  else
    {
      ri->closeScope (riScope);
    }

  if (rootSimFile)
    {
      gco->set ("sourcefile", filename);
    }

  return gco;
}



#endif