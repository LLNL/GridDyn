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

#ifndef STATE_GRABBER_H_
#define STATE_GRABBER_H_

#include "gridObjects.h"
#include "events/eventInterface.h"
#include "core/objectOperatorInterface.h"
#include <functional>

using objJacFunction=std::function<void(gridObject *obj, const stateData &sD, matrixData<double> &ad, const solverMode &sMode)> ;
using objStateGrabberFunction=std::function<double(gridObject *obj, const stateData &sD, const solverMode &sMode)>;

enum class jacobian_mode
{
	none,
	direct,
	computed,
};

/**class for grabbing a subset of fields directly from the state vector for performing certain calculations
*/
class stateGrabber:public objectOperatorInterface
{
public:
  std::string field; //!< name of the field to capture

  gridUnits::units_t outputUnits = gridUnits::defUnit;  //!< desired output units
  gridUnits::units_t inputUnits = gridUnits::defUnit; //!< units of the input
  index_t offset = kInvalidLocation; //!< the state offset location
  bool loaded = false; //!< flag indicating the grabber is loaded
  bool cacheUpdateRequired = false; //!< flag indicating that the cache should be updated before the call
  double gain = 1.0; //!< multiplier on the input
  double bias = 0.0; //!<  bias on the input
  
protected:
	jacobian_mode jacMode = jacobian_mode::none; //!< the mode of the Jacobian calculation
  gridObject *cobj = nullptr; //!< the target object
  objStateGrabberFunction fptr; //!< the functional to grab the data
  objJacFunction jacIfptr; //!< the functional to compute the Jacobian
  index_t prevIndex=kInvalidLocation; 
public:
	stateGrabber();
  explicit stateGrabber(coreObject *obj);

  stateGrabber (const std::string &fld, coreObject *obj);
  stateGrabber(index_t noffset, coreObject *obj);
  
  virtual std::shared_ptr<stateGrabber> clone (std::shared_ptr<stateGrabber > ggb = nullptr) const;
  virtual void updateField (const std::string &fld);
  virtual double grabData (const stateData &sD, const solverMode &sMode);
  virtual void outputPartialDerivatives (const stateData &sD, matrixData<double> &ad, const solverMode &sMode);
  virtual void updateObject (coreObject *obj, object_update_mode mode = object_update_mode::direct) override;
  virtual coreObject * getObject() const override;
  virtual void getObjects(std::vector<coreObject *> &objects) const override;

  jacobian_mode getJacobianMode() const
  {
	  return jacMode;
  }
protected:
  void busLoadInfo (const std::string &fld);
  void linkLoadInfo (const std::string &fld);
  void relayLoadInfo (const std::string &fld);
  void secondaryLoadInfo (const std::string &fld);
  void areaLoadInfo (const std::string &fld);
  void objectLoadInfo(const std::string &fld);
};

using fstateobjectPair=std::pair<std::function<double(gridObject *, const stateData &sD, const solverMode &sMode)>, gridUnits::units_t>;

std::vector < std::unique_ptr < stateGrabber >> makeStateGrabbers (const std::string & command, coreObject * obj);

/**
class with an additional capability of a totally custom function grabber call
*/
class customStateGrabber : public stateGrabber
{
public:
	customStateGrabber() {};
	explicit customStateGrabber(gridObject *obj);
  virtual std::shared_ptr<stateGrabber> clone (std::shared_ptr<stateGrabber > ggb = nullptr) const override;
  void setGrabberFunction (objStateGrabberFunction nfptr);
  void setGrabberJacFunction(objJacFunction nJfptr);
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
  virtual std::shared_ptr<stateGrabber> clone (std::shared_ptr<stateGrabber> ggb = nullptr) const override;
  virtual double grabData (const stateData &sD, const solverMode &sMode) override;
  virtual void outputPartialDerivatives (const stateData &sD, matrixData<double> &ad, const solverMode &sMode) override;
  virtual void updateObject (coreObject *obj, object_update_mode mode = object_update_mode::direct) override;
  virtual coreObject * getObject () const override;
  virtual void updateField (const std::string &fld) override;
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
  virtual std::shared_ptr<stateGrabber> clone (std::shared_ptr<stateGrabber> ggb = nullptr) const override;
  virtual double grabData (const stateData &sD, const solverMode &sMode) override;
  virtual void outputPartialDerivatives (const stateData &sD, matrixData<double> &ad, const solverMode &sMode) override;
  virtual void updateObject (coreObject *obj, object_update_mode mode = object_update_mode::direct) override;
  void updateObject (coreObject *obj, int num);
  virtual coreObject * getObject () const override;
  virtual void updateField (const std::string &fld) override;
};

#endif