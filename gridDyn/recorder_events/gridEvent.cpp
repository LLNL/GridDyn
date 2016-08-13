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

#include "gridEvent.h"
#include "fileReaders.h"
#include "gridDyn.h"
#include "units.h"
#include "objectInterpreter.h"
#include "stringOps.h"

#include <sstream>

#include <string>

bool compareEvent (const std::shared_ptr<gridEvent> s1, const std::shared_ptr<gridEvent> s2)
{
  return ((s1->nextTriggerTime () < s2->nextTriggerTime ()) ? true : false);
}

gridEvent::gridEvent (double time0,double ResetPeriod)
{
  triggerTime = time0;
  period = ResetPeriod;
}

std::shared_ptr<gridEvent> gridEvent::clone ()
{
  auto nE = std::make_shared<gridEvent> ();
  nE->triggerTime = triggerTime;
  nE->period = period;
  nE->name =  name;
  nE->value = value;
  nE->field = field;
  nE->description = description;
  nE->ts = ts;
  nE->currIndex = currIndex;

  nE->armed = armed;
  nE->m_obj = m_obj;

  nE->unitType = unitType;
  return nE;
}

std::shared_ptr<gridEvent>gridEvent::clone (gridCoreObject *newObj)
{
  auto nE = clone ();
  nE->setTarget (newObj);
  return nE;
}

gridEvent::~gridEvent ()
{
}


void gridEvent::setTime (double time)
{
  triggerTime = time;
}

void gridEvent::setTimeValue (double time, double val)
{
  triggerTime = time;
  value = val;
}

void gridEvent::setTimeValue (const std::vector<double> &time, const std::vector<double> &val)
{
  ts = std::make_shared<timeSeries> ();
  ts->reserve (static_cast<fsize_t> (time.size ()));

  if (ts->addData (time, val))
    {
      triggerTime = time[0];
      value = val[0];
      currIndex = 0;
    }
  else               //the vectors were not of valid lengths
    {

    }

}

void gridEvent::updateTrigger (double time)
{
  if (currIndex != kNullLocation)             //we have a file operation
    {
      currIndex++;
      if (static_cast<size_t> (currIndex) >= ts->count)
        {
          if (period > 0)                     //if we have a period loop the time series
            {
              if (time - ts->time[currIndex] > period)
                {
                  for (size_t kk = 0; kk < ts->count; ++kk)
                    {
                      ts->time[kk] += period + triggerTime;
                    }
                }
              else
                {
                  for (size_t kk = 0; kk < ts->count; ++kk)
                    {
                      ts->time[kk] += period;
                    }
                }

              currIndex = 0;
              triggerTime = ts->time[currIndex];
              value = ts->data[currIndex];
            }
          else
            {
              armed = false;
            }
        }
      else                   //just proceed to the next trigger and Value
        {
          triggerTime = ts->time[currIndex];
          value = ts->data[currIndex];
        }
    }
  else                //no file so loop if there is a period otherwise disarm
    {
      if (period > 0)
        {
          do
            {
              triggerTime = triggerTime + period;
            }
          while (time >= triggerTime);
        }
      else
        {
          armed = false;
        }
    }
}

std::string gridEvent::toString ()
{
  // @time1[,time2,time3,... |+ period] >[rootobj::obj:]field(units) = val1,[val2,val3,...]
  std::stringstream ss;
  if (eFile.empty ())
    {
      ss << '@' << triggerTime;
      if (period > 0)
        {
          ss << '+' << period << '|';
        }
      else if ((ts) && (ts->count > 0))
        {
          for (size_t nn = currIndex + 1; nn < ts->count; ++nn)
            {
              ss << ", " << ts->time[nn];
            }
          ss << "| ";
        }
      else
        {
          ss << " | ";
        }
      ss << fullObjectName (m_obj) << ':' << field;
      if (unitType != gridUnits::defUnit)
        {
          ss << '(' << gridUnits::to_string (unitType) << ')';
        }
      ss << " = " << value;
      if ((ts) && (ts->count > 0))
        {
          for (size_t nn = currIndex + 1; nn < ts->count; ++nn)
            {
              ss << ", " << ts->data[nn];
            }
        }
    }
  else
    {
      ss << fullObjectName (m_obj) << ':' << field;
      if (unitType != gridUnits::defUnit)
        {
          ss << '(' << gridUnits::to_string (unitType) << ')';
        }
      ss << " = <" << eFile;
      if (eColumn > 0)
        {
          ss << '#' << eColumn;
        }
      ss << '>';
    }
  return ss.str ();
}
change_code gridEvent::trigger ()
{
  return (m_obj->set (field, value, unitType)) ? change_code::execution_failure : change_code::parameter_change;
}

change_code gridEvent::trigger (double time)
{
  change_code ret = change_code::not_triggered;
  if (time >= triggerTime)
    {
      if (m_obj->set (field, value, unitType) != PARAMETER_FOUND)
        {
          ret = change_code::execution_failure;
        }
      else
        {
          ret = change_code::parameter_change;
        }
      updateTrigger (time);
    }
  return ret;
}

bool gridEvent::setTarget ( gridCoreObject *gdo,const std::string var)
{
  if (!var.empty ())
    {
      field = var;
    }
  m_obj = gdo;

  if (m_obj)
    {
      name = m_obj->getName ();
      armed = true;
    }
  return armed;
}

void gridEvent::EventFile (const std::string &fname,unsigned int column)
{
  eFile = fname;
  eColumn = column;
  ts = std::make_shared<timeSeries> ();
  int ret = ts->loadBinaryFile (eFile, column);
  if (ret != FILE_LOAD_SUCCESS)
    {
      ts.reset ();
    }
  else
    {
      currIndex = 0;
      triggerTime = ts->time[0];
      value = ts->data[0];
    }
}



std::shared_ptr<gridEvent> make_event (const std::string &eventString, gridCoreObject *rootObject)
{

  // @time1[,time2,time3,... + period] |[rootobj::obj:]field(units) const = val1,[val2,val3,...]  or
  // [rootobj::obj:]field(units) = val1,[val2,val3,...] @time1[,time2,time3,...|+ period] or
  std::string objString;

  auto ev = std::make_shared<gridEvent> ();
  std::vector<double> times;
  std::vector<double> vals;
  auto posA = eventString.find_first_of ("@");
  if (posA == std::string::npos)
    {
      objString = eventString;
    }
  else
    {
      auto posT = eventString.find_first_of ("|", posA + 2);
      std::string tstring = (posT != std::string::npos) ? eventString.substr (posA + 1, posT - posA - 1) : eventString.substr (posA + 1, std::string::npos);
      trimString (tstring);
      auto cstr = tstring.find_first_of (",");
      if (cstr == std::string::npos)
        {
          cstr = tstring.find_first_of ("+");
          if (cstr == std::string::npos)
            {

              times.push_back (std::stod (tstring));
            }
          else
            {
              times.push_back (std::stod (tstring.substr (0, cstr - 1)));
              ev->period = std::stod (tstring.substr (cstr + 1, std::string::npos));
            }

        }
      else
        {
          times = str2vector (tstring,-1,",");
        }
      if (posA > 2)
        {
          objString = eventString.substr (0, posA - 1);
        }
      else
        {
          objString = eventString.substr (posT + 1, std::string::npos);
        }
    }


  trimString (objString);
  auto posE = objString.find_first_of ('=');
  std::string vstring = objString.substr (posE + 1, std::string::npos);
  trimString (vstring);
  objString = objString.substr (0, posE);
  //break down the object specification
  objInfo fdata (objString, rootObject);

  ev->setTarget (fdata.m_obj);
  ev->unitType = fdata.m_unitType;
  ev->field = fdata.m_field;

  posE = vstring.find_first_of ('<');
  if (posE != std::string::npos)
    { //now we get into file based event
      auto fstr = vstring.substr (posE + 1);
      fstr.pop_back ();     //get rid of the tailing ">"
      int col = trailingStringInt (fstr, fstr, 0);
      ev->EventFile (fstr, col);
    }
  else
    {


      auto cstr = vstring.find_first_of (',');
      if (cstr == std::string::npos)
        {
          vals.push_back (std::stod (vstring));
        }
      else
        {
          vals = str2vector (vstring, -1, ",");
        }



      if (times.size () > 1)
        {
          if (vals.size () == times.size ())
            {
              ev->setTimeValue (times, vals);
            }
          else
            {
              for (auto tv : times)
                {
                  ev->ts->addData (tv, vals[0]);
                }
              ev->triggerTime = times[0];
              ev->value = vals[0];
            }
        }
      else
        {
          ev->setTimeValue (times[0], vals[0]);
        }
    }
  return ev;
}


std::shared_ptr<gridEvent> make_event (const std::string &field, double val, double eventTime, gridCoreObject *rootObject)
{
  auto ev = std::make_shared<gridEvent> (eventTime);
  objInfo fdata (field, rootObject);
  ev->setTarget (fdata.m_obj);
  ev->unitType = fdata.m_unitType;
  ev->field = fdata.m_field;
  ev->value = val;
  return ev;
}


std::shared_ptr<gridEvent> make_event (gridEventInfo *gdEI, gridCoreObject *rootObject)
{

  auto ev = gdEI->eString.empty () ? (std::make_shared<gridEvent> ()) : make_event (gdEI->eString,rootObject);

  ev->period = gdEI->period;

  ev->description = gdEI->description;
  objInfo fdata;
  gridCoreObject *obj = nullptr;
  if (!(gdEI->name.empty ()))
    {
      size_t rlc = gdEI->name.find_last_of (':');
      if (rlc != std::string::npos)
        {
          if (gdEI->name[rlc - 1] == ':')
            {
              obj = locateObject (gdEI->name, rootObject);
            }
          else
            {
              fdata.LoadInfo (gdEI->name, rootObject);
              obj = fdata.m_obj;
            }
        }
      else
        {
          obj = locateObject (gdEI->name, rootObject);
        }

    }

  if (!(gdEI->field.empty ()))
    {
      fdata.LoadInfo (gdEI->field, (obj) ? (obj) : rootObject);
    }

  if (fdata.m_obj)
    {
      ev->setTarget (fdata.m_obj);
      ev->name = fdata.m_obj->getName ();
    }
  if (!fdata.m_field.empty ())
    {
      ev->field = fdata.m_field;
    }
  if (fdata.m_unitType != gridUnits::defUnit)
    {
      ev->unitType = fdata.m_unitType;
    }



  if (gdEI->time.size () > 1)
    {
      ev->setTimeValue (gdEI->time, gdEI->value);
    }
  else if (gdEI->time.size () == 1)
    {
      if (gdEI->value.size () > 1)
        {
          ev->triggerTime = gdEI->time[0];
          double ttime = ev->triggerTime;
          ev->ts = std::make_shared<timeSeries> ();
          ev->ts->reserve (static_cast<fsize_t> (gdEI->value.size ()));
          for (auto v : gdEI->value)
            {
              ev->ts->addData (ttime, v);
              ttime = ttime + ev->period;
            }
          ev->currIndex = 0;
        }
      else if (!(gdEI->value.empty ()))
        {
          ev->ts.reset ();
          ev->triggerTime = gdEI->time[0];
          ev->value = gdEI->value[0];
        }

    }
  else if (!(gdEI->value.empty ()))
    {
      ev->triggerTime = 0.0;
      ev->value = gdEI->value[0];
    }
  if (gdEI->unitType != gridUnits::defUnit)
    {
      ev->unitType = gdEI->unitType;
    }
  makeLowerCase (ev->field);
  if (gdEI->file.length () > 3)
    {
      ev->EventFile (gdEI->file, gdEI->column);
    }

  return ev;
}
