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

#ifndef SCHEDULER_H_
#define SCHEDULER_H_


#include "sourceModels/gridSource.h"
#include "schedulerInfo.h"
#include "comms/commManager.h"
#include <utility>
#include <list>

class AGControl;
class gridDynGenerator;
class gridCommunicator;
class commMessage;
/** object to manage scheduling for devices
*/
class scheduler : public gridSource
{
public:
protected:
  double Pmax = kBigNum;  //!< [puMW] maximum set power
  double Pmin = -kBigNum;  //!< [puMW] minimum set power
  double m_Base = 100;    //!< [MW] generator base power
  double PCurr = 0;            //!<[puMW] current power output
  std::list<tsched> pTarget;  //!< target list
  std::shared_ptr<gridCommunicator> commLink;       //!<communicator link
  commManager cManager;
  std::uint64_t dispatcher_id = 0;  //!< communication id of the dispatcher
public:
  scheduler (const std::string &objName = "scheduler_#", double initialValue=0.0);
  scheduler(double initialValue, const std::string &objName = "scheduler_#");
  virtual coreObject * clone (coreObject *obj = nullptr) const override;
  virtual ~scheduler ();

  virtual void updateA (coreTime time) override;
  virtual double predict (coreTime time);

  virtual void setTarget (coreTime time, double target);
  virtual void setTarget (double target);
  virtual void setTarget (const std::string &filename);
  virtual void setTarget (std::vector<double> &time, std::vector<double> &target);
  virtual double getTarget () const;
  double getEnergy ()
  {
    return PCurr;
  }
protected:
  virtual void dynObjectInitializeA (coreTime time, unsigned long flags) override;
  virtual void dynObjectInitializeB (const IOdata &inputs, const IOdata &desiredOutput, IOdata &inputSet) override;
public:
  virtual void set (const std::string &param,  const std::string &val) override;
  virtual void set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;
  virtual void setFlag(const std::string &flag, bool val=true) override;
  virtual double get (const std::string &param, gridUnits::units_t unitType = gridUnits::defUnit) const override;
  /** tie the scheduler to a dispatcher */
  virtual void dispatcherLink ();
  /** get the maximum available power withing a specified time window
  @param[in] time the time window to make the power
  */
  virtual double getMax(coreTime time = maxTime) const;
  /** get the low power level available withing a specified time window
  @param[in] time the time window to make the power level
  */
  virtual double getMin(coreTime time = maxTime) const;
protected:
  virtual void insertTarget (tsched ts);
  void clearSchedule ();
  virtual void receiveMessage (std::uint64_t sourceID, std::shared_ptr<commMessage> message);
};

/** @brief scheduler that can deal with ramping of the power on a continuous basis
as well as handling spinning reserve like capacity in an object
*/
class schedulerRamp : public scheduler
{
public:
  enum rampMode_t
  {
    midPoint,
    justInTime,
    onTargetRamp,
    delayed,
    interp,
  };
protected:
	
  double rampUp = kBigNum;  //!< maximum ramp rate in the up direction
  double rampDown = kBigNum; //!< maximum ramp rate in the down direction
  coreTime rampTime = 20.0 * 60.0;  //!< the ramp window
  double dPdt = 0.0;                      //!< the actual ramp rate
  double PRampCurr = 0.0;         //!< the current scheduled ramp rate
  coreTime lastTargetTime = negTime;  //!< the time of the last scheduled target power level

  double ramp10Up = kBigNum;            //!<[puMW] The 10 minute maximum up ramp
  double ramp30Up = kBigNum;            //!< the 30 minute maximum up ramp
  double ramp10Down = kBigNum;          //!< the 10 minute maximum down ramp
  double ramp30Down = kBigNum;          //!< the 30 minute maximum down ramp
  rampMode_t mode = interp;                     //!< the interpolation mode

  //spinning reserve capacity
  double reserveAvail = 0.0;                      //!< the amount of reserve power in the generator
  double reserveUse = 0.0;                        //!< the amount of reserve power to use
  coreTime reserveRampTime = 15.0 * 60.0;     //!< the time window the object has to meet the reserve
  double reserveAct = 0.0;                        //!< the actual current reserve
  double reservePriority = 0.0;           //!< the priority level of the reserve


public:
  explicit schedulerRamp (const std::string &objName = "schedulerRamp_#");
  schedulerRamp (double initialValue, const std::string &objName = "schedulerRamp_#");

  virtual coreObject * clone (coreObject *obj = nullptr) const override;
  void setTarget (coreTime time, double target) override;
  void setTarget (double target) override;
  void setTarget (const std::string &filename) override;

  virtual void updateA (coreTime time) override;
  virtual double predict (coreTime time) override;

  virtual void dynObjectInitializeA (coreTime time, unsigned long flags) override;
  virtual void dynObjectInitializeB (const IOdata &inputs, const IOdata &desiredOutput, IOdata &inputSet) override;

  virtual double getRamp (double *tRem) const;
  virtual double getRamp () const;
  virtual void set (const std::string &param,  const std::string &val) override;
  virtual void set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;

  virtual double get (const std::string &param, gridUnits::units_t unitType = gridUnits::defUnit) const override;

  virtual void setReserveTarget (double target);
  double getReserveTarget ()
  {
    return reserveUse;
  }
  double getReserveUse ()
  {
    return reserveAct;
  }
  double getReserveAvailable ()
  {
    return reserveAvail;
  }
  virtual void dispatcherLink () override;
  virtual double getMax (coreTime time = maxTime) const override;
  virtual double getMin (coreTime time = maxTime) const override;
protected:
  virtual void updatePTarget ();
  virtual void insertTarget (tsched ts) override;

  virtual void receiveMessage (std::uint64_t sourceID, std::shared_ptr<commMessage> message) override;
};

/** @brief scheduler targeted at handling regulation management
*/
class schedulerReg : public schedulerRamp
{

protected:
  double regMax;  //!< the maximum regulation an object has
  double regMin;  //!< the minimum regulation level

  double regRampUp;             //!< the rate at which the regulation must ramp
  double regRampDown;   //!< the maximum rate at which regulation can ramp down
  double regCurr = 0;   //!< the current regulation output
  double regTarget = 0.0;       //!< the current regulation target
  double regUpFrac = 0.0;       //!< the capacity of the object to keep for regulation up purposes
  double regDownFrac = 0.0;             //!< the capacity of the object to keep for regulation down purposes
  double regPriority;                //a priority queue for the AGC controller if used.
  bool regEnabled = false;   //!< flag indicating that the regulation system is active
  double pr;                    //!< ?
private:
  AGControl *agc = nullptr;


public:
  explicit schedulerReg (const std::string &objName = "schedulerReg_#");
  schedulerReg (double initialValue, const std::string &objName = "schedulerReg_#");
  schedulerReg (double initialValue,double initialReg, const std::string &objName = "schedulerReg_#");
  virtual coreObject * clone (coreObject *obj = nullptr) const override;
  ~schedulerReg ();
  void setReg (double regLevel);

  double getRegTarget () const
  {
    return regTarget;
  }
  double getReg () const
  {
    return regCurr;
  }
  double getRegUpAvailable () const
  {
    return regUpFrac * pr;
  }
  double getRegDownAvailable () const
  {
    return regDownFrac * pr;
  }
  bool getRegEnabled () const
  {
    return regEnabled;
  }

  void updateA (coreTime time) override;
  double predict (coreTime time) override;

  virtual void dynObjectInitializeA (coreTime time, unsigned long flags) override;
  virtual void dynObjectInitializeB (const IOdata &inputs, const IOdata &desiredOutput, IOdata &inputSet) override;

  double getRamp (double *tRem) const override;

  void regSettings (bool active, double upFrac = -1.0,double downFrac = -1.0);

  virtual void set (const std::string &param,  const std::string &val) override;
  virtual void set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;

  virtual double get (const std::string &param, gridUnits::units_t unitType = gridUnits::defUnit) const override;
  virtual void dispatcherLink () override;
  virtual double getMax (coreTime time = maxTime) const override;
  virtual double getMin (coreTime time = maxTime) const override;

protected:
  virtual void receiveMessage (std::uint64_t sourceID, std::shared_ptr<commMessage> message) override;
};

#endif /*SCHEDULER_H_*/

