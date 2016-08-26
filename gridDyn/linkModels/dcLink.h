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

#ifndef GRID_DC_LINK_H_
#define GRID_DC_LINK_H_

#include "linkModels/gridLink.h"
/** implementing a DC transmission line model
*/
class dcLink : public gridLink
{
public:
/*  enum dclink_flags
  {
    fixed_target_power = object_flag5,
  };*/
protected:
  double Idc = 0;			//!< [puA] storage for DC current
  double r = 0;				//!< [puOhm]  the dc resistance
  double x = 0.0001;		//!< [puOhm]  the dc inductance
public:
  dcLink (const std::string &objName = "dclink_$");
  dcLink (double rP,double Lp, const std::string &objName = "dclink_$");
  //gridLink(double max_power,gridBus *bus1, gridBus *bus2);
  virtual ~dcLink ();
  virtual gridCoreObject * clone (gridCoreObject *obj = nullptr) const override;

  virtual int updateBus (gridBus *bus, index_t busnumber) override;

  virtual void updateLocalCache () override;
  virtual void updateLocalCache (const stateData *sD, const solverMode &sMode) override;

  virtual double getMaxTransfer () const override;
  virtual void pFlowObjectInitializeA (double time0, unsigned long flags) override;
  virtual void pFlowObjectInitializeB () override;

  virtual void dynObjectInitializeA (double time0, unsigned long flags) override;

  virtual void loadSizes (const solverMode &sMode, bool dynOnly) override;

  virtual double timestep (double ttime,const solverMode &sMode) override;

  virtual double quickupdateP () override
  {
    return 0;
  }

  virtual int set (const std::string &param,  const std::string &val) override;
  virtual int set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;

  //initializeB dynamics
  //virtual void dynObjectInitializeA (double time0, unsigned long flags);
  virtual void ioPartialDerivatives (index_t  busId, const stateData *sD, arrayData<double> *ad, const IOlocs &argLocs, const solverMode &sMode) override;
  virtual void outputPartialDerivatives  (index_t  busId, const stateData *sD, arrayData<double> *ad, const solverMode &sMode) override;

  virtual void jacobianElements (const stateData *sD, arrayData<double> *ad, const solverMode &sMode) override;
  virtual void residual (const stateData *sD, double resid[], const solverMode &sMode) override;
  virtual void setState (double ttime, const double state[], const double dstate_dt[], const solverMode &sMode) override;
  virtual void guess (double ttime, double state[], double dstate_dt[], const solverMode &sMode) override;
  //for computing all the jacobian elements at once
  virtual void getStateName (stringVec &stNames, const solverMode &sMode, const std::string &prefix = "") const override;
  virtual int fixRealPower (double power, index_t  terminal, index_t  fixedTerminal = 0, gridUnits::units_t unitType = gridUnits::defUnit) override;
  virtual int fixPower (double power, double /*qPower*/, index_t  terminal, index_t  fixedTerminal = 0, gridUnits::units_t unitType = gridUnits::defUnit) final
  {
    return fixRealPower (power,terminal,fixedTerminal,unitType);
  }
};


#endif