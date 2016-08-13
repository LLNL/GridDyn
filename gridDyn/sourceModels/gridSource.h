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

#ifndef GRIDSOURCE_H_
#define GRIDSOURCE_H_

#include "gridObjects.h"
#include "gridRandom.h"
#include "fileReaders.h"
/** gridSource is a signal generator in GridDyn.
The component Definition class defines the interface for a gridSource
*/
class gridSource : public gridSubModel
{
public:
  std::string m_type;        //!< string for use by applications to indicate usage
protected:
  double m_tempOut = 0;      //!< temporary output corresponding to desired time
  double m_output = 0;       //!< the output corresponding to the last setTime
  double lasttime = 0;       //!<storage for the previously queried time
public:
  gridSource (const std::string &objName = "source_#", double startVal = 0.0);
  virtual gridCoreObject * clone (gridCoreObject *obj = nullptr) const override;

  virtual int set (const std::string &param,  const std::string &val) override;
  virtual int set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;

  virtual double timestep (double ttime, const IOdata &args,const solverMode &sMode) override;

  virtual IOdata getOutputs (const IOdata &args, const stateData *sD, const solverMode &sMode) override;
  virtual double getOutput (const IOdata &args, const stateData *sD, const solverMode &sMode, index_t num = 0) const override;

  virtual double getOutput (index_t num = 0) const override;
  virtual double getOutputLoc (const IOdata &args, const stateData *sD, const solverMode &sMode, index_t &currentLoc, index_t num = 0) const override;
  /** update the source output
  @param[in] ttime  the time to update to
  */
  virtual void sourceUpdate (double ttime);
  /** update the source output and advance the model time
  @param[in] ttime  the time to update to
  */
  virtual void sourceUpdateForward (double ttime);
};

/**@brief defines a ramping source
*/
class rampSource : public gridSource
{
protected:
  double mp_dOdt = 0.0;  //!< the ramp rate of the output
public:
  rampSource (const std::string &objName = "rampSource_#",double startVal = 0.0);
  virtual gridCoreObject * clone (gridCoreObject *obj = nullptr) const override;

  virtual int set (const std::string &param,  const std::string &val) override;
  virtual int set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;
  virtual void sourceUpdate (double ttime) override;
  virtual double getDoutdt (const stateData *sD, const solverMode &sMode, index_t num = 0) override;
  /** @brief clear the ramp (set it to 0)*/
  void clearRamp ()
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
  double period = kBigNum;         //!< pulse period
  double dutyCycle = 0.5;                       //!< pulse duty cycle
  double A = 0.0;                                       //!< pulse amplitude
  double cycleTime = kBigNum;           //!< the start time of the last cycle
  double baseValue;                                     //!< the base level of the output
  double shift = 0;                 //!< storage for phase shift fraction (should be between 0 and 1)

public:
  pulseSource (const std::string &objName = "pulseSource_#",double startVal = 0.0);

  virtual gridCoreObject * clone (gridCoreObject *obj = nullptr) const override;
  virtual int set (const std::string &param,  const std::string &val) override;
  virtual int set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;
  virtual void objectInitializeA (double time0, unsigned long flags) override;

  virtual void sourceUpdate (double time) override;
  virtual void sourceUpdateForward (double ttime) override;
  virtual double getDoutdt (const stateData *sD, const solverMode &sMode, index_t num = 0) override;
  /** function to calculate a value of the pulsing output
  @param[in] td the time change from the last update
  @return the output value
  */
  double pulseCalc (double td);

};
/** A source generating a sinusoidal output
*/
class sineSource : public pulseSource
{
public:
  static const char pulsed_flag = object_flag4;
protected:
  double frequency = 0.0;
  double phase = 0.0;
  double lastCycle = -kBigNum;
  double Amp = 0.0;
  double sinePeriod = kBigNum;
  double dfdt = 0.0;
  double dAdt = 0.0;

public:
  sineSource (const std::string &objName = "sineSource_#",double startVal = 0.0) : pulseSource (objName,startVal)
  {

  }

  virtual gridCoreObject * clone (gridCoreObject *obj = nullptr) const override;
  virtual int set (const std::string &param,  const std::string &val) override;
  virtual int set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;

  virtual void objectInitializeA (double time0, unsigned long flags) override;
  virtual double getDoutdt (const stateData *sD, const solverMode &sMode, index_t num = 0) override;

  virtual void sourceUpdate (double ttime) override;
  virtual void sourceUpdateForward (double ttime) override;
};

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
  double keyTime = 0.0;         //!< the next time change

  std::unique_ptr<gridRandom> timeGenerator;            //!< random number generator for the time
  std::unique_ptr<gridRandom> valGenerator;                     //!< random number generator for the value

public:
  randomSource (const std::string &objName = "randomsource_#",double startVal = 0.0);

  virtual ~randomSource ()
  {
  }

  virtual gridCoreObject * clone (gridCoreObject *obj = nullptr) const override;

  virtual void objectInitializeA (double time0, unsigned long flags) override;
  virtual double timestep (double ttime, const IOdata &args, const solverMode &sMode) override;


  bool isTriggered ()
  {
    return opFlags[triggered_flag];
  }
  void reset ();

  virtual int set (const std::string &param,  const std::string &val) override;
  virtual int set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;
  virtual void updateA (double time) override;

  int setFlag (const std::string &flag, bool val = true) override;

  void setTime (double time) override;
protected:
  void nextStep (double triggerTime);
  double ntime ();
  double nval ();
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
  timeSeries schedLoad;  //!< time series containing the output schedule
  index_t currIndex = 0;                //!< the current location in the file
  count_t count = 0;            //!< the total number of elements in the file
  index_t m_column = 0;         //!< the column of the file to use

public:
  fileSource (const std::string filename = "", int column = 0);

  gridCoreObject * clone (gridCoreObject *obj = nullptr) const override;

  virtual int set (const std::string &param,  const std::string &val) override;
  virtual int set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;

  int setFile (const std::string filename, index_t column);
  virtual void objectInitializeA (double time0, unsigned long flags) override;

  virtual void updateA (double time) override;
  virtual double timestep (double ttime, const IOdata &args, const solverMode &sMode) override;
  //let predict fall through to ramp function

  virtual void setTime (double time) override;
private:
  /** @brief load the file*/
  int loadFile ();
};


#endif
