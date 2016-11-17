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

#include "gridCore.h"
#include "gridObjects.h"
#include "eventInterface.h"
#include "core/objectOperatorInterface.h"
#include <functional>

typedef std::function<void(gridCoreObject *obj, const stateData *sD, matrixData<double> &ad, const solverMode &sMode)> objJacFunction;
typedef std::function<double(gridCoreObject *obj, const stateData *sD, const solverMode &sMode)> objStateGrabberFunction;
/**class for grabbing a subset of fields directly from the state vector for performing certain calculations
*/
class stateGrabber:public objectOperatorInterface
{
public:
  std::string field; //!< name of the field to capture

  gridUnits::units_t outputUnits = gridUnits::defUnit;  //!< desired output units
  gridUnits::units_t inputUnits = gridUnits::defUnit; //!< units of the input
  double gain = 1.0; //!< multiplier on the input
  double bias = 0.0; //!<  bias on the input
  index_t offset = kInvalidLocation; //!< the state offset location
  bool loaded = false; //!< flag indicating the grabber is loaded
  bool jacCapable = false; //!< flag indicating that the grabber can compute a Jacobian
protected:
  gridCoreObject *cobj = nullptr; //!< the target object
  objStateGrabberFunction fptr; //!< the functional to grab the data
  objJacFunction jacIfptr; //!< the functional to compute the Jacobian
  index_t prevIndex=kInvalidLocation; 
public:
  stateGrabber ()
  {
  }
  explicit stateGrabber(gridCoreObject *obj);

  stateGrabber (std::string fld, gridCoreObject *obj);
  virtual ~stateGrabber ()
  {
  }
  virtual std::shared_ptr<stateGrabber> clone (std::shared_ptr<stateGrabber > ggb = nullptr) const;
  virtual int updateField (std::string fld);
  virtual double grabData (const stateData *sD, const solverMode &sMode);
  virtual void outputPartialDerivatives (const stateData *sD, matrixData<double> &ad, const solverMode &sMode);
  virtual void updateObject (gridCoreObject *obj, object_update_mode mode = object_update_mode::direct) override;
  virtual gridCoreObject * getObject() const override;
  virtual void getObjects(std::vector<gridCoreObject *> &objects) const override;
protected:
  void busLoadInfo (const std::string &fld);
  void linkLoadInfo (const std::string &fld);
  void relayLoadInfo (const std::string &fld);
  void secondaryLoadInfo (const std::string &fld);
  void areaLoadInfo (const std::string &fld);
};

typedef std::pair<std::function<double(gridCoreObject *, const stateData *sD, const solverMode &sMode)>, gridUnits::units_t> fstateobjectPair;

std::vector < std::shared_ptr < stateGrabber >> makeStateGrabbers (const std::string & command, gridCoreObject * obj);

/**
class with an addition capability of a totally custom function grabber call
*/
class customStateGrabber : public stateGrabber
{
public:
	customStateGrabber() {};
	explicit customStateGrabber(gridCoreObject *obj);
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
  virtual double grabData (const stateData *sD, const solverMode &sMode) override;
  virtual void outputPartialDerivatives (const stateData *sD, matrixData<double> &ad, const solverMode &sMode) override;
  virtual void updateObject (gridCoreObject *obj, object_update_mode mode = object_update_mode::direct) override;
  virtual gridCoreObject * getObject () const override;
  virtual int updateField (std::string fld) override;
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
  virtual double grabData (const stateData *sD, const solverMode &sMode) override;
  virtual void outputPartialDerivatives (const stateData *sD, matrixData<double> &ad, const solverMode &sMode) override;
  virtual void updateObject (gridCoreObject *obj, object_update_mode mode = object_update_mode::direct) override;
  void updateObject (gridCoreObject *obj, int num);
  virtual gridCoreObject * getObject () const override;
  virtual int updateField (std::string fld) override;
};

#endif