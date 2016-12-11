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

#ifndef SOURCE_TYPES_H_
#define SOURCE_TYPES_H_

#include "sourceModels/gridSource.h"
#include "timeSeries.h"
#include "gridRandom.h"

/**@brief defines a ramping source
*/
class rampSource : public gridSource
{
protected:
	double mp_dOdt = 0.0;  //!< [1/s] the ramp rate of the output
public:
	rampSource(const std::string &objName = "rampSource_#", double startVal = 0.0);
	virtual coreObject * clone(coreObject *obj = nullptr) const override;

	virtual void set(const std::string &param, const std::string &val) override;
	virtual void set(const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;
	
	virtual double computeOutput(gridDyn_time ttime) const override;
	virtual double getDoutdt(const stateData &sD, const solverMode &sMode, index_t num = 0) const override;
	/** @brief clear the ramp (set it to 0)*/
	void clearRamp()
	{
		mp_dOdt = 0.0;
	}
};

/** @brief describe a pulsing source*/
class pulseSource : public gridSource
{

public:
	static const char invert_flag = object_flag3;  //
	enum class pulse_type_t
	{
		square, triangle, gaussian, biexponential, exponential, cosine, flattop, monocycle
	};
	pulse_type_t ptype = pulse_type_t::square;  //!< the type of the pulse
protected:
	gridDyn_time period = maxTime;         //!<[s] pulse period
	double dutyCycle = 0.5;           //!<[%] pulse duty cycle
	double A = 0.0;                    //!< pulse amplitude
	gridDyn_time cycleTime= maxTime;           //!<[s] the start time of the last cycle
	double baseValue;                  //!< the base level of the output
	double shift = 0;                 //!< storage for phase shift fraction (should be between 0 and 1)

public:
	pulseSource(const std::string &objName = "pulseSource_#", double startVal = 0.0);

	virtual coreObject * clone(coreObject *obj = nullptr) const override;
	virtual void set(const std::string &param, const std::string &val) override;
	virtual void set(const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;
	virtual void objectInitializeA(gridDyn_time time0, unsigned long flags) override;

	virtual void updateOutput(gridDyn_time time) override;
	virtual double computeOutput(gridDyn_time ttime) const override;
	virtual double getDoutdt(const stateData &sD, const solverMode &sMode, index_t num = 0) const override;

	virtual void setLevel(double val) override;
	/** function to calculate a value of the pulsing output
	@param[in] td the time change from the last update
	@return the output value
	*/
	double pulseCalc(double td) const;

};
/** A source generating a sinusoidal output
*/
class sineSource : public pulseSource
{
public:
	static const char pulsed_flag = object_flag4;
protected:
	double frequency = 0.0;			//!<[Hz] frequency of an oscillation
	double phase = 0.0;				//!<[rad]  the offset angle
	gridDyn_time lastCycle = negTime;		///!< time of the last cycle completion
	double Amp = 0.0;					//!< the amplitude of the pulse
	gridDyn_time sinePeriod = maxTime;		//!< the period of the sinusoid
	double dfdt = 0.0;				///!<[Hz/s] the rate of change of frequency
	double dAdt = 0.0;				//!< [1/s] the rate of change of amplitude

public:
	sineSource(const std::string &objName = "sineSource_#", double startVal = 0.0);

	virtual coreObject * clone(coreObject *obj = nullptr) const override;
	virtual void set(const std::string &param, const std::string &val) override;
	virtual void set(const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;

	virtual void objectInitializeA(gridDyn_time time0, unsigned long flags) override;

	virtual void updateOutput(gridDyn_time ttime) override;
	virtual double computeOutput(gridDyn_time ttime) const override;
};

class gridRandom;
/** @brief a source generating a random output*/
class randomSource : public rampSource
{
public:
	enum random_load_flags
	{
		interpolate_flag = object_flag5,
		proportional_flag = object_flag6,
		repeated_flag = object_flag7,
		triggered_flag = object_flag8,

	};

protected:
	double min_t = 0.0;           //!< the minimum time between random updates
	double max_t = 100;           //!< the maximum time between random updates
	double min_L = 0.0;           //!< the minimum level of the update
	double max_L = 0.0;           //!< the maximum change
	double mean_t = 0.0;          //!< the mean time between changes
	double mean_L = 0.0;            //!< the mean level change
	double scale_t = 0.0;         //!< scaling factor for the times
	double stdev_L = 0.0;         //!< the standard deviation of the level changes
	double zbias = 0.0;           //!< a factor describing the preference of changes to trend toward zero mean
	double offset = 0.0;          //!< the current bias in the value
	gridDyn_time keyTime = 0.0;         //!< the next time change

	std::unique_ptr<gridRandom> timeGenerator;            //!< random number generator for the time
	std::unique_ptr<gridRandom> valGenerator;                     //!< random number generator for the value

public:
	randomSource(const std::string &objName = "randomsource_#", double startVal = 0.0);

	virtual ~randomSource()
	{
	}

	virtual coreObject * clone(coreObject *obj = nullptr) const override;

	virtual void objectInitializeA(gridDyn_time time0, unsigned long flags) override;
	virtual void timestep(gridDyn_time ttime, const IOdata &args, const solverMode &sMode) override;


	bool isTriggered()
	{
		return opFlags[triggered_flag];
	}
	void reset();

	virtual void set(const std::string &param, const std::string &val) override;
	virtual void set(const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;
	virtual void updateA(gridDyn_time time) override;

	void setFlag(const std::string &flag, bool val = true) override;

	virtual void updateOutput(gridDyn_time time) override;
protected:
	void nextStep(gridDyn_time triggerTime);
	gridDyn_time ntime();
	double nval();
};


/** Source getting its data from a file*/
class fileSource : public rampSource
{
public:
	enum file_load_flags
	{
		use_absolute_time_flag = object_flag7,
		use_step_change_flag = object_flag8,
	};
private:
	std::string fname;  //!< name of the file
	timeSeries<double, gridDyn_time> schedLoad;  //!< time series containing the output schedule
	index_t currIndex = 0;                //!< the current location in the file
	count_t count = 0;            //!< the total number of elements in the file
	index_t m_column = 0;         //!< the column of the file to use

public:
	fileSource(const std::string filename = "", int column = 0);

	coreObject * clone(coreObject *obj = nullptr) const override;

	virtual void set(const std::string &param, const std::string &val) override;
	virtual void set(const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;

	int setFile(const std::string filename, index_t column);
	virtual void objectInitializeA(gridDyn_time time0, unsigned long flags) override;

	virtual void updateA(gridDyn_time time) override;
	virtual void timestep(gridDyn_time ttime, const IOdata &args, const solverMode &sMode) override;
	//let predict fall through to ramp function

private:
	/** @brief load the file*/
	int loadFile();
};


#endif

