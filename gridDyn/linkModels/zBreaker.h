/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil;  c-set-offset 'innamespace 0; -*- */
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

#ifndef ZBREAKER_H_
#define ZBREAKER_H_


#include "linkModels/gridLink.h"
/** @brief class that acts as a zero impedance tie
used for implementing a bus-breaker model of a power system as well as creating slave buses and a few other types of linkages
*/
class zBreaker : public gridLink
{
protected:
  bool merged = false;  //!< flag indicating that the buses have been merged
public:
  zBreaker (const std::string &objName = "zbreaker_$");
  virtual coreObject * clone (coreObject *obj = nullptr) const override;
  // parameter set functions

  virtual void set (const std::string &param,  const std::string &val) override;
  virtual void set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;

  virtual void pFlowObjectInitializeA (coreTime time0, unsigned long flags) override;
  virtual void dynObjectInitializeA (coreTime time0, unsigned long flags) override;
  virtual void switchMode (index_t num, bool mode) override;

  virtual void updateLocalCache () override;
  virtual void updateLocalCache (const IOdata &inputs, const stateData &sD, const solverMode &sMode) override;
  virtual double quickupdateP () override;


  virtual void ioPartialDerivatives(index_t  /*busId*/, const stateData &, matrixData<double> &, const IOlocs & /*inputLocs*/, const solverMode &) override;
 
  virtual void outputPartialDerivatives(index_t  /*busId*/, const stateData &, matrixData<double> &, const solverMode &) override;


  virtual int fixRealPower(double /*power*/, index_t  /*measureTerminal*/, index_t  /*fixedTerminal*/ = 0, gridUnits::units_t = gridUnits::defUnit) override;

  virtual int fixPower(double /*rPower*/, double /*qPower*/, index_t  /*measureTerminal*/, index_t  /*fixedTerminal*/ = 0, gridUnits::units_t = gridUnits::defUnit) override;

  virtual void coordinateMergeStatus();

protected:

	virtual void switchChange(int switchNum) override;
  /** @brief merge the two buses together*/
  void merge ();
  /** @brief unmerge the merged buses*/
  void unmerge ();
};

#endif
