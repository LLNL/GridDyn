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

#ifndef GRIDDYN_CONDITION_H_
#define GRIDDYN_CONDITION_H_

#include "fileReaders.h"
#include "gridGrabbers.h"
#include "stateGrabber.h"
#include "gridObjects.h"

#include <functional>
/**
*condition class:  sets up a condition trigger
**/
class gridCondition
{
protected:
  double m_constant = 0.0;
  double m_margin = 0.0;
  double m_curr_margin = 0.0;
  bool m_constB = false;
  bool use_margin = false;
public:
  std::shared_ptr<gridGrabber> conditionA;        //!<grabber for left side condition
  std::shared_ptr<stateGrabber> conditionAst;        //!<grabber for left side state grabber
  std::shared_ptr<gridGrabber> conditionB;        //!<grabber for right side condition
  std::shared_ptr<stateGrabber> conditionBst;       //!<grabber for right side state grabber
  enum class comparison_type
  {
    eq,ne,gt,ge,le,lt
  };

  gridCondition ()
  {
  }
  /**
  *run the comparison between condition sides from gridGrabbers
  * @return returns a difference between the condition and parameter designed to go negative if the condition is met
  **/
  double evalCondition ();
  /**
  *run the comparison between for state grabbers
  * @return returns a difference between the condition and parameter designed to go negative if the condition is met
  **/
  double evalCondition (const stateData *sD, const solverMode &sMode);
  /**
  *get the value for which the comparison is made
  * @return returns the value for the comparison side=1 is left hand side, side=2 is the right hand side
  **/
  double getVal (int side,const stateData *sD,const solverMode &sMode) const;
  /**
  *get the value for which the comparison is made
  * @return returns the value for the comparison side=1 is left hand side, side=2 is the right hand side
  **/
  double getVal (int side) const;

  bool checkCondition ();
  bool checkCondition (const stateData *sD, const solverMode &sMode);
  void setComparison (comparison_type ct);
  void setLevel (double val)
  {
    m_constB = true;
    m_constant = val;
  }
  void setMargin (double val);
  void useMargin (bool margin_on)
  {
    m_curr_margin = (margin_on) ? (m_margin) : 0.0;
    use_margin = margin_on;
  }
private:
  std::function<double(double A, double B,double margin)> evalf;
  comparison_type comp = comparison_type::gt;
};

/** make a condition object
* function to create a condition object from a string describing the condition
* @param[in] condString a condition string  like bus1:Voltage>bus2::voltage
* @param[in] rootObject the root object to find the other objects
*/
std::shared_ptr<gridCondition> make_condition (const std::string &condString, gridCoreObject *rootObject);
std::shared_ptr<gridCondition> make_condition (const std::string &field, const std::string &compare, double level, gridCoreObject *rootObject);
/** evaluate a compound condition consisting of multiple individual conditions
*/
class compoundCondition : public gridCondition
{


public:
  enum class compound_mode
  {
    c_and, c_or, c_any, c_xor,c_one_of, c_two_of, c_three_of
  };
  compoundCondition ()
  {
  }
  double evalCondition ();
  double evalCondition (double ttime, double state[], double dstate_dt[], const solverMode &sMode);
  bool checkCondition ();
  bool checkCondition (double ttime, double state[], double dstate_dt[], const solverMode &sMode);
  int add (std::shared_ptr<gridCondition> gc);
  void setMode (compound_mode md)
  {
    mode = md;
  }
private:
  std::vector<std::shared_ptr<gridCondition> > conditions;
  compound_mode mode = compound_mode::c_and;
};

#endif