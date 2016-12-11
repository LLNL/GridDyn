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

#include "timeSeriesMulti.h"
#include <array>

class gridBus;

/** @brief a load with ramping of the load types*/
class gridRampLoad : public gridLoad
{

protected:
  double dPdt = 0.0;                                          //!< [pu] real component of the load (constant Power)
  double dQdt = 0.0;                                          //!< [pu] imaginary component of the load (constant Power)
  double drdt = 0.0;                                          //!< [pu] resistive load (constant impedance)
  double dxdt = 0.0;                                          //!< [pu] reactive load (constant impedance)
  double dIpdt = 0.0;                                         //!< [pu] real current; (constant current)
  double dIqdt = 0.0;                                         //!< [pu] imaginary current (constant current)
  double dYpdt = 0.0;                                         //!< [pu] ramp in real impedance power
  double dYqdt = 0.0;                                         //!< [pu] ramp in imaginary constant impedance power
public:
  explicit gridRampLoad (const std::string &objName = "rampLoad_$");
  gridRampLoad (double rP, double rQ, const std::string &objName = "rampLoad_$");

  virtual ~gridRampLoad ();

  virtual coreObject * clone (coreObject *obj = nullptr) const override;

  virtual void set (const std::string &param, const std::string &val) override;
  virtual void set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;

  virtual void updateLocalCache(const IOdata &args, const stateData &sD, const solverMode &sMode) override;
  void clearRamp ();
};



/** @brief a load with period pulses of various shapes*/
/*
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
  double cycleTime =kBigNum;
  double baseLoadP = 0.0;
  double baseLoadQ = 0.0;
  double shift = 0.0;                                 //!<storage for phase shift fraction (should be between 0 and 1)

  double Pfrac = 1.0;
  double Qfrac = 0.0;
  gridDyn_time nextToggleTime = 0;
public:
  explicit gridPulseLoad (const std::string &objName = "pulseLoad_$");
  gridPulseLoad (double rP, double rQ, const std::string &objName = "pulseLoad_$");

  virtual ~gridPulseLoad ()
  {
  }

  virtual coreObject * clone (coreObject *obj = nullptr) const override;

  virtual void set (const std::string &param, const std::string &val) override;
  virtual void set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;

  virtual void pFlowObjectInitializeA (gridDyn_time time0, unsigned long flags) override;
  virtual void dynObjectInitializeA (gridDyn_time time0, unsigned long flags) override;

  //virtual void loadUpdate (gridDyn_time ttime) override;
  //virtual void loadUpdateForward (gridDyn_time ttime) override;
  virtual void updateLocalCache(const IOdata &args, const stateData &sD, const solverMode &sMode) override;
  double pulseCalc (double td);
protected:
};
*/
class gridSource;
/** @brief a load that uses sources to calculate the values for the each of the load parameters
eventually will replace most of the shaped loads*/
class sourceLoad : public gridLoad
{
public:
	enum sourceLoc
	{
		p_source = 0,
		q_source = 1,
		yp_source = 2,
		yq_source = 3,
		ip_source = 4,
		iq_source = 5,
		r_source = 6,
		x_source = 7,
	};
	enum class sourceType
	{
		other,
		pulse,
		sine,
		random,
	};
private:
	
	std::vector<gridSource *> sources;
	std::array<int, 8> sourceLink; //source lookups for the values
	sourceType sType = sourceType::other;
public:
	
	explicit sourceLoad(const std::string &objName = "sourceLoad_$");
	sourceLoad(sourceType sources, const std::string &objName = "sourceLoad_$");
	virtual coreObject * clone(coreObject *obj = nullptr) const override;

	virtual void add(coreObject *obj) override;
	virtual void add(gridSource *obj);
	virtual void remove(coreObject *obj) override;
	virtual void remove(gridSource *obj);
	
	virtual void set(const std::string &param, const std::string &val) override;
	virtual void set(const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;
	virtual void setFlag(const std::string &flag, bool val=true) override;

	virtual void pFlowObjectInitializeA(gridDyn_time time0, unsigned long flags) override;
	virtual void dynObjectInitializeA(gridDyn_time time0, unsigned long flags) override;

	virtual void updateLocalCache(const IOdata &args, const stateData &sD, const solverMode &sMode) override;
	
	virtual void setState(gridDyn_time ttime, const double state[], const double dstate_dt[], const solverMode &sMode) override;
	virtual void timestep(gridDyn_time ttime, const IOdata &args, const solverMode &sMode) override;

	coreObject *find(const std::string &obj) const override;
private:
	void getSourceLoads();
	gridSource *makeSource(sourceLoc loc);
	gridSource *findSource(const std::string &srcname);
	gridSource *findSource(const std::string &srcname) const;
};

/** @brief a load with sinusoidal variations, can incorporate pulses*/
/*
class gridSineLoad : public gridPulseLoad
{
public:
  static const char pulsed_flag = object_flag6;
protected:
  double frequency = kBigNum;
  double phase = 0.0;
  gridDyn_time lastCycle = negTime;
  double Amp = 0.0;
  gridDyn_time sinePeriod = maxTime;
  double dfdt = 0.0;
  double dAdt = 0.0;

public:
  explicit gridSineLoad (const std::string &objName = "sineLoad_$");
  gridSineLoad (double rP, double rQ, const std::string &objName = "sineLoad_$");

  virtual ~gridSineLoad ()
  {
  }

  virtual coreObject * clone (coreObject *obj = nullptr) const override;

  virtual void set (const std::string &param, const std::string &val) override;
  virtual void set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;

  virtual void pFlowObjectInitializeA (gridDyn_time time0, unsigned long flags) override;
  virtual void dynObjectInitializeA (gridDyn_time time0, unsigned long flags) override;

  virtual void loadUpdate (gridDyn_time ttime) override;
  virtual void loadUpdateForward (gridDyn_time ttime) override;
  virtual void updateLocalCache(const IOdata &args, const stateData &sD, const solverMode &sMode) override;
};

class gridRandom;
*/
/** @brief a load with random variations of various types*/

/*
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
  double min_t = 0.0;
  double max_t = 100.0;
  double min_L = 0.0;
  double max_L = 0.0;
  double mean_t = 0.0;
  double mean_L = 0.0;
  double scale_t = 0.0;
  double stdev_L = 0.0;
  double zbias = 0.0;

  std::unique_ptr<gridRandom> timeGenerator;
  std::unique_ptr<gridRandom> valGenerator;

  double offset = 0;

  gridDyn_time keyTime = timeZero;

public:
  explicit gridRandomLoad (const std::string &objName = "randomLoad_$");
  gridRandomLoad (double rP, double rQ, const std::string &objName = "randomLoad_$");

  virtual ~gridRandomLoad ();

  virtual coreObject * clone (coreObject *obj = nullptr) const override;

  virtual void pFlowObjectInitializeA (gridDyn_time time0, unsigned long flags) override;
  virtual void dynObjectInitializeA (gridDyn_time time0, unsigned long flags) override;

  virtual void updateA (gridDyn_time time) override;

  virtual void timestep (gridDyn_time ttime, const IOdata &args, const solverMode &sMode) override;

  bool isTriggered ()
  {
    return opFlags[triggered_flag];
  }
  void resetTrigger ()
  {
    opFlags.reset (triggered_flag), opFlags.reset (armed_flag), offset = 0;
  }

  virtual void set (const std::string &param, const std::string &val) override;
  virtual void set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;

  void setFlag (const std::string &flag, bool val = true) override;

protected:
  void nextStep (gridDyn_time triggerTime);
  gridDyn_time ntime ();
  double nval ();
};
*/
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
  std::string fname;			//!< the name of the file
  timeSeriesMulti<double, gridDyn_time> schedLoad;		//!< time series containing the load information
  gridUnits::units_t inputUnits = gridUnits::defUnit;
  double scaleFactor = 1.0;			//!< scaling factor on the load
  index_t currIndex = 0;			//!< the current index on timeSeries
  count_t count = 0;
  double qratio = kNullVal;
  std::vector<int> columnkey;
public:
  explicit gridFileLoad (const std::string &objName = "fileLoad_$");

  ~gridFileLoad ()
  {
  }
  coreObject * clone (coreObject *obj = nullptr) const override;
  virtual void pFlowObjectInitializeA (gridDyn_time time0, unsigned long flags) override;

  virtual void updateA (gridDyn_time time) override;

  virtual void timestep (gridDyn_time ttime, const IOdata &args, const solverMode &sMode) override;

  virtual void setFlag (const std::string &param, bool val = true) override;
  virtual void set (const std::string &param, const std::string &val) override;
  virtual void set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;
private:
  count_t loadFile ();
  int columnCode (const std::string &ldc);
};

/** @brief a load with powers as a exponential function of voltage*/
class exponentialLoad : public gridLoad
{
public:
protected:
  double alphaP = 0.0;	//!< the voltage exponent parameter for the real power output
  double alphaQ = 0.0;	//!< the voltage exponent parameter for the reactive power output

public:
  explicit exponentialLoad (const std::string &objName = "expLoad_$");
  exponentialLoad (double rP, double rQ, const std::string &objName = "expLoad_$");

  virtual coreObject * clone (coreObject *obj = nullptr) const override;

  virtual void set (const std::string &param, const std::string &val) override;
  virtual void set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;

  virtual void ioPartialDerivatives (const IOdata &args, const stateData &sD, matrixData<double> &ad, const IOlocs &argLocs, const solverMode &sMode) override;
  virtual double getRealPower  (const IOdata &args, const stateData &sD, const solverMode &sMode) const override;
  virtual double getReactivePower (const IOdata &args, const stateData &sD, const solverMode &sMode) const override;
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
  double betaP = 0.0;   //!< the frequency exponent parameter for the real power output
  double betaQ = 0.0;	//!< the frequency exponent parameter for the reactive power output

public:
  explicit fDepLoad (const std::string &objName = "fdepLoad_$");
  fDepLoad (double rP, double rQ, const std::string &objName = "fdepLoad_$");

  virtual coreObject * clone (coreObject *obj = nullptr) const override;

  virtual void dynObjectInitializeA (gridDyn_time time0, unsigned long flags) override;

  virtual void set (const std::string &param, const std::string &val) override;
  virtual void set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;

  virtual void ioPartialDerivatives (const IOdata &args, const stateData &sD, matrixData<double> &ad, const IOlocs &argLocs, const solverMode &sMode) override;
  virtual double getRealPower  (const IOdata &args, const stateData &sD, const solverMode &sMode) const override;
  virtual double getReactivePower (const IOdata &args, const stateData &sD, const solverMode &sMode) const override;
  virtual double getRealPower (double V) const override;
  virtual double getReactivePower (double V) const override;
  virtual double getRealPower () const override;
  virtual double getReactivePower () const override;
  virtual double getRealPower(double V, double f) const;
  virtual double getReactivePower(double V,double f) const;

};


#endif
