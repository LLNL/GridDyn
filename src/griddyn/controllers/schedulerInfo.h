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

#ifndef SCHEDULERINFO_H_
#define SCHEDULERINFO_H_

#include "gridDynDefinitions.hpp"

#define SCHEDULER_UPDATE 1501

namespace griddyn
{
class tsched
{
public:
  coreTime time = maxTime;
  double target = 0;
  tsched ()
  {
  }
  tsched (coreTime atime, double atarget) : time (atime),
                                          target (atarget)
  {
  }

};

//comparison operators for tsched classes
bool operator< (const tsched &td1, const tsched &td2);

bool operator<= (const tsched &td1, const tsched &td2);

bool operator> (const tsched &td1, const tsched &td2);

bool operator>= (const tsched &td1, const tsched &td2);

bool operator== (const tsched &td1, const tsched &td2);

bool operator!= (const tsched &td1, const tsched &td2);

bool operator< (const tsched &td1, coreTime timeC);

bool operator<= (const tsched &td1, coreTime timeC);

bool operator> (const tsched &td1, coreTime timeC);

bool operator>= (const tsched &td1, coreTime timeC);

bool operator== (const tsched &td1, coreTime timeC);

bool operator!= (const tsched &td1, coreTime timeC);




}//namespace griddyn

#endif
