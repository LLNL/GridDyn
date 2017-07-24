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

#ifndef BREAKER_RELAY_H_
#define BREAKER_RELAY_H_

#include "Relay.h"
namespace griddyn
{
namespace relays
{
/** relay implementing a overcurrent breaker for a transmission line
*/
class breaker : public Relay
{
public:
  enum breaker_flags
  {
    nondirectional_flag = object_flag8,
    overlimit_flag = object_flag9,
    breaker_tripped_flag = object_flag10,
    nonlink_source_flag = object_flag11,
  };
protected:
  coreTime minClearingTime = timeZero;   //!<[s] minimum clearing time for from bus breaker
  coreTime recloseTime1 = 1.0;      //!<[s] first reclose time
  coreTime recloseTime2 = 5.0;    //!<[s] second reclose time
  parameter_t recloserTap = 0;       //!< From side tap multiplier
  parameter_t limit = 1.0;         //!<[puA] maximum current in puA
  coreTime lastRecloseTime = negTime;     //!<[s] last reclose time
  coreTime recloserResetTime = coreTime(60.0);    //!<[s] time the breaker has to be on before the recloser count resets
  std::uint16_t maxRecloseAttempts = 0;        //!< total number of recloses
private:
	std::uint16_t recloseAttempts = 0;  //!< reclose attempt counter
protected:
  index_t m_terminal = 1;       //!< link terminal
  gridBus *bus = nullptr;
private:
  double cTI = 0.0;			//!< storage for the current integral
  double Vbase = 120.0;       //!< Voltage base for bus1
  bool &useCTI;		//!< internal flag to use the CTI stuff
public:
  explicit breaker (const std::string &objName = "breaker_$");
  virtual coreObject * clone (coreObject *obj) const override;
  virtual void setFlag (const std::string &flag, bool val = true) override;
  virtual void set (const std::string &param,  const std::string &val) override;

  virtual void set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;

  virtual void dynObjectInitializeA (coreTime time0, std::uint32_t flags) override;
  virtual void updateA (coreTime time) override;

  //dynamic state functions
  virtual void timestep (coreTime time, const IOdata &inputs, const solverMode &sMode) override;
  virtual void jacobianElements (const IOdata &inputs, const stateData &sD, matrixData<double> &md, const IOlocs &inputLocs, const solverMode &sMode) override;
  virtual void setState (coreTime time, const double state[], const double dstate_dt[], const solverMode &sMode) override;
  virtual void residual (const IOdata &inputs, const stateData &sD, double resid[], const solverMode &sMode) override;
  virtual void guessState (coreTime time, double state[], double dstate_dt[], const solverMode &sMode) override;
  virtual void loadSizes (const solverMode &sMode, bool dynOnly) override;
  virtual void getStateName (stringVec &stNames, const solverMode &sMode, const std::string &prefix) const override;

protected:
  virtual void conditionTriggered (index_t conditionNum, coreTime triggeredTime) override;
  /** trip the breaker
  @param[in] time current time
  */
  void tripBreaker (coreTime time);
  /** reset the breaker
  @param[in] time current time
  */
  void resetBreaker (coreTime time);

};

}//namespace relays
}//namespace griddyn
#endif
