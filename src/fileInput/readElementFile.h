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

#ifndef READ_ELEMENT_FILE_H_
#define READ_ELEMENT_FILE_H_
#pragma once

#include "readElement.h"
#include "formatInterpreters/readerElement.h"
#include "core/coreObject.h"
#include "simulation/gridSimulation.h"
#include "boost/filesystem.hpp"

#include <type_traits>
#include <memory>

namespace griddyn
{
void readConfigurationFields (std::shared_ptr<readerElement> &sim, readerInfo &ri);

template<class RX>
coreObject * loadElementFile (coreObject *parentObject, const std::string &fileName, readerInfo *ri)
{
  using namespace readerConfig;
  static_assert (std::is_base_of<readerElement, RX>::value, "classes must be inherited from coreObject");
  // pointers

  coreObject *gco = nullptr;
  bool rootSimFile = true;
  if (parentObject != nullptr)
    {
      auto rootObj = parentObject->getRoot();
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

  
  //May need to create a readerInfo object this to ensure it gets deleted even under exception
  std::unique_ptr<readerInfo> rip = (ri == nullptr) ? std::make_unique<readerInfo>() : nullptr;
  readerInfo::scopeID riScope=0;
  if (ri==nullptr)
    {
      ri = rip.get(); //we created the unique_ptr now
     
    }
  else
  {
	  riScope = ri->newScope();
  }


  boost::filesystem::path mainPath (fileName);

  ri->addDirectory (boost::filesystem::current_path ().string ());
  ri->addDirectory (mainPath.parent_path ().string ());


  // read xml file from location
  LEVELPRINT (READER_SUMMARY_PRINT, "loading file " << fileName);

  auto doc = std::make_shared<RX> (fileName);

  if (!doc->isValid ())
    {
      WARNPRINT (READER_WARN_ALL, "Unable to open File" << fileName);
      if (dynamic_cast<gridSimulation *> (parentObject))
        {
          dynamic_cast<gridSimulation *> (parentObject)->setErrorCode (GS_INVALID_FILE_ERROR);
        }
      return nullptr;
    }
  auto sim = std::static_pointer_cast<readerElement> (doc);
  if (rootSimFile)
    {
      readConfigurationFields (sim, *ri);
    }
  gco = (rootSimFile) ? readSimulationElement (sim, *ri, nullptr, static_cast<gridSimulation *> (gco)) : parentObject;
  std::string name = sim->getName ();
  if (!rootSimFile)
    {
      loadElementInformation (gco, sim, "import", *ri, { "version" });
    }
  if (!rip)
    {//scope closing can be expensive so don't do it unless we need to
	  ri->closeScope(riScope);
    }

  if (rootSimFile)
    {
      gco->set ("sourcefile", fileName);
    }

  return gco;
}

}//namespace griddyn

#endif