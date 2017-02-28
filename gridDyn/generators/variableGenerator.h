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

#ifndef VARIABLEGENERATOR_H_
#define VARIABLEGENERATOR_H_

#include "generators/gridDynGenerator.h"


class gridBus;
class basicBlock;
class gridSource;
/** @brief class defining some additional components for a variable generator such as a renewable source like wind and solar
  the generator includes the addition of a source and a filter block to define some sort of input and a filtering function on that input*/
class variableGenerator : public gridDynGenerator
{
protected:
  gridSource *m_source = nullptr;
  basicBlock *m_cBlock = nullptr;
  double mp_Vcutout = -1.0;
  double mp_Vmax = kBigNum;
public:
  //!< @brief new submodel locations for the extra variable generator block extends the ones defined in gridDynGenerator
  enum extra_block_locations
  {
    source_loc = 5, control_block_loc = 6
  };
  /** @brief default constructor*/
  explicit variableGenerator (const std::string &objName = "varGen_$");
  variableGenerator (dynModel_t dynModel, const std::string &objName = "varGen_$");
  virtual coreObject * clone (coreObject *obj = nullptr) const override;

  /** @brief destructor method*/
  virtual ~variableGenerator ();
protected:
  // virtual void pFlowObjectInitializeA (coreTime time0, unsigned long flags) override;

  virtual void dynObjectInitializeB (const IOdata & inputs, const IOdata & desiredOutput, IOdata &fieldSet) override;
public:
  virtual void set (const std::string &param,  const std::string &val) override;
  virtual void set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;

  virtual void add (coreObject *obj) override;

  virtual void add (gridSubModel *obj) override;


  virtual void residual (const IOdata &inputs, const stateData &sD, double resid[], const solverMode &sMode) override;

  virtual void jacobianElements (const IOdata &inputs, const stateData &sD, matrixData<double> &ad, const IOlocs &inputLocs, const solverMode &sMode) override;

  virtual double getAdjustableCapacityUp  (coreTime /*time*/ = maxTime) const override
  {
    return 0.0;
  }                                                                                    //get the available adjustment Up within the specified timeframe
  virtual double getAdjustableCapacityDown (coreTime /*time*/ = maxTime) const override
  {
    return 0.0;
  }                                                                                      //get the available adjustment Up within the specified timeframe
  virtual void powerAdjust (double /*adjustment*/) override
  {
  }
  virtual coreObject * find (const std::string &object) const override;
  virtual coreObject * getSubObject (const std::string &type, index_t num) const override;
protected:
	virtual double pSetControlUpdate(const IOdata &inputs, const stateData &sD, const solverMode &sMode) override;
	virtual index_t pSetLocation(const solverMode &sMode) override;
};

#endif
