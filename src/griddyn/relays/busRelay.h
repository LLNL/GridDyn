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

#ifndef BUS_RELAY_H_
#define BUS_RELAY_H_

#include "Relay.h"
namespace griddyn
{
namespace relays
{
/** relay object for bus protection can isolate a bus based on voltage or frequency
with a controllable delay time operates on undervoltage and underfrequency
*/
class busRelay : public Relay
{
public:
  enum busrelay_flags
  {
    nondirectional_flag = object_flag10,  //!< specify that the relay is non directional
  };
protected:
  parameter_t cutoutVoltage = 0.0;		//!<[puV] low voltage limit
  parameter_t cutoutFrequency = 0.0;		//!<[puHz] trip on low frequency
  coreTime voltageDelay = timeZero;		//!< [s] period of time the voltage must be below limit to activate
  coreTime frequencyDelay = timeZero;		//!< [s] period of time the frequency must be below limit to activate
public:
  explicit busRelay (const std::string &objName = "busrelay_$");
  virtual coreObject * clone (coreObject *obj) const override;
  virtual void setFlag (const std::string &flag, bool val = true) override;
  virtual void set (const std::string &param,  const std::string &val) override;

  virtual void set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;

  virtual void pFlowObjectInitializeA (coreTime time0, std::uint32_t flags) override;
protected:
  virtual void actionTaken (index_t ActionNum, index_t conditionNum, change_code actionReturn, coreTime actionTime) override;

};
}//namespace relays
}//namespace griddyn
#endif