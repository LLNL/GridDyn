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

#ifndef EVENTINTERFACE_H_
#define EVENTINTERFACE_H_

#include "gridDynTypes.h"

enum class event_execution_mode
{
  normal = 0,
  delayed = 1,
  two_part_execution = 2,
};



class eventInterface
{
public:
  virtual ~eventInterface ()
  {
  }
  virtual double nextTriggerTime () const = 0;
  virtual event_execution_mode executionMode () const = 0;
  virtual change_code trigger (double ctime) = 0;
  virtual bool isArmed () const
  {
    return true;
  }

};


#endif