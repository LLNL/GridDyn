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

#include "gridDynFileInput.h"
#include "readElement.h"
#include "utilities/stringOps.h"
#include "formatInterpreters/readerElement.h"
#include "gridObjects.h"
#include "core/objectInterpreter.h"
#include "readerHelper.h"


using namespace readerConfig;

coreObject* getParent (std::shared_ptr<readerElement> &element, readerInfo &ri, coreObject *parentObject,const std::string &alternateName)
{
  std::string parentName = getElementField (element, "parent", defMatchType);
  if (!parentName.empty ())
    {
      parentName = ri.checkDefines (parentName);
      return locateObject (parentName, parentObject);
    }
  else if (!alternateName.empty ())
    {
      parentName = getElementAttribute (element, alternateName, defMatchType);
      if (!parentName.empty ())
        {
          parentName = ri.checkDefines (parentName);
          return locateObject (parentName, parentObject);
        }
    }
  return nullptr;
}