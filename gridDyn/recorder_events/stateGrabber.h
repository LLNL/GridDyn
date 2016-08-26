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

#ifndef STATE_GRABBER_H_
#define STATE_GRABBER_H_

#include "basicDefs.h"
#include "gridCore.h"
#include "gridObjects.h"
#include <functional>

//class for grabbing a subset of fields directly from the state vector for performing certain calculations
class stateGrabber
{
public:
  std::string field;

  gridUnits::units_t outputUnits = gridUnits::defUnit;
  gridUnits::units_t inputUnits = gridUnits::defUnit;
  double gain = 1.0;
  double bias = 0.0;
  index_t offset = kInvalidLocation;
  bool loaded = false;
  bool jacCapable = false;
protected:
  gridCoreObject *cobj = nullptr;
  std::function<double(const stateData *sD, const solverMode &sMode)> fptr;
  std::function<void(const stateData *sD,arrayData<double> *ad,const solverMode &sMode)> jacIfptr;
  index_t prevIndex;
public:
  stateGrabber ()
  {
  }
  stateGrabber (std::string fld, gridCoreObject *obj);
  virtual ~stateGrabber ()
  {
  }
  virtual std::shared_ptr<stateGrabber> clone (gridCoreObject *nobj = nullptr, std::shared_ptr<stateGrabber > ggb = nullptr) const;
  virtual int setInfo (std::string fld, gridCoreObject* obj);
  virtual double grabData (const stateData *sD, const solverMode &sMode);
  virtual void outputPartialDerivatives (const stateData *sD, arrayData<double> *ad, const solverMode &sMode);
  virtual void updateObject (gridCoreObject *obj);
  virtual gridCoreObject * getObject () const
  {
    return cobj;
  }
protected:
  void busLoadInfo (const std::string &fld);
  void linkLoadInfo (const std::string &fld);
  void relayLoadInfo (const std::string &fld);
  void secondaryLoadInfo (const std::string &fld);
  void areaLoadInfo (const std::string &fld);
};

std::vector < std::shared_ptr < stateGrabber >> makeStateGrabbers (const std::string & command, gridCoreObject * obj);

/**
class with an addition capability of a totally custom function grabber call
*/
class customStateGrabber : public stateGrabber
{
public:
  virtual std::shared_ptr<stateGrabber> clone (gridCoreObject *nobj = nullptr, std::shared_ptr<stateGrabber > ggb = nullptr) const override;
  void setGrabberFunction (std::function<double(const stateData *sD, const solverMode &sMode)> nfptr);
};

/** function operation on a state grabber*/
class stateFunctionGrabber : public stateGrabber
{
public:
protected:
  std::shared_ptr<stateGrabber> bgrabber;		//!< the grabber that gets the data that the function operates on
  std::string function_name;					//!< the name of the function
  std::function<double(double val)> opptr;		//!< function object

public:
  stateFunctionGrabber ()
  {
  }
  stateFunctionGrabber (std::shared_ptr<stateGrabber> ggb, std::string func);
  virtual std::shared_ptr<stateGrabber> clone (gridCoreObject *nobj = nullptr, std::shared_ptr<stateGrabber> ggb = nullptr) const override;
  virtual double grabData (const stateData *sD, const solverMode &sMode) override;
  virtual void outputPartialDerivatives (const stateData *sD, arrayData<double> *ad, const solverMode &sMode) override;
  virtual void updateObject (gridCoreObject *obj) override;
  virtual gridCoreObject * getObject () const override;
  virtual int setInfo (std::string fld, gridCoreObject* obj) override;
};

/** a state grabber with operation or two argument functions*/
class stateOpGrabber : public stateGrabber
{
protected:
  std::shared_ptr<stateGrabber> bgrabber1;	//!< grabber 1 as the first argument
  std::shared_ptr<stateGrabber> bgrabber2;	//!< grabber 2 as the second argument
  std::string op_name;			//!< the name of the operation
  std::function<double(double val1, double val2)> opptr;	//!< function pointer for a two argument function

public:
  stateOpGrabber ()
  {
  }
  stateOpGrabber (std::shared_ptr<stateGrabber> ggb1, std::shared_ptr<stateGrabber> ggb2, std::string op);
  virtual std::shared_ptr<stateGrabber> clone (gridCoreObject *nobj = nullptr, std::shared_ptr<stateGrabber> ggb = nullptr) const override;
  virtual double grabData (const stateData *sD, const solverMode &sMode) override;
  virtual void outputPartialDerivatives (const stateData *sD, arrayData<double> *ad, const solverMode &sMode) override;
  virtual void updateObject (gridCoreObject *obj) override;
  void updateObject (gridCoreObject *obj, int num);
  virtual gridCoreObject * getObject () const override;
  virtual int setInfo (std::string fld, gridCoreObject* obj) override;
};

#endif