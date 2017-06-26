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

#ifndef GRIDSOURCE_H_
#define GRIDSOURCE_H_
#pragma once
#include "gridSubModel.h"

namespace griddyn
{
/** Source is a signal generator in GridDyn.
The component Definition class defines the interface for a Source
*/
class Source : public gridSubModel
{
public:
  std::string m_purpose;        //!< string for use by applications to indicate usage
protected:
  double m_tempOut = 0;      //!< temporary output corresponding to desired time
  coreTime lastTime = timeZero;       //!<storage for the previously queried time
  //gridUnits::units_t outputUnits = gridUnits::defUnit;
public:
  Source (const std::string &objName = "source_#", double startVal = 0.0);
  virtual coreObject * clone (coreObject *obj = nullptr) const override;

  virtual void set (const std::string &param,  const std::string &val) override;
  virtual void set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;

  virtual void timestep (coreTime time, const IOdata &inputs,const solverMode &sMode) override;

  virtual IOdata getOutputs (const IOdata &inputs, const stateData &sD, const solverMode &sMode) const override;
  virtual double getOutput (const IOdata &inputs, const stateData &sD, const solverMode &sMode, index_t num = 0) const override;

  virtual double getOutput (index_t num = 0) const override;
  virtual index_t getOutputLoc (const solverMode &sMode,  index_t num = 0) const override;
  
  virtual count_t outputDependencyCount(index_t num, const solverMode &sMode) const override;
  virtual void setState(coreTime time, const double state[], const double dstate_dt[], const solverMode &sMode) override;
  virtual void updateOutput(coreTime time);
  virtual void updateLocalCache(const IOdata &inputs, const stateData &sD, const solverMode &sMode) override;
 
  /** update the source output and advance the model time
  @param[in] time  the time to update to
  */
  virtual double computeOutput (coreTime time) const;
  /** set the output level 
  @param[in] newLevel the level to set the output at
  */
  virtual void setLevel(double newLevel);
};

}//namespace griddyn
#endif
