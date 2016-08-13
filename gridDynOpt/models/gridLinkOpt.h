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

#ifndef GRIDLINKOPT_H_
#define GRIDLINKOPT_H_

// headers
#include "basicDefs.h"
#include "gridOptObjects.h"
// forward classes


class gridLoadOpt;
class gridGenOpt;
class gridBusOpt;

class gridLink;

class gridLinkOpt : public gridOptObject
{

public:
  enum bus_flags
  {

  };

protected:
  gridBusOpt *B1;
  gridBusOpt *B2;

  gridLink *link = nullptr;
  double rampUpLimit;
  double rampDownLimit;

public:
  gridLinkOpt (const std::string &objName = "");
  gridLinkOpt (gridCoreObject *obj, const std::string &objName = "");
  ~gridLinkOpt ();

  virtual gridCoreObject * clone (gridCoreObject *obj = nullptr) const override;
  // add components
  virtual int add (gridCoreObject *obj) override;

  // remove components
  virtual int remove (gridCoreObject *obj) override;

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


  virtual void disable () override;
  // parameter set functions

  virtual void setOffsets (const optimOffsets &newOffsets, const optimMode &oMode) override;

  virtual int set (const std::string &param,  const std::string &val) override;
  virtual int set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;
  // parameter get functions
  virtual double get (const std::string &param, gridUnits::units_t unitType = gridUnits::defUnit) const override;


  // find components
  virtual gridCoreObject * find (const std::string &objname) const override;
  virtual gridCoreObject * getSubObject (const std::string &typeName, index_t num) const override;
  virtual gridCoreObject * findByUserID (const std::string &typeName, index_t searchID) const override;

  virtual gridOptObject * getBus (index_t index) const override;
  virtual gridOptObject * getArea (index_t index) const override;
protected:
};

#endif
