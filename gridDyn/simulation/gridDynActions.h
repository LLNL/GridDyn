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

#ifndef GRIDDYN_ACTIONS_H_
#define GRIDDYN_ACTIONS_H_


#include "gridDynTypes.h"
#include <string>


/** @brief class to define action and parameters for Griddyn operations
*/
class gridDynAction
{
public:
  /** @brief the list of possible actions
  */
  enum class gd_action_t
  {
    ignore,               //!< null action
    set,                            //!< set a parameter
    setsolver,                  //!< set a parameter in the solver
    setall,                     //!< set a parameter in all the models of a particular type
    settime,                        //!< set the time of the solver
    print,                    //!< print a variable
    initialize,                  //!< initialize the models
    powerflow,                 //!< run a powerflow
    reset,                  //!< reset the models
    iterate,                 //!< perform an iterative power flow
    eventmode,                 //!< run in event Mode
    dynamicDAE,                 //!< do a dynamic calculation using the DAE solver
    dynamicPart,                 //!< do a dynamic calculation using the partitioned solver
    dynamicDecoupled,                 //!< do a dynamic calculation using the decoupled mode
    step,                 //!< perform a single step operation
    run,                 //!< run the script or the model based on stored parameters
    save,                  //!< save the results
    load,                 //!< load a state into the simulation
    add,                 //!< add a model to the simulation
    rollback,                 //!< rollback the simulation to a particular timepoint
    checkpoint,                 //!< checkpoint the complete system state
    contingency,                  //!< perform a contingency analysis
    continuation                  //!< perform a continuation analysis
  };
  gd_action_t command = gd_action_t::ignore;        //!< the command to execute
  std::string string1;        //!< string parameter 1 of the action
  std::string string2;        //!< string parameter 2 of the action
  int val_int = -1;        //!< integer parameter of action
  double val_double = kNullVal;        //!< double parameter 1 of the action
  double val_double2 = kNullVal;        //!< double parameter 2 of the action

  /** @brief constructor*/
  gridDynAction ();

  /** @brief constructor with action string
  @param[in] operation  a string containing the information for a specific action*/
  explicit gridDynAction (const std::string &operation);

  /** @brief fill an actions parameters based on a string
  @param[in] operation  a string containing the information for a specific action*/
  void process (const std::string &operation);

  /** @brief reset the action to base state*/
  void reset ();
};


#endif