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
#include "readerElement.h"
#include "readerHelper.h"
#include "elementReaderTemplates.hpp"
#include "gridDynFileInput.h"

#include "recorder_events/gridRecorder.h"
#include "objectInterpreter.h"
#include <cstdio>

using namespace readerConfig;

static const IgnoreListType recorderIgnoreStrings {
  "file", "name", "column", "offset", "units", "gain", "bias", "field", "target"
};

static const std::string recorderNameString ("recorder");

int loadRecorderElement (std::shared_ptr<readerElement> &element, gridCoreObject *obj, readerInfo *ri)
{
  int ret = FUNCTION_EXECUTION_SUCCESS;
  std::string name = ri->checkDefines (getElementField (element, "name", defMatchType));
  std::string fname = ri->checkDefines (getElementField (element, "file", defMatchType));

  auto rec = ri->findRecorder (name, fname);
  if (!(rec))
    {
      rec = std::make_shared<gridRecorder> ();
      ri->recorders.push_back (rec);
      if (!name.empty ())
        {
          rec->name = name;
        }
      if (!fname.empty ())
        {
          rec->set ("file",fname);
        }

    }

  gridGrabberInfo gdRI;
  name = getElementField (element, "target", defMatchType);
  if (!name.empty ())
    {
      name = ri->checkDefines (name);
      gdRI.m_target = name;
    }
  stringVec fieldList = getElementFieldMultiple (element, "field", defMatchType);

  if (!(fieldList.empty ()))
    {
      gdRI.field = "";
      for (auto &fstr : fieldList)
        {
          fstr = ri->checkDefines (fstr);
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

  std::string field = getElementField (element, "bias", defMatchType);
  if (!field.empty ())
    {
      gdRI.bias = interpretString (field, ri);
    }
  field = getElementField (element, "gain", defMatchType);
  if (!field.empty ())
    {
      gdRI.gain = interpretString (field, ri);
    }
  field = getElementFieldOptions (element, { "units","unit" }, defMatchType);
  if (!field.empty ())
    {
      field = ri->checkDefines (field);
      gdRI.outputUnits = gridUnits::getUnits (field);
    }
  field = getElementField (element, "column", defMatchType);
  if (!field.empty ())
    {
      gdRI.column = static_cast<int> (interpretString (field, ri));
    }
  field = getElementField (element, "offset", defMatchType);
  if (!field.empty ())
    {
      gdRI.offset = static_cast<int> (interpretString (field, ri));
      if (!(gdRI.field.empty ()))
        {
          WARNPRINT (READER_WARN_ALL, "specifying offset in recorder overrides field specification");
        }
    }
  //now load the other fields for the recorder

  setAttributes (rec, element, recorderNameString, ri, recorderIgnoreStrings);
  setParams (rec, element, recorderNameString, ri, recorderIgnoreStrings);

  if ((gdRI.m_target.empty ())||(gdRI.m_target == obj->getName ()))
    {
      ret = rec->add (&gdRI, obj);
      if (ret == OBJECT_ADD_FAILURE)
        {
          WARNPRINT (READER_WARN_IMPORTANT, "recorder for " << obj->getName () << "cannot find field " << gdRI.field);
        }
      else
        {
          if (rec->name.empty ())
            {
              rec->set ("name", obj->getName () + "_recorder");
            }
        }

    }
  else
    {
      auto oj2 = locateObject (gdRI.m_target, obj);
      if (oj2)
        {
          ret = rec->add (&gdRI, oj2);
          if (ret == OBJECT_ADD_FAILURE)
            {
              WARNPRINT (READER_WARN_IMPORTANT, "recorder for " << gdRI.m_target << "cannot find field " << gdRI.field);
            }
          else
            {
              if (rec->name.empty ())
                {
                  rec->set ("name", oj2->getName () + "_recorder");
                }
            }
        }
      else
        {
          ret = OBJECT_ADD_FAILURE;
        }


    }

  if (ret == OBJECT_ADD_FAILURE)
    {

      WARNPRINT (READER_WARN_IMPORTANT, "recorder for " << gdRI.m_target << "cannot find field " << gdRI.field);
    }

  return ret;
}
