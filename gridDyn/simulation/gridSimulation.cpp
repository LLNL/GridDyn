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

#include "simulation/gridSimulation.h"
#include "gridArea.h"
#include "linkModels/gridLink.h"
#include "gridBus.h"
#include "events/gridEvent.h"
#include "measurement/collector.h"
#include "relays/gridRelay.h"
#include "events/eventQueue.h"
#include "loadModels/zipLoad.h"
#include "generators/gridDynGenerator.h"
#include "utilities/stringOps.h"
#include "core/coreObjectTemplates.h"
#include "core/coreExceptions.h"

#include <map>
#include <utility>

#include <cstdio>
#include <iostream>

gridSimulation::gridSimulation (const std::string &objName) : gridArea (objName)
{
  EvQ = std::make_unique<eventQueue> ();
}

gridSimulation::~gridSimulation ()
{
  opFlags.set (being_deleted);       //set this flag to handle some unusual circumstances with extra objects
}

coreObject *gridSimulation::clone (coreObject *obj) const
{
	gridSimulation *sim = cloneBase<gridSimulation, gridArea>(this, obj);
	if (!sim)
	{
		return obj;
	}
  sim->stopTime = stopTime;
  sim->currentTime = currentTime;
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

  EvQ->clone(sim->EvQ.get());
  sim->EvQ->mapObjectsOnto(sim);
  //TODO:: mapping the collectors
  return sim;
}


void gridSimulation::setErrorCode (int ecode)
{
  pState = ((ecode == GS_NO_ERROR) ? pState : gridState_t::GD_ERROR), errorCode = ecode;
}

void gridSimulation::add (std::shared_ptr<collector> col)
{
  
  if (!recordDirectory.empty ())
    {
     col->set ("directory", recordDirectory);
    }
  EvQ->insert (col);
  collectorList.push_back(std::move(col));
}

void gridSimulation::add (std::shared_ptr<gridEvent> evnt)
{
  EvQ->insert (std::move(evnt));
}

void gridSimulation::add (std::shared_ptr<eventAdapter> eA)
{
  EvQ->insert (std::move(eA));
}

void gridSimulation::add (const std::vector<std::shared_ptr<gridEvent> > &elist)
{
  for (auto &ev : elist)
    {
      EvQ->insert (ev);
    }
}

void gridSimulation::getEventObjects(std::vector<coreObject *> &objV) const
{
	EvQ->getEventObjects(objV);
}

int gridSimulation::run (coreTime /*finishTime*/)
{
  return FUNCTION_EXECUTION_FAILURE;
}

int gridSimulation::step ()
{
  return FUNCTION_EXECUTION_FAILURE;
}

void gridSimulation::saveRecorders ()
{
  //save the recorder files
  for (auto col : collectorList)
    {
	  try
	  {
		  col->flush();
		  LOG_SUMMARY("collector successfully flushed to :" + col->getSinkName());
	  }
      catch(const invalidFileName &)
        {
          LOG_ERROR ("unable to open file for writing " + col->getSinkName ());
        }
    }
}

static const std::string consoleprint("consoleprintlevel");
void gridSimulation::set (const std::string &param,  const std::string &val)
{
  std::string temp;
  if ((param == "recorddirectory")||(param=="outputdirectory"))
    {
      recordDirectory = val;
      for (auto col : collectorList)
        {
          col->set ("directory", recordDirectory);
        }
    }
  else if (param == "logprintlevel")
    {
      temp = convertToLowerCase (val);
	  logPrintLevel = stringToPrintLevel(temp);
     
    }
  else if (param == consoleprint)
    {
      temp = convertToLowerCase (val);
	  consolePrintLevel = stringToPrintLevel(temp);
    }
  else if (param == "printlevel")
    {
      temp = convertToLowerCase (val);
	  consolePrintLevel = stringToPrintLevel(temp);
	  logPrintLevel = consolePrintLevel;

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
      gridArea::set (param, val);
    }
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

void gridSimulation::set (const std::string &param, double val, gridUnits::units_t unitType)
{
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
  else if (param == "printlevel")
  {
	   auto testLevel= static_cast<print_level> (static_cast<int>(val));
	   if ((testLevel > print_level::trace) || (testLevel < print_level::no_print))
	   {
		   throw(invalidParameterValue());
	   }
	   consolePrintLevel = testLevel;
	  logPrintLevel = testLevel;
  }
  else if (param == consoleprint)
    {
	  auto testLevel = static_cast<print_level> (static_cast<int>(val));
	  if ((testLevel > print_level::trace) || (testLevel < print_level::no_print))
	  {
		  throw(invalidParameterValue());
	  }
	  consolePrintLevel = testLevel;
    }
  else if (param == "logprintlevel")
    {
	  auto testLevel = static_cast<print_level> (static_cast<int>(val));
	  if ((testLevel > print_level::trace) || (testLevel < print_level::no_print))
	  {
		  throw(invalidParameterValue());
	  }
	  logPrintLevel = testLevel;
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
		gridArea::set(param, val, unitType);
    }


}


// find collector
std::shared_ptr<collector> gridSimulation::findCollector (const std::string &collectorName)
{
	for (auto &col : collectorList)
	{
		if (collectorName == col->getName())
		{
			return col;
		}

		if (collectorName == col->getSinkName())
		{
			return col;
		}
	}
  return nullptr;
}

void gridSimulation::log (coreObject *object, print_level level, const std::string &message)
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
  std::string simtime = ((currentTime > negTime) ? '(' + std::to_string (currentTime) + ')' : std::string ("(PRESTART)"));
  std::string key;
  if (level == print_level::warning)
    {
      key = "||WARNING||";
      ++warnCount;
    }
  else if (level == print_level::error)
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

void gridSimulation::alert (coreObject *object, int code)
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
		default:
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
          log (object, print_level::summary, astr);
        }
      else
        {
          std::string message = "Unrecognized alert code (" + std::to_string (code) + ')';
          log (object, print_level::summary, message);
        }

    }
}



double gridSimulation::get (const std::string &param, gridUnits::units_t unitType) const
{
  count_t ival = kInvalidCount;
  double fval = kNullVal;
  if ((param == "collectorcount")||(param=="recordercount"))
    {
      ival = static_cast<count_t> (collectorList.size ());
    }
  else if (param == "alertcount")
    {
      ival = alertCount;
    }
  else if (param == "eventcount")
    {
      ival = EvQ->size()-1;
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
      ival = static_cast<int>(logPrintLevel);
    }
  else if ((param == "consoleprintlevel") || (param == "printlevel"))
    {
      ival = static_cast<int>(consolePrintLevel);
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

//TODO:: this really shouldn't be a function,  but still debating alternative approaches to the need it addressed
void gridSimulation::resetObjectCounters ()
{
  zipLoad::loadCount = 0;
 // gridArea::areaCount = 0;
  gridBus::busCount = 0;
  gridLink::linkCount = 0;
  gridRelay::relayCount = 0;
  gridDynGenerator::genCount = 0;
}


coreTime gridSimulation::getEventTime () const
{
  return EvQ->getNextTime ();
}

coreObject * findMatchingObject (coreObject *obj1, gridPrimary *src, gridPrimary *sec)
{
  
  if (!obj1)
    {
      return nullptr;
    }
  if (obj1->getID () == src->getID ())
    {
      return sec;
    }
  coreObject *obj2 = nullptr;
  if (dynamic_cast<gridSecondary *> (obj1))             //we know it is a gen or load so it parent should be a bus
    {
      gridBus *bus = dynamic_cast<gridBus *> (obj1->getParent ());
      gridBus *bus2 = getMatchingBus (bus, src, sec);
      if (bus2)
        {
          if (dynamic_cast<gridDynGenerator *> (obj1))
            {
              obj2 = bus2->getGen (obj1->locIndex);
            }
          else if (dynamic_cast<zipLoad *> (obj1))
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
      coreObject *pobj = findMatchingObject (obj1->getParent (), src, sec);
      if (pobj)
        {//this is an internal string sequence, likely won't be documented
          obj2 = pobj->getSubObject ("submodelcode", obj1->locIndex);
        }

    }
  return obj2;
}

