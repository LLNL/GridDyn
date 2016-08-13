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

#include "simulation/gridSimulation.h"
#include "gridArea.h"
#include "linkModels/gridLink.h"
#include "gridBus.h"
#include "gridEvent.h"
#include "gridRecorder.h"
#include "relays/gridRelay.h"
#include "eventQueue.h"
#include "loadModels/gridLoad.h"
#include "generators/gridDynGenerator.h"
#include "stringOps.h"
#include "gridCoreList.h"

#include <map>
#include <utility>

#include <cstdio>
#include <iostream>

gridSimulation::gridSimulation (const std::string &objName) : gridArea (objName)
{
  EvQ = std::make_shared<eventQueue> ();
}

gridSimulation::~gridSimulation ()
{
  opFlags.set (being_deleted);       //set this flag to handle some unusual circumstances with extra objects
}

gridCoreObject *gridSimulation::clone (gridCoreObject *obj) const
{
  gridSimulation *sim;
  if (obj == nullptr)
    {
      sim = new gridSimulation ();
    }
  else
    {
      sim = dynamic_cast<gridSimulation *> (obj);
      if (sim == nullptr)
        {
          gridPrimary::clone (obj);
          return obj;
        }
    }
  gridArea::clone (sim);
  sim->stopTime = stopTime;
  sim->timeCurr = timeCurr;
  sim->startTime = startTime;
  sim->stepTime = stepTime;
  sim->recordStart = recordStart;
  sim->recordStop = recordStop;
  sim->nextRecordTime = nextRecordTime;
  sim->alertCount = alertCount;
  sim->pState = pState;
  sim->state_record_period = state_record_period;
  sim->consolePrintLevel = consolePrintLevel;
  sim->errorCode = errorCode;


  sim->sourceFile = sourceFile;                                                                                 //main source file  name
  sim->minUpdateTime = minUpdateTime;                                                                           //minimum time period to go between updates; for the hybrid simultaneous partitioned solution
  sim->maxUpdateTime = maxUpdateTime;                                                                           //(s) max time period to go between updates
  sim->absTime = absTime;                                                                                       // [s] seconds in unix time;

  return sim;
}


void gridSimulation::setErrorCode (int ecode)
{
  pState = ((ecode == GS_NO_ERROR) ? pState : gridState_t::GD_ERROR), errorCode = ecode;
}

int gridSimulation::addsp (std::shared_ptr<gridCoreObject> obj)
{
  gridCoreObject *gco = obj.get ();
  obj->setOwner (nullptr, gco);       //set an ownership loop so the object would never get deleted in another way
  auto ret = gridArea::add (gco);       //add the object to the regular system
  extraObjects.push_back (obj);
  if (ret == OBJECT_NOT_RECOGNIZED)        //catch for extraObjects
    {
      obj->locIndex = static_cast<index_t> (extraObjects.size ()) - 1;
      obj->setParent (this);
      obList->insert (gco);
      if (obj->getNextUpdateTime () < kHalfBigNum)               //check if the object has updates
        {
          EvQ->insert (gco);
        }
      ret = OBJECT_ADD_SUCCESS;
    }
  return ret;
}


int gridSimulation::add (std::shared_ptr<gridRecorder> rec)
{
  ++recordCount;
  recordList.push_back (rec);
  if (!recordDirectory.empty ())
    {
      rec->set ("directory", recordDirectory);
    }
  EvQ->insert (rec);
  return 0;
}

int gridSimulation::add (std::shared_ptr<gridEvent> evnt)
{
  ++eventCount;
  EvQ->insert (evnt);
  return 0;
}

int gridSimulation::add (std::shared_ptr<eventAdapter> eA)
{
  ++eventCount;
  EvQ->insert (eA);
  return 0;
}

int gridSimulation::add (std::list<std::shared_ptr<gridEvent> > elist)
{
  for (auto &ev : elist)
    {
      ++eventCount;
      EvQ->insert (ev);

    }
  return 0;
}


int gridSimulation::run (double /*finishTime*/)
{
  return FUNCTION_EXECUTION_FAILURE;
}

int gridSimulation::step ()
{
  return FUNCTION_EXECUTION_FAILURE;
}

void gridSimulation::saveRecorders ()
{
  int ret;
  //save the recorder files
  for (auto gr : recordList)
    {
      ret = gr->saveFile ();
      if (ret == FILE_NOT_FOUND)
        {
          LOG_ERROR ("unable to open file for writing " + gr->getFileName ());
        }
      else
        {
          LOG_SUMMARY ("saving recorder output:" + gr->getFileName ());
        }
    }
}

int gridSimulation::set (const std::string &param,  const std::string &val)
{
  int out = PARAMETER_FOUND;
  std::string temp;
  if (param == "recorddirectory")
    {
      recordDirectory = val;
      for (auto gr : recordList)
        {
          gr->set ("directory", recordDirectory);
        }
    }
  else if (param == "logprintlevel")
    {
      temp = convertToLowerCase (val);

      if (temp == "trace")
        {
          logPrintLevel = GD_TRACE_PRINT;
        }
      else if (temp == "debug")
        {
          logPrintLevel = GD_DEBUG_PRINT;
        }
      else if (temp == "normal")
        {
          logPrintLevel = GD_NORMAL_PRINT;
        }
      else if (temp == "summary")
        {
          logPrintLevel = GD_SUMMARY_PRINT;
        }
      else if (temp == "warning")
        {
          logPrintLevel = GD_WARNING_PRINT;
        }
      else if (temp == "error")
        {
          logPrintLevel = GD_ERROR_PRINT;
        }
      else if (temp == "none")
        {
          logPrintLevel = GD_NO_PRINT;
        }
      else
        {
          out = INVALID_PARAMETER_VALUE;
        }

    }
  else if (param == "consoleprintlevel")
    {
      temp = convertToLowerCase (val);
      if (temp == "trace")
        {
          consolePrintLevel = GD_TRACE_PRINT;
        }
      else if (temp == "debug")
        {
          consolePrintLevel = GD_DEBUG_PRINT;
        }
      else if (temp == "normal")
        {
          consolePrintLevel = GD_NORMAL_PRINT;
        }
      else if (temp == "summary")
        {
          consolePrintLevel = GD_SUMMARY_PRINT;
        }
      else if (temp == "warning")
        {
          consolePrintLevel = GD_WARNING_PRINT;
        }
      else if (temp == "error")
        {
          consolePrintLevel = GD_ERROR_PRINT;
        }
      else if (temp == "none")
        {
          consolePrintLevel = GD_NO_PRINT;
        }
      else
        {
          out = INVALID_PARAMETER_VALUE;
        }
    }
  else if (param == "printlevel")
    {
      temp = convertToLowerCase (val);
      if (temp == "trace")
        {
          consolePrintLevel = GD_TRACE_PRINT;
          logPrintLevel = GD_TRACE_PRINT;
        }
      else if (temp == "debug")
        {
          consolePrintLevel = GD_DEBUG_PRINT;
          logPrintLevel = GD_DEBUG_PRINT;
        }
      else if (temp == "normal")
        {
          consolePrintLevel = GD_NORMAL_PRINT;
          logPrintLevel = GD_NORMAL_PRINT;
        }
      else if (temp == "summary")
        {
          consolePrintLevel = GD_SUMMARY_PRINT;
          logPrintLevel = GD_SUMMARY_PRINT;
        }
      else if (temp == "warning")
        {
          consolePrintLevel = GD_WARNING_PRINT;
          logPrintLevel = GD_WARNING_PRINT;
        }
      else if (temp == "error")
        {
          consolePrintLevel = GD_ERROR_PRINT;
          logPrintLevel = GD_ERROR_PRINT;
        }
      else if (temp == "none")
        {
          consolePrintLevel = GD_NO_PRINT;
          logPrintLevel = GD_NO_PRINT;
        }
      else
        {
          out = INVALID_PARAMETER_VALUE;
        }
    }
  else if (param == "logfile")
    {
      logFile = val;
      if (logFileStream.is_open ())
        {
          logFileStream.close ();
        }
      logFileStream.open (val.c_str (), std::ios::out | std::ios::trunc);
    }
  else if (param == "statefile")
    {
      stateFile = val;
    }
  else if (param == "sourcefile")
    {
      sourceFile = val;
    }
  else
    {
      out = gridArea::set (param, val);
    }
  return out;
}

std::string gridSimulation::getString (const std::string &param) const
{
  if (param == "logfile")
    {
      return logFile;
    }
  else if (param == "statefile")
    {
      return stateFile;
    }
  else if (param == "sourcefile")
    {
      return sourceFile;
    }
  else
    {
      return gridArea::getString (param);
    }
}

int gridSimulation::set (const std::string &param, double val, gridUnits::units_t unitType)
{
  int out = PARAMETER_FOUND;
  if ((param == "timestart") || (param == "start")||(param == "starttime"))
    {
      startTime = gridUnits::unitConversionTime (val, unitType, gridUnits::sec);
    }
  else if ((param == "abstime") || (param == "walltime"))
    {
      absTime = val;
    }
  else if ((param == "stoptime")|| (param == "stop") || (param == "timestop"))
    {
      stopTime = gridUnits::unitConversionTime (val, unitType, gridUnits::sec);
    }
  else if (param == "consoleprintlevel")
    {
      consolePrintLevel = static_cast<int> (val);
    }
  else if (param == "logprintlevel")
    {
      logPrintLevel = static_cast<int> (val);
    }
  else if ((param == "steptime") || (param == "step") || (param == "timestep"))
    {
      stepTime = gridUnits::unitConversionTime (val, unitType, gridUnits::sec);
    }
  else if ((param == "minupdatetime"))
    {
      minUpdateTime = gridUnits::unitConversionTime (val, unitType, gridUnits::sec);
    }
  else if (param == "maxupdatetime")
    {
      maxUpdateTime = gridUnits::unitConversionTime (val, unitType, gridUnits::sec);
    }
  else if (param == "staterecordperiod")
    {
      state_record_period = gridUnits::unitConversionTime (val, unitType, gridUnits::sec);
    }
  else if (param == "recordstop")
    {
      recordStop = gridUnits::unitConversionTime (val, unitType, gridUnits::sec);
    }
  else if (param == "recordstart")
    {
      recordStart = gridUnits::unitConversionTime (val, unitType, gridUnits::sec);
    }
  else
    {
      out = gridArea::set (param, val, unitType);
      if (out == PARAMETER_NOT_FOUND)
        {
          if (m_Areas.size () == 1)
            {
              out = m_Areas[0]->set (param, val, unitType);
            }
        }
    }
  return out;


}


// find gridRecorder
std::shared_ptr<gridRecorder> gridSimulation::findRecorder (std::string recordname)
{
  std::shared_ptr<gridRecorder> rec;
  std::string fname;
  for (auto &gr : recordList)
    {
      if (recordname == gr->name)
        {
          rec = gr;
          break;
        }
      else
        {
          fname = gr->getFileName ();
          if (recordname == fname)
            {
              rec = gr;
              break;
            }
        }
    }
  return rec;
}

void gridSimulation::log (gridCoreObject *object, int level, const std::string &message)
{
  if ((level > consolePrintLevel) && (level > logPrintLevel))
    {
      return;
    }
  if (!object)
    {
      object = this;
    }
  std::string cname = '[' + ((object->getID () == getID ()) ? "sim" : (fullObjectName (object) + '(' + std::to_string (object->getUserID ()) + ')')) + ']';
  std::string simtime = ((timeCurr > kNullVal / 2) ? '(' + std::to_string (timeCurr) + ')' : std::string ("(PRESTART)"));
  std::string key;
  if (level == GD_WARNING_PRINT)
    {
      key = "||WARNING||";
      ++warnCount;
    }
  else if (level == GD_ERROR_PRINT)
    {
      key = "||ERROR||";
      ++errorCount;
    }
  if (logFileStream.is_open ())
    {
      if (level <= logPrintLevel)
        {

          logFileStream << simtime  << cname << "::" << key << message << '\n';
        }
    }

  if (level <= consolePrintLevel)
    {
      std::cout << simtime << cname << "::" << key << message << '\n';
    }
}


/* *INDENT-OFF* */
static const std::map<int, std::string> alertStrings {
  {TRANSLINE_ANGLE_TRIP, "angle limit trip"},
  {TRANSLINE_LIMIT_TRIP, "transline limit trip"},
  {GENERATOR_UNDERFREQUENCY_TRIP, "generator underfrequency trip"},
  {GENERATOR_OVERSPEED_TRIP, "generator overspeed trip"},
  {GENERATOR_FAULT, "generator fault"},
  {BUS_UNDER_POWER, "bus under power"},
  { BUS_UNDER_VOLTAGE, "bus low Voltage" },
  { BUS_UNDER_FREQUENCY, "bus under frequency" },
  { LOAD_TRIP, "load trip" },
  { UNDERFREQUENCY_LOAD_TRIP, "underfrequency load trip" },
  { SWITCH1_CLOSE, "Switch 1 close" },
  { SWITCH1_OPEN, "Switch 1 open" },
  { SWITCH2_CLOSE, "Switch 2 close" },
  { SWITCH2_OPEN, "Switch 2 open" },
  { SWITCH_OPEN, "Switch open" },
  { SWITCH_CLOSE, "switch close" },
  { FUSE1_BLOWN_ANGLE, "fuse 1 blown from angle limit" },
  { FUSE1_BLOWN_CURRENT, "fuse 1 blown from current limit" },
  { FUSE_BLOWN_CURRENT, "fuse blown from current limit" },
  { FUSE_BLOWN_ANGLE, "fuse blown from angle limit" },
  { FUSE_BLOWN, "fuse blown" },
  { FUSE2_BLOWN_ANGLE, "fuse 2 blown from angle limit" },
  { FUSE2_BLOWN_CURRENT, "fuse 2 blown from current limit" },
  { BREAKER1_TRIP_CURRENT, "breaker 1 blown from current limit" },
  { BREAKER1_TRIP_ANGLE, "breaker 1 blown from angle limit" },
  { BREAKER2_TRIP_CURRENT, "breaker 1 blown from current limit" },
  { BREAKER2_TRIP_ANGLE, "breaker 1 blown from angle limit" },
  { BREAKER_TRIP_CURRENT, "breaker trip from current limit" },
  { BREAKER_TRIP_ANGLE, "breaker trip from angle limit" },
  { BREAKER_TRIP, "breaker trip" },
  { BREAKER1_RECLOSE, "breaker 1 reclose" },
  { BREAKER2_RECLOSE, "breaker 2 reclose" },
  { BREAKER_RECLOSE, "breaker reclose" },

};
/* *INDENT-ON* */

void gridSimulation::alert (gridCoreObject *object, int code)
{


  if (code > MAX_CHANGE_ALERT)
    {
      switch (code)
        {
        case UPDATE_TIME_CHANGE:
          EvQ->recheck ();
          break;
        case UPDATE_REQUIRED:
          EvQ->insert (object);
          break;
        case UPDATE_NOT_REQUIRED:
          break;
        case OBJECT_NAME_CHANGE:
        case OBJECT_ID_CHANGE:
        case OBJECT_IS_SEARCHABLE:
          gridArea::alert (object, code);
          break;
        }
    }
  else if (code < MIN_CHANGE_ALERT)
    {
      alertCount++;
      std::string astr;
      auto res = alertStrings.find (code);
      if (res != alertStrings.end ())
        {
          astr = res->second;
          log (object, GD_SUMMARY_PRINT, astr);
        }
      else
        {
          std::string message = "Unrecognized alert code (" + std::to_string (code) + ')';
          log (object, GD_SUMMARY_PRINT, message);
        }

    }
}



double gridSimulation::get (const std::string &param, gridUnits::units_t unitType) const
{
  count_t ival = kInvalidCount;
  double fval = kNullVal;
  if (param == "recordercount")
    {
      ival = static_cast<count_t> (recordList.size ());
    }
  else if (param == "alertcount")
    {
      ival = alertCount;
    }
  else if (param == "eventcount")
    {
      ival = eventCount;
    }
  else if (param == "warncount")
    {
      ival = warnCount;
    }
  else if (param == "errorcount")
    {
      ival = errorCount;
    }
  else if (param == "logprintlevel")
    {
      ival = logPrintLevel;
    }
  else if ((param == "consoleprintlevel") || (param == "printlevel"))
    {
      ival = consolePrintLevel;
    }
  else if ((param == "stepsize")||(param == "steptime"))
    {
      fval = gridUnits::unitConversionTime (stepTime, gridUnits::sec, unitType);
    }
  else if ((param == "stop") || (param == "stoptime"))
    {
      fval = gridUnits::unitConversionTime (stopTime, gridUnits::sec, unitType);
    }
  else if ((param == "currenttime")||(param == "time"))
    {
      fval = gridUnits::unitConversionTime (getCurrentTime (), gridUnits::sec, unitType);
    }
  else if (param == "starttime")
    {
      fval = gridUnits::unitConversionTime (getStartTime (), gridUnits::sec, unitType);
    }
  else if (param == "eventtime")
    {
      fval = gridUnits::unitConversionTime (getEventTime (), gridUnits::sec, unitType);
    }
  else if (param == "state")
    {
      fval = static_cast<double> (pState);
    }
  else
    {
      fval = gridArea::get (param,unitType);
    }
  return (ival != kInvalidCount) ? static_cast<double> (ival) : fval;
}


void gridSimulation::resetObjectCounters ()
{
  gridLoad::loadCount = 0;
  gridArea::areaCount = 0;
  gridBus::busCount = 0;
  gridLink::linkCount = 0;
  gridRelay::relayCount = 0;
  gridDynGenerator::genCount = 0;
}


double gridSimulation::getEventTime () const
{
  return EvQ->getNextTime ();
}

gridCoreObject * findMatchingObject (gridCoreObject *obj1, gridPrimary *src, gridPrimary *sec)
{
  gridCoreObject *obj2 = nullptr;
  if (!obj1)
    {
      return nullptr;
    }
  if (obj1->getID () == src->getID ())
    {
      return sec;
    }
  else if (dynamic_cast<gridSecondary *> (obj1))             //we know it is a gen or load so it parent should be a bus
    {
      gridBus *bus = dynamic_cast<gridBus *> (obj1->getParent ());
      gridBus *bus2 = getMatchingBus (bus, src, sec);
      if (bus2 != NULL)
        {
          if (dynamic_cast<gridDynGenerator *> (obj1))
            {
              obj2 = bus2->getGen (obj1->locIndex);
            }
          else if (dynamic_cast<gridLoad *> (obj1))
            {
              obj2 = bus2->getLoad (obj1->locIndex);
            }
        }
    }
  else if (dynamic_cast<gridBus *> (obj1))
    {
      obj2 = getMatchingBus (dynamic_cast<gridBus *> (obj1), src, sec);
    }
  else if (dynamic_cast<gridArea *> (obj1))
    {
      obj2 = getMatchingArea (dynamic_cast<gridArea *> (obj1), src, sec);
    }
  else if (dynamic_cast<gridLink *> (obj1))
    {
      obj2 = getMatchingLink (dynamic_cast<gridLink *> (obj1), src, sec);
    }
  else               //now we get ugly we are gridSecondary Object
    {
      gridCoreObject *pobj = findMatchingObject (obj1->getParent (), src, sec);
      if (pobj)
        {
          obj2 = pobj->getSubObject ("submodelcode", obj1->locIndex);
        }

    }
  return obj2;
}

