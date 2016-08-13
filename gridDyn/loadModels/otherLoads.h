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

#ifndef OTHERLOAD_H_
#define OTHERLOAD_H_

#include "gridLoad.h"

#include "fileReaders.h"

class gridBus;

/** @brief a load with ramping of the load types*/
class gridRampLoad : public gridLoad
{

protected:
  double dPdt = 0.0;                                          //!< [p.u.] real component of the load (constant Power)
  double dQdt = 0.0;                                          //!< [p.u.] imaginary component of the load (constant Power)
  double drdt = 0.0;                                          //!< [p.u.] resistive load (constant impedance)
  double dxdt = 0.0;                                          //!< [p.u.] reactive load (constant impedance)
  double dIpdt = 0.0;                                         //!< [p.u.] real current; (constant current)
  double dIqdt = 0.0;                                         //!< [p.u.] imaginary current (constant current)
  double dYpdt = 0.0;                                         //!< [p.u.] ramp in real impedance power
  double dYqdt = 0.0;                                         //!< [p.u.] ramp in imaginary constant impedance power
public:
  gridRampLoad (const std::string &objName = "rampLoad_$");
  gridRampLoad (double rP, double rQ, const std::string &objName = "rampLoad_$");

  virtual ~gridRampLoad ();

  virtual gridCoreObject * clone (gridCoreObject *obj = nullptr) const override;

  virtual int set (const std::string &param, const std::string &val) override;
  virtual int set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;

  virtual void loadUpdate (double ttime) override;
  void clearRamp ();
};

/** @brief a load with period pulses of various shapes*/
class gridPulseLoad : public gridLoad
{
public:
  static const char invert_flag = object_flag5;
  enum class pulse_type_t
  {
    square, triangle, gaussian, biexponential, exponential, cosine, flattop, monocycle
  };
  pulse_type_t ptype = pulse_type_t::square;
protected:
  double period = kBigNum;
  double dutyCycle = 0.5;
  double transTime = 0.05;
  double A = 0.0;
  double cycleTime = kBigNum;
  double baseLoadP = 0.0;
  double baseLoadQ = 0.0;
  double shift = 0.0;                                 //!<storage for phase shift fraction (should be between 0 and 1)

  double Pfrac = 1.0;
  double Qfrac = 0.0;
  double nextToggleTime = 0;
public:
  gridPulseLoad (const std::string &objName = "pulseLoad_$");
  gridPulseLoad (double rP, double rQ, const std::string &objName = "pulseLoad_$");

  virtual ~gridPulseLoad ()
  {
  }

  virtual gridCoreObject * clone (gridCoreObject *obj = nullptr) const override;

  virtual int set (const std::string &param, const std::string &val) override;
  virtual int set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;

  virtual void pFlowObjectInitializeA (double time0, unsigned long flags) override;
  virtual void dynObjectInitializeA (double time0, unsigned long flags) override;

  virtual void loadUpdate (double ttime) override;
  virtual void loadUpdateForward (double ttime) override;
  double pulseCalc (double td);
protected:
};

/** @brief a load with sinusoidal variations, can incorporate pulses*/
class gridSineLoad : public gridPulseLoad
{
public:
  static const char pulsed_flag = object_flag6;
protected:
  double frequency = 0.0;
  double phase = 0.0;
  double lastCycle = -kBigNum;
  double Amp = 0.0;
  double sinePeriod = kBigNum;
  double dfdt = 0.0;
  double dAdt = 0.0;

public:
  gridSineLoad (const std::string &objName = "sineLoad_$");
  gridSineLoad (double rP, double rQ, const std::string &objName = "sineLoad_$");

  virtual ~gridSineLoad ()
  {
  }

  virtual gridCoreObject * clone (gridCoreObject *obj = nullptr) const override;

  virtual int set (const std::string &param, const std::string &val) override;
  virtual int set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;

  virtual void pFlowObjectInitializeA (double time0, unsigned long flags) override;
  virtual void dynObjectInitializeA (double time0, unsigned long flags) override;

  virtual void loadUpdate (double ttime) override;
  virtual void loadUpdateForward (double ttime) override;
};

class gridRandom;

/** @brief a load with random variations of various types*/
class gridRandomLoad : public gridRampLoad
{
public:
  enum random_load_flags
  {
    independent_flag = object_flag5,
    interpolate_flag = object_flag6,
    proportional_flag = object_flag7,
    repeated_flag = object_flag8,
    triggered_flag = object_flag9,
    armed_flag = object_armed_flag,

  };

protected:
  double min_t = 0;
  double max_t = 100;
  double min_L = 0;
  double max_L = 0;
  double mean_t = 0;
  double mean_L = 0;
  double scale_t = 0;
  double stdev_L = 0;
  double zbias = 0;

  std::unique_ptr<gridRandom> timeGenerator;
  std::unique_ptr<gridRandom> valGenerator;

  double offset = 0;

  double keyTime = 0;

public:
  gridRandomLoad (const std::string &objName = "randomLoad_$");
  gridRandomLoad (double rP, double rQ, const std::string &objName = "randomLoad_$");

  virtual ~gridRandomLoad ();

  virtual gridCoreObject * clone (gridCoreObject *obj = nullptr) const override;

  virtual void pFlowObjectInitializeA (double time0, unsigned long flags) override;
  virtual void dynObjectInitializeA (double time0, unsigned long flags) override;

  virtual void updateA (double time) override;

  virtual double timestep (double ttime, const IOdata &args, const solverMode &sMode) override;

  bool isTriggered ()
  {
    return opFlags[triggered_flag];
  }
  void resetTrigger ()
  {
    opFlags.reset (triggered_flag), opFlags.reset (armed_flag), offset = 0;
  }

  virtual int set (const std::string &param, const std::string &val) override;
  virtual int set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;

  int setFlag (const std::string &flag, bool val = true) override;

  void setTime (double time) override;
protected:
  void nextStep (double triggerTime);
  double ntime ();
  double nval ();
};

/** @brief a load that generates its value from files*/
class gridFileLoad : public gridRampLoad
{
public:
  enum file_load_flags
  {
    use_absolute_time_flag = object_flag7,
    use_step_change_flag = object_flag8,
  };
protected:
  std::string fname;
  timeSeries2 schedLoad;
  gridUnits::units_t inputUnits = gridUnits::defUnit;
  double scaleFactor = 1.0;
  index_t currIndex = 0;
  count_t count = 0;
  double qratio = kNullVal;
  std::vector<int> columnkey;
public:
  gridFileLoad (const std::string &objName = "fileLoad_$");

  ~gridFileLoad ()
  {
  }
  gridCoreObject * clone (gridCoreObject *obj = nullptr) const override;
  virtual void pFlowObjectInitializeA (double time0, unsigned long flags) override;

  virtual void updateA (double time) override;

  virtual double timestep (double ttime, const IOdata &args, const solverMode &sMode) override;

  void setTime (double time) override;
  virtual int setFlag (const std::string &param, bool val = true) override;
  virtual int set (const std::string &param, const std::string &val) override;
  virtual int set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;
private:
  count_t loadFile ();
  int columnCode (const std::string &ldc);
};

/** @brief a load with powers as a exponential function of voltage*/
class exponentialLoad : public gridLoad
{
public:
protected:
  double alphaP = 0.0;
  double alphaQ = 0.0;

public:
  exponentialLoad (const std::string &objName = "expLoad_$");
  exponentialLoad (double rP, double rQ, const std::string &objName = "expLoad_$");

  virtual gridCoreObject * clone (gridCoreObject *obj = nullptr) const override;

  virtual int set (const std::string &param, const std::string &val) override;
  virtual int set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;

  virtual void ioPartialDerivatives (const IOdata &args, const stateData *sD, arrayData<double> *ad, const IOlocs &argLocs, const solverMode &sMode) override;
  virtual double getRealPower  (const IOdata &args, const stateData *sD, const solverMode &sMode) override;
  virtual double getReactivePower (const IOdata &args, const stateData *sD, const solverMode &sMode) override;
  virtual double getRealPower (double V) const override;
  virtual double getReactivePower (double V) const override;
  virtual double getRealPower () const override;
  virtual double getReactivePower () const override;

};

/** @brief a load with powers as a exponential function of voltage and frequency*/
class fDepLoad : public exponentialLoad
{
public:
protected:
  double betaP = 0.0;
  double betaQ = 0.0;

public:
  fDepLoad (const std::string &objName = "fdepLoad_$");
  fDepLoad (double rP, double rQ, const std::string &objName = "fdepLoad_$");

  virtual gridCoreObject * clone (gridCoreObject *obj = nullptr) const override;

  virtual void dynObjectInitializeA (double time0, unsigned long flags) override;

  virtual int set (const std::string &param, const std::string &val) override;
  virtual int set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;

  virtual void ioPartialDerivatives (const IOdata &args, const stateData *sD, arrayData<double> *ad, const IOlocs &argLocs, const solverMode &sMode) override;
  virtual double getRealPower  (const IOdata &args, const stateData *sD, const solverMode &sMode) override;
  virtual double getReactivePower (const IOdata &args, const stateData *sD, const solverMode &sMode) override;
  virtual double getRealPower (double V) const override;
  virtual double getReactivePower (double V) const override;
  virtual double getRealPower () const override;
  virtual double getReactivePower () const override;

};


#endif
