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

#ifndef SCHEDULER_H_
#define SCHEDULER_H_


#include "gridObjects.h"
#include "schedulerInfo.h"
#include <utility>
#include <list>

class AGControl;
class gridDynGenerator;
class gridCommunicator;
class commMessage;
/** object to manage scheduling for devices
*/
class scheduler : public gridSubModel
{
public:
protected:
  double Pmax = kBigNum;  //!< maximum set power
  double Pmin = -kBigNum;  //!< minimum set power
  double m_Base = 100;    //!< generator base power
  double PCurr = 0;                     //!< current power output
  std::list<tsched> pTarget;  //!< target list
  double output = 0;            //!< current output

  std::shared_ptr<gridCommunicator> commLink;       //!<communicator link
  std::string commType;                 //!< communication link type
  std::uint64_t dispatcher_id = 0;  //!< communication id of the dispatcher
public:
  scheduler (const std::string &objName = "scheduler_#");
  scheduler (double initialValue, const std::string &objName = "scheduler_#");
  virtual gridCoreObject * clone (gridCoreObject *obj = nullptr) const override;
  virtual ~scheduler ();

  virtual void updateA (double time) override;
  virtual double predict (double time);
  virtual double getOutput (index_t /*num*/ = 0) const override
  {
    return output;
  }

  virtual void setTarget (double time, double target);
  virtual void setTarget (double target);
  virtual int setTarget (const std::string &filename);
  virtual void setTarget (std::vector<double> &time, std::vector<double> &target);
  virtual double getTarget () const;
  double getEnergy ()
  {
    return PCurr;
  }
protected:
  virtual void objectInitializeA (double time, unsigned long flags) override;
  virtual void objectInitializeB (const IOdata &args, const IOdata &outputSet, IOdata &inputSet) override;
public:
  virtual int set (const std::string &param,  const std::string &val) override;
  virtual int set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;

  virtual double get (const std::string &param, gridUnits::units_t unitType = gridUnits::defUnit) const override;
  void setTime (double time) override;
  virtual void dispatcherLink ();
  virtual double getMax (double /*time*/ = kBigNum) const
  {
    return Pmax;
  }
  virtual double getMin (double /*time*/ = -kBigNum) const
  {
    return Pmin;
  }
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
  double rampTime = 20 * 60;  //!< the ramp window
  double dPdt = 0;                      //!< the actual ramp rate
  double PRampCurr = 0;         //!< the current scheduled ramp rate
  double lastTargetTime = -kBigNum;  //!< the time of the last scheduled target power level

  double ramp10Up = kBigNum;            //!< The 10 minute maximum up ramp
  double ramp30Up = kBigNum;            //!< the 30 minute maximum up ramp
  double ramp10Down = kBigNum;          //!< the 10 minute maximum down ramp
  double ramp30Down = kBigNum;          //!< the 30 minute maximum down ramp
  rampMode_t mode = interp;                     //!< the interpolation mode

  //spinning reserve capacity
  double reserveAvail = 0;                      //!< the amount of reserve power in the generator
  double reserveUse = 0;                        //!< the amount of reserve power to use
  double reserveRampTime = 15 * 60;     //!< the time window the object has to meet the reserve
  double reserveAct = 0;                        //!< the actual current reserve
  double reservePriority = 0;           //!< the priority level of the reserve


public:
  schedulerRamp (const std::string &objName = "schedulerRamp_#");
  schedulerRamp (double initialValue, const std::string &objName = "schedulerRamp_#");
  virtual ~schedulerRamp ()
  {
  }
  virtual gridCoreObject * clone (gridCoreObject *obj = nullptr) const override;
  void setTarget (double time, double target) override;
  void setTarget (double target) override;
  int setTarget (const std::string &filename) override;

  virtual void updateA (double time) override;
  virtual double predict (double time) override;

  virtual void objectInitializeA (double time, unsigned long flags) override;
  virtual void objectInitializeB (const IOdata &args, const IOdata &outputSet, IOdata &inputSet) override;

  virtual double getRamp (double *tRem) const;
  virtual double getRamp () const;
  virtual int set (const std::string &param,  const std::string &val) override;
  virtual int set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;

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
  virtual double getMax (double time = kBigNum) const override;
  virtual double getMin (double time = -kBigNum) const override;
protected:
  virtual void updatePTarget ();
  virtual void insertTarget (tsched ts) override;

  virtual void receiveMessage (std::uint64_t sourceID, std::shared_ptr<commMessage> message) override;
};

/** @brief scheduler targetted at handling regulation management
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
  schedulerReg (const std::string &objName = "schedulerReg_#");
  schedulerReg (double initialValue, const std::string &objName = "schedulerReg_#");
  schedulerReg (double initialValue,double initialReg, const std::string &objName = "schedulerReg_#");
  virtual gridCoreObject * clone (gridCoreObject *obj = nullptr) const override;
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

  void updateA (double time) override;
  double predict (double time) override;

  virtual void objectInitializeA (double time, unsigned long flags) override;
  virtual void objectInitializeB (const IOdata &args, const IOdata &outputSet, IOdata &inputSet) override;

  double getRamp (double *tRem) const override;

  void regSettings (bool active, double upFrac = -1.0,double downFrac = -1.0);

  virtual int set (const std::string &param,  const std::string &val) override;
  virtual int set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;

  virtual double get (const std::string &param, gridUnits::units_t unitType = gridUnits::defUnit) const override;
  virtual void dispatcherLink () override;
  virtual double getMax (double time = kBigNum) const override;
  virtual double getMin (double time = -kBigNum) const override;

protected:
  virtual void receiveMessage (std::uint64_t sourceID, std::shared_ptr<commMessage> message) override;
};

#endif /*SCHEDULER_H_*/

