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
#include "readerElement.h"

#include "recorder_events/gridEvent.h"
#include "units.h"
#include "stringOps.h"
#include <cstdio>
#include <iostream>

using namespace readerConfig;

void readEventElement (std::shared_ptr<readerElement> &aP, gridEventInfo &gdEI, readerInfo *ri)
{

  //get the event strings that may be present
  gdEI.eString = aP->getMultiText (", ");

  //check for the field attributes
  std::string name = getElementField (aP, "target", defMatchType);
  if (!name.empty ())
    {
      name = ri->checkDefines (name);
      gdEI.name = name;
    }

  std::string field = getElementField (aP, "field", defMatchType);
  if (!field.empty ())
    {
      field = ri->checkDefines (field);
      gdEI.field = field;
    }
  field = getElementField (aP, "units", defMatchType);
  if (!field.empty ())
    {
      field = ri->checkDefines (field);
      gdEI.unitType = gridUnits::getUnits (field);
    }

  gdEI.description = getElementField (aP, "description", defMatchType);

  field = getElementField (aP, "period", defMatchType);
  if (!field.empty ())
    {
      gdEI.period = interpretString (field,ri);
    }

  field = getElementFieldOptions (aP, { "t","time" }, defMatchType);
  if (!field.empty ())
    {
      gdEI.time = str2vector (field,kNullVal);
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
      gdEI.column = static_cast<int> (interpretString (field, ri));
    }


}


int loadEventElement (std::shared_ptr<readerElement> &element, gridCoreObject *obj, readerInfo *ri)
{


  int ret = FUNCTION_EXECUTION_SUCCESS;
  element->bookmark ();
  gridEventInfo gdEI;
  readEventElement (element,gdEI,ri);
  std::shared_ptr<gridEvent> gdE = make_event (&gdEI, obj);

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
