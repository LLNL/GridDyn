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
#ifndef DIFFERENTIAL_RELAY_H_
#define DIFFERENTIAL_RELAY_H_

#include "Relay.h"
#include "comms/commMessage.h"
namespace griddyn
{
namespace relays
{
/** relay implementing differential relay protection scheme
*/
class differentialRelay : public Relay
{
public:
  enum differentialrelay_flags
  {
    relative_differential_flag = object_flag10,
    link_mode = object_flag11,
    bus_mode = object_flag12,
  };
protected:
  double m_resetMargin = 0.01; //!< the reset margin for clearing a fault
  double m_delayTime = 0.08; //!<[s] the delay time from first onset to trigger action
  double m_max_differential = 0.2; //!< the maximum allowable differential
  double m_minLevel = 0.01; //!< the minimum absolute level to trigger for relative differential mode
public:
  explicit differentialRelay (const std::string &objName = "diffRelay_$");
  virtual coreObject * clone (coreObject *obj = nullptr) const override;
  virtual void setFlag (const std::string &flag, bool val = true) override;
  virtual bool getFlag (const std::string &flag) const override;

  virtual void set (const std::string &param,  const std::string &val) override;

  virtual void set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;
  virtual void getParameterStrings (stringVec &pstr, paramStringType pstype) const override;
  virtual void pFlowObjectInitializeA (coreTime time0, std::uint32_t flags) override;

  virtual void receiveMessage (std::uint64_t sourceID, std::shared_ptr<commMessage> message) override;
protected:
  virtual void actionTaken (index_t ActionNum, index_t conditionNum, change_code actionReturn, coreTime actionTime) override;
  virtual void conditionTriggered (index_t conditionNum, coreTime triggerTime) override;
  virtual void conditionCleared (index_t conditionNum, coreTime triggerTime) override;

};

}//namespace relays
}//namespace griddyn
#endif