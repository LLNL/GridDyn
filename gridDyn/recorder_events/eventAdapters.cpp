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

#include "eventAdapters.h"

#include "gridEvent.h"
#include "relays/gridRelay.h"
#include "gridCore.h"

#include <typeinfo>
#include <cmath>

std::atomic<count_t> eventAdapter::eventCounter(0);

eventAdapter::eventAdapter (double nextTime, double period) : m_period (period),m_nextTime (nextTime)
{
  eventID = ++eventCounter;
}


eventAdapter::~eventAdapter ()
{
}

std::shared_ptr<eventAdapter> eventAdapter::clone(std::shared_ptr<eventAdapter> eA) const
{
	auto newAdapter = eA;
	if (!newAdapter)
	{
		newAdapter = std::make_shared<eventAdapter>(m_nextTime, m_period);
	}
	newAdapter->m_remove_event = m_remove_event;
	newAdapter->partBdelay = partBdelay;
	newAdapter->two_part_execute = two_part_execute;
	newAdapter->partB_turn = partB_turn;
	newAdapter->partB_only = partB_only;
	return newAdapter;
}

void eventAdapter::updateObject(gridCoreObject * /*newObject*/, object_update_mode /*mode*/)
{
	
}

void eventAdapter::executeA (double /*cTime*/)
{
}

void eventAdapter::updateTime ()
{
}

change_code eventAdapter::execute (double cTime)
{
  if (m_period > 0)
    {
      m_nextTime += std::floor ((cTime - m_nextTime) / m_period) * m_period + m_period;
    }
  else
    {
      m_nextTime = kBigNum;
    }
  return change_code::no_change;
}

change_code functionEventAdapter::execute (double cTime)
{
  auto retval = fptr ();
  if (m_period > 0)
    {
      m_nextTime += std::floor ((cTime - m_nextTime) / m_period) * m_period + m_period;
    }
  else
    {
      m_remove_event = true;
    }
  return retval;
}


bool compareEventAdapters (const std::shared_ptr<eventAdapter> e1, const std::shared_ptr<eventAdapter> e2)
{
  return ((e1->m_nextTime < e2->m_nextTime) ? true : false);
}
