/*
* LLNS Copyright Start
* Copyright (c) 2014-2018, Lawrence Livermore National Security
* This work was performed under the auspices of the U.S. Department
* of Energy by Lawrence Livermore National Laboratory in part under
* Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
* Produced at the Lawrence Livermore National Laboratory.
* All rights reserved.
* For details, see the LICENSE file.
* LLNS Copyright End
*/
#ifndef LOAD_RELAY_H_
#define LOAD_RELAY_H_

#include "../Relay.h"
namespace griddyn
{
namespace relays
{
/** class implementing a protective relay for load objects
the protective systems include underfrequency, undervoltage, and a return time so the load automatically recovers
*/
class loadRelay : public Relay
{
public:
  enum loadrelay_flags
  {
    nondirectional_flag = object_flag10,
  };
protected:
  double cutoutVoltage = 0.0;			//!<[puV] low voltage trigger for load
  double cutoutFrequency = 0.0;		//!<[puHz] low frequency trigger for load
  coreTime voltageDelay = timeZero;			//!<[s]  the delay on the voltage trip
  coreTime frequencyDelay = timeZero;		//!<[s] the delay on the frequency tripping
  coreTime offTime = maxTime;			//!<[s] the time before the load comes back on line if the trip cause has been corrected
public:
  explicit loadRelay (const std::string &objName = "loadRelay_$");
  virtual coreObject * clone (coreObject *obj = nullptr) const override;
  virtual void setFlag (const std::string &flag, bool val = true) override;
  virtual void set (const std::string &param,  const std::string &val) override;

  virtual void set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;

  virtual void dynObjectInitializeA (coreTime time0, std::uint32_t flags) override;
protected:
  virtual void actionTaken (index_t ActionNum, index_t conditionNum, change_code actionReturn, coreTime actionTime) override;
  virtual void conditionTriggered (index_t conditionNum, coreTime triggerTime) override;
  virtual void conditionCleared (index_t conditionNum, coreTime triggerTime) override;

};
}//namespace relays
}//namespace griddyn
#endif