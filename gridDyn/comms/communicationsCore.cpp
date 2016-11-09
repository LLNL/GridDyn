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

#include "communicationsCore.h"
#include "gridCommunicator.h"

std::shared_ptr<communicationsCore> communicationsCore::m_pInstance = nullptr;

std::shared_ptr<communicationsCore> communicationsCore::instance ()
{
  if (!m_pInstance)
    {  //can't use make_shared since the communicationsCore constructor is private
      m_pInstance = std::shared_ptr<communicationsCore> (new communicationsCore ());
    }
  return m_pInstance;
}

int communicationsCore::registerCommunicator (gridCommunicator *comm)
{
  auto ret = m_stringMap.emplace(comm->getName(), comm);
  if (ret.second == false)
    {
      return -1;
    }
  auto ret2 = m_idMap.emplace(comm->getID(), comm);
  if (ret2.second == false)
    {
      return -1;
    }
  return 0;
}

int communicationsCore::unregisterCommunicator(gridCommunicator *comm)
{
	auto resName = m_stringMap.find(comm->getName());
	if (resName != m_stringMap.end())
	{
		m_stringMap.erase(resName);
	}
	auto resID = m_idMap.find(comm->getID());
	if (resID != m_idMap.end())
	{
		m_idMap.erase(resID);
	}
	return 0;
}

int communicationsCore::send (std::uint64_t source, const std::string &dest, std::shared_ptr<commMessage> message)
{
  auto res = m_stringMap.find (dest);
  if (res != m_stringMap.end ())
    {
      res->second->receive (source, dest, message);
      return SEND_SUCCESS;
    }
  return DESTINATION_NOT_FOUND;
}

int communicationsCore::send (std::uint64_t source, std::uint64_t dest, std::shared_ptr<commMessage> message)
{
  auto res = m_idMap.find (dest);
  if (res != m_idMap.end ())
    {
      res->second->receive (source, dest, message);
      return SEND_SUCCESS;
    }
  return DESTINATION_NOT_FOUND;
}

communicationsCore::communicationsCore ()
{

}

std::uint64_t communicationsCore::lookup(std::string commName)
{
	auto res = m_stringMap.find(commName);
	if (res != m_stringMap.end())
	{
		return res->second->getID();
	}
	return 0;
}
std::string communicationsCore::lookup(std::uint64_t did)
{
	auto res = m_idMap.find(did);
	if (res != m_idMap.end())
	{
		return res->second->getName();
	}
	return "";
}