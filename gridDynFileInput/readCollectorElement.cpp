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
#include "core/factoryTemplates.h"

#include "measurement/collector.h"
#include "core/objectInterpreter.h"


using namespace readerConfig;

static const std::string nameString("name");

static const IgnoreListType collectorIgnoreStrings {
   "file","name", "column", "offset", "units", "gain", "bias", "field", "target", "type"
};

static const std::string collectorNameString ("collector");

int loadCollectorElement (std::shared_ptr<readerElement> &element, coreObject *obj, readerInfo &ri)
{
  int ret = FUNCTION_EXECUTION_SUCCESS;
  std::string name = ri.checkDefines (getElementField (element, nameString, defMatchType));
  std::string fname = ri.checkDefines(getElementFieldOptions(element, { "file","sink" }, defMatchType));
  std::string type = ri.checkDefines(getElementField(element, "type", defMatchType));

  auto col = ri.findCollector(name, fname);
  if ((!type.empty())&&(name.empty())&&(fname.empty()))
  {
	  col = nullptr;
  }
  if (type.empty())
  {
	  if (element->getName() != collectorNameString)
	  {
		  type = element->getName();
	  }
  }
  
  if (!(col))
    {
	  col = makeCollector(type, name);
     
      if (!fname.empty ())
        {
          col->set ("file",fname);
        }
	  ri.collectors.push_back(col);
    }

  gridGrabberInfo gdRI;
  name = getElementField (element, "target", defMatchType);
  if (!name.empty ())
    {
      name = ri.checkDefines (name);
      gdRI.m_target = name;
    }
  auto fieldList = getElementFieldMultiple (element, "field", defMatchType);

  if (!(fieldList.empty ()))
    {
      gdRI.field = "";
      for (auto &fstr : fieldList)
        {
          fstr = ri.checkDefines (fstr);
          if (gdRI.field.empty ())
            {
              gdRI.field = fstr;
            }
          else
            {
              gdRI.field += "; " + fstr;
            }

        }
    }

  std::string elementText = getElementField (element, "bias", defMatchType);
  if (!elementText.empty ())
    {
      gdRI.bias = interpretString (elementText, ri);
    }
  elementText = getElementField (element, "gain", defMatchType);
  if (!elementText.empty ())
    {
      gdRI.gain = interpretString (elementText, ri);
    }
  elementText = getElementFieldOptions (element, { "units","unit" }, defMatchType);
  if (!elementText.empty ())
    {
	  elementText = ri.checkDefines (elementText);
      gdRI.outputUnits = gridUnits::getUnits (elementText);
    }
  elementText = getElementField (element, "column", defMatchType);
  if (!elementText.empty ())
    {
      gdRI.column = static_cast<int> (interpretString (elementText, ri));
    }
  elementText = getElementField (element, "offset", defMatchType);
  if (!elementText.empty ())
    {
      gdRI.offset = static_cast<int> (interpretString (elementText, ri));
      if (!(gdRI.field.empty ()))
        {
          WARNPRINT (READER_WARN_ALL, "specifying offset in recorder overrides field specification");
        }
    }
  //now load the other fields for the recorder

  setAttributes (col.get(), element, collectorNameString, ri, collectorIgnoreStrings);
  setParams (col.get(), element, collectorNameString, ri, collectorIgnoreStrings);
  coreObject *targetObj = obj;

  if (!((gdRI.m_target.empty()) || (gdRI.m_target == obj->getName())))
  {
	  targetObj=locateObject(gdRI.m_target, obj);
  }
    if (targetObj)
	{
	  try
	  {
		  col->add(&gdRI, targetObj);
		  if (col->getName().empty())
		  {
			  col->setName(targetObj->getName() + "_"+type);
		  }
	  }
	  catch (const addFailureException &)
	  {
		  WARNPRINT(READER_WARN_IMPORTANT, type<<" for " << obj->getName() << "cannot find field " << gdRI.field);
	  }
     

    }
  else
    {
	  WARNPRINT(READER_WARN_IMPORTANT, type<<" for " << gdRI.m_target << "cannot find field " << gdRI.field);
    }


  if (col->getWarningCount() > 0)
  {
	  for (const auto &warning : col->getWarnings())
	  {
		  WARNPRINT(READER_WARN_IMPORTANT, "recorder "<<col->getName()<<" "<<warning );
	  }
  }
  return ret;
}
