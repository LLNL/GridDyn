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

#ifndef GRIDGENOPT_H_
#define GRIDGENOPT_H_

// headers
#include "basicDefs.h"
#include "gridOptObjects.h"
// forward classes

class gridLinkOpt;
class gridLoadOpt;
class gridGenOpt;
class gridBusOpt;
class gridDynGenerator;

class gridGenOpt : public gridOptObject
{

public:
  enum optgen_flags
  {
    piecewise_linear_cost = 1,
    limit_override = 2,
  };

protected:
  gridDynGenerator *gen = nullptr;
  gridBusOpt *bus;
  double m_heatRate = -kBigNum;
  std::vector<double> Pcoeff;
  std::vector<double> Qcoeff;
  double m_penaltyCost = 0;
  double m_fuelCost = -1;
  double m_Pmax = kBigNum;
  double m_Pmin = kBigNum;
  double m_forecast = -kBigNum;
public:
  gridGenOpt (const std::string &objName = "");
  gridGenOpt (gridCoreObject *obj, const std::string &objName = "");
  ~gridGenOpt ();

  virtual gridCoreObject * clone (gridCoreObject *obj = nullptr) const override;
  // add components

  virtual int add (gridCoreObject * obj)  override;
  virtual void objectInitializeA (unsigned long flags) override;
  virtual void loadSizes (const optimMode &oMode) override;

  virtual void setValues (const optimData *oD, const optimMode &oMode) override;
  //for saving the state
  virtual void guess (double ttime, double val[], const optimMode &oMode) override;
  virtual void getTols (double tols[], const optimMode &oMode) override;
  virtual void getVariableType (double sdata[], const optimMode &oMode) override;

  virtual void valueBounds (double ttime, double upLimit[], double lowerLimit[], const optimMode &oMode) override;

  virtual void linearObj (const optimData *oD, vectData *linObj, const optimMode &oMode) override;
  virtual void quadraticObj (const optimData *oD, vectData *linObj, vectData *quadObj, const optimMode &oMode) override;

  virtual double objValue (const optimData *oD, const optimMode &oMode) override;
  virtual void derivative (const optimData *oD, double deriv[], const optimMode &oMode) override;
  virtual void jacobianElements (const optimData *oD, arrayData<double> *ad, const optimMode &oMode) override;
  virtual void getConstraints (const optimData *oD, arrayData<double> *cons, double upperLimit[], double lowerLimit[], const optimMode &oMode) override;
  virtual void constraintValue (const optimData *oD, double cVals[], const optimMode &oMode) override;
  virtual void constraintJacobianElements (const optimData *oD, arrayData<double> *ad, const optimMode &oMode) override;
  virtual void getObjName (stringVec &objNames, const optimMode &oMode, const std::string &prefix = "") override;

  // parameter set functions
  virtual int set (const std::string &param,  const std::string &val) override;
  virtual int set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;
  // parameter get functions
  virtual double get (const std::string &param, gridUnits::units_t unitType = gridUnits::defUnit) const override;

  virtual void loadCostCoeff (std::vector<double> coeff,int mode);
  // find components

  virtual gridOptObject * getBus (index_t index) const override;
  virtual gridOptObject * getArea (index_t index) const override;
protected:
};
#endif
