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
#include "readerElement.h"
#include "elementReaderTemplates.hpp"

#include "recorder_events/gridEvent.h"
#include "units.h"
#include "stringConversion.h"
#include <iostream>

using namespace readerConfig;

static const std::string eventNameString("event");

static const IgnoreListType eventIgnoreStrings{
	"file","name", "column","value","va", "units","description","period",
	"time","t","offset", "units", "gain", "bias", "field", "target", "type"
};

void readEventElement (std::shared_ptr<readerElement> &aP, gridEventInfo &gdEI, readerInfo *ri,gridCoreObject *obj)
{

  //get the event strings that may be present
auto eventString = aP->getMultiText (", ");
	if (!eventString.empty())
	{
		gdEI.loadString(eventString, obj);
	}

	//check for the field attributes
	std::string name = getElementField(aP, "name", defMatchType);
	if (!name.empty())
	{
		name = ri->checkDefines(name);
		gdEI.name = name;
	}

	//check for the field attributes
	std::string type = getElementField(aP, "type", defMatchType);
	if (!type.empty())
	{
		gdEI.type = ri->checkDefines(type);
	}

  //check for the field attributes
  auto targetList = getElementFieldMultiple (aP, "target", defMatchType);
  for (auto &ss:targetList)
    {
      ss = ri->checkDefines (ss);
	  gdEI.targetObjs.push_back(locateObject(ss, obj));
    }

  auto fieldList = getElementFieldMultiple (aP, "field", defMatchType);
  for (auto &ss : fieldList)
  {
	  ss = ri->checkDefines(ss);
	  gdEI.fieldList.push_back(ss);
  }
  auto unitList = getElementFieldMultiple(aP, "units", defMatchType);
  for (auto &ss : unitList)
  {
	  ss = ri->checkDefines(ss);
	  gdEI.units.push_back(gridUnits::getUnits(ss));
  }
 
  gdEI.description = getElementField (aP, "description", defMatchType);

  std::string field = getElementField (aP, "period", defMatchType);
  if (!field.empty ())
    {
      gdEI.period = interpretString (field,ri);
    }

  field = getElementFieldOptions (aP, { "t","time" }, defMatchType);
  if (!field.empty ())
    {
      gdEI.time = str2vector<gridDyn_time> (field,negTime);
    }

  field = getElementFieldOptions (aP, { "value","val" }, defMatchType);
  if (!field.empty ())
    {
      gdEI.value = str2vector (field, kNullVal);
    }

  name = getElementField (aP, "file", defMatchType);
  if (!name.empty ())
    {
      ri->checkFileParam (name);
      gdEI.file = name;
    }

  field = getElementField (aP, "column", defMatchType);
  if (!field.empty ())
    {
      gdEI.columns.push_back(static_cast<int> (interpretString (field, ri)));
    }


}


int loadEventElement (std::shared_ptr<readerElement> &element, gridCoreObject *obj, readerInfo *ri)
{


  int ret = FUNCTION_EXECUTION_SUCCESS;
  element->bookmark ();
  gridEventInfo gdEI;
  readEventElement (element,gdEI,ri,obj);
  std::shared_ptr<gridEvent> gdE = make_event (&gdEI, obj);

  setAttributes(gdE, element, eventNameString, ri, eventIgnoreStrings);
  setParams(gdE, element, eventNameString, ri, eventIgnoreStrings);

  if ((gdE)&&(!(gdE->isArmed ())))
    {
      WARNPRINT (READER_WARN_IMPORTANT, "event for " << gdEI.name << ":unable to load event");
      ret = FUNCTION_EXECUTION_FAILURE;
    }
  else
    {
      ri->events.push_back (gdE);
    }
  element->restore ();
  return ret;
}
