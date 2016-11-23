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
#include "timeSeries.h"
/** gridSource is a signal generator in GridDyn.
The component Definition class defines the interface for a gridSource
*/
class gridSource : public gridSubModel
{
public:
  std::string m_purpose;        //!< string for use by applications to indicate usage
protected:
  double m_tempOut = 0;      //!< temporary output corresponding to desired time
  double m_output = 0;       //!< the output corresponding to the last setTime
  double lasttime = 0;       //!<storage for the previously queried time
  //gridUnits::units_t outputUnits = gridUnits::defUnit;
public:
  gridSource (const std::string &objName = "source_#", double startVal = 0.0);
  virtual gridCoreObject * clone (gridCoreObject *obj = nullptr) const override;

  virtual void set (const std::string &param,  const std::string &val) override;
  virtual void set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;

  virtual void timestep (gridDyn_time ttime, const IOdata &args,const solverMode &sMode) override;

  virtual IOdata getOutputs (const IOdata &args, const stateData *sD, const solverMode &sMode) override;
  virtual double getOutput (const IOdata &args, const stateData *sD, const solverMode &sMode, index_t num = 0) const override;

  virtual double getOutput (index_t num = 0) const override;
  virtual index_t getOutputLoc (const solverMode &sMode,  index_t num = 0) const override;
  /** update the source output
  @param[in] ttime  the time to update to
  */
  virtual void sourceUpdate (gridDyn_time ttime);
  /** update the source output and advance the model time
  @param[in] ttime  the time to update to
  */
  virtual void sourceUpdateForward (gridDyn_time ttime);
  /** set the outputlevel 
  @param[in] newLevel the level to set the output at
  */
  virtual void setLevel(double newLevel);
};


#endif
