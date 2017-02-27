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

#ifndef GRIDDYN_CONDITION_H_
#define GRIDDYN_CONDITION_H_

#include "gridObjects.h"
#include "core/objectOperatorInterface.h"
#include <functional>

class grabberSet;
/** enumeration of the available comparison types*/
enum class comparison_type
{
	eq, ne, gt, ge, le, lt, null,
};

comparison_type comparisonFromString(const std::string &compStr);
std::string toString(comparison_type comp);

/**
*condition class:  sets up a comparison operator for both states and regular grabbers
**/
class gridCondition : public objectOperatorInterface
{
protected:
  double m_constant = 0.0;  //!< right hand side constant
  double m_margin = 0.0;	//!< the margin around the conditions
  double m_curr_margin = 0.0; //!< the currently used margin
  bool m_constB = false;	//!< flag indicating use of a constant RHS
  bool use_margin = false;	//!< flag indicating margin use

  std::shared_ptr<grabberSet> conditionA;        //!<grabber for left side condition
  std::shared_ptr<grabberSet> conditionB;        //!<grabber for right side condition
public:  
 
 
  /** default constructor 
  @param[in] valGrabber the grabber for the LHS of the equation
  @param[in] valGrabberSt the state portion of the grabber for the LHS of the equation
  */
  gridCondition(std::shared_ptr<grabberSet> valGrabber = nullptr);
  virtual ~gridCondition();

  virtual std::shared_ptr<gridCondition> clone(std::shared_ptr<gridCondition> gc = nullptr) const;
  /** load the grabbers for the Left hand side of the condition equation
  @param[in] valGrabber a gridGrabber
  @param[in] valGrabberSt a stateGrabber default nullptr
  */
  void setConditionLHS(std::shared_ptr<grabberSet> valGrabber);
  /** load the grabbers for the Right hand side of the condition equation
  @param[in] valGrabber a gridGrabber 
  @param[in] valGrabberSt a stateGrabber default nullptr
  */
  void setConditionRHS(std::shared_ptr<grabberSet> valGrabber);
  /**run the comparison between condition sides from gridGrabbers
  * @return returns a difference between the condition and parameter designed to go negative if the condition is met
  **/
  virtual double evalCondition ();
  /**
  *run the comparison between for state grabbers
  * @return returns a difference between the condition and parameter designed to go negative if the condition is met
  **/
  virtual double evalCondition (const stateData &sD, const solverMode &sMode);
  /**
  *get the value for which the comparison is made
  * @return returns the value for the comparison side=1 is left hand side, side=2 is the right hand side
  **/
  double getVal (int side,const stateData &sD,const solverMode &sMode) const;
  /**
  *get the value for which the comparison is made
  * @return returns the value for the comparison side=1 is left hand side, side=2 is the right hand side
  **/
  double getVal (int side) const;

  /** update the object used in the conditions
 @param[in] obj the object we want the condition based on
@param[in] mode  the mode of the update
*/
  virtual void updateObject(coreObject *obj, object_update_mode mode = object_update_mode::direct) override;

  /** evaluation the condition based on object data
  @return true if the condition evaluates true
  */
  virtual bool checkCondition () const;
  /** evaluation the condition based on state data
  @param[in] sD the state data
  @param[in] sMode the solver mode related to the data
  @return true if the condition evaluates true
  */
  virtual bool checkCondition (const stateData &sD, const solverMode &sMode) const;
  /** set the comparison operator
  @param[in] ct the comparison type*/
  void setComparison (comparison_type ct);
  /** set the comparison operator
  @param[in] compStr the comparison type as a string*/
  void setComparison(const std::string &compStr);
  
  /** set the level of the right side of the condition operate as a constant value
  @param[in] val the threshold
  */
  void setConditionRHS (double val)
  {
    m_constB = true;
    m_constant = val;
  }
  /** set the margin level for equality conditions
  @param[in] val the new margin
  */
  virtual void setMargin (double val);
  /** change the margin setting functionality
  @param[in] margin_on  a boolean true if the condition operator should use margins, false otherwise
  */
  virtual void useMargin (bool margin_on)
  {
    m_curr_margin = (margin_on) ? (m_margin) : 0.0;
    use_margin = margin_on;
  }

  virtual coreObject * getObject() const override;

  virtual void getObjects(std::vector<coreObject *> &objects) const override;
private:
  std::function<double(double A, double B,double margin)> evalf; //!< the function evaluation used in the condition
  comparison_type comp = comparison_type::gt;	//!< the condition operator
};

/** make a condition object
* function to create a condition object from a string describing the condition
* @param[in] condString a condition string  like bus1:Voltage>bus2::voltage
* @param[in] rootObject the root object to find the other objects
*/
std::unique_ptr<gridCondition> make_condition (const std::string &condString, coreObject *rootObject);
std::unique_ptr<gridCondition> make_condition (const std::string &field, const std::string &compare, double level, coreObject *rootObject);
std::unique_ptr<gridCondition> make_condition (const std::string &field, comparison_type comp, double level, coreObject *rootObject);
/** evaluate a compound condition consisting of multiple individual conditions
*/
class compoundCondition : public gridCondition
{

public:
  enum class compound_mode
  {
    c_and, c_or, c_any, c_xor,c_one_of, c_two_of, c_three_of, c_none, c_two_or_more, c_three_or_more
  };
  compoundCondition();

  virtual double evalCondition () override;
  virtual double evalCondition (const stateData &sD, const solverMode &sMode) override;
  virtual bool checkCondition () const override;
  virtual bool checkCondition (const stateData &sD, const solverMode &sMode) const override;
  void add (std::shared_ptr<gridCondition> gc);
  void setMode(compound_mode md);
private:
	bool breakFalse = true;
	bool breakTrue=false;
  std::vector<std::shared_ptr<gridCondition> > conditions;
  compound_mode mode = compound_mode::c_and;
private:
	bool evalCombinations(unsigned int trueCount) const;
};

#endif