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

#ifndef GRIDDYN_COMMUNICATIONS_CORE_
#define GRIDDYN_COMMUNICATIONS_CORE_

#include <memory>
#include <string>
#include <map>
#include <cstdint>
#include <vector>

class gridCommunicator;
class commMessage;

typedef std::map<std::string, std::shared_ptr<gridCommunicator> > commMapString;
typedef std::map<std::uint64_t, std::shared_ptr<gridCommunicator> > commMapID;

#define SEND_SUCCESS (0)
#define DESTINATION_NOT_FOUND (-1);

class communicationsCore
{
public:
  static std::shared_ptr<communicationsCore> instance ();
  int registerCommunicator (std::shared_ptr<gridCommunicator> comm);
  int send (std::uint64_t, const std::string &dest, std::shared_ptr<commMessage> message);
  int send (std::uint64_t, std::uint64_t, std::shared_ptr<commMessage> message);
  double getTime ()
  {
    return m_time;
  }
  void setTime (double nTime)
  {
    m_time = nTime;
  }

  std::uint64_t lookup (std::string commName);
  std::string lookup (std::uint64_t did);

private:
  communicationsCore ();
  static std::shared_ptr<communicationsCore> m_pInstance;
  commMapString m_stringMap;
  commMapID m_idMap;
  double m_time;
};

#endif