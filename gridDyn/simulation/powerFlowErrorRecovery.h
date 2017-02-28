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

#ifndef POWER_FLOW_ERROR_RECOVERY_H_
#define POWER_FLOW_ERROR_RECOVERY_H_



#include <memory>

class gridDynSimulation;
class solverInterface;

/** @brief the purpose of this class is to try to recover a valid power flow solution after a solver failure*/
class powerFlowErrorRecovery
{
public:
  /** @brief enumeration describing possible return options
  */
  enum class recovery_return_codes
  {
    more_options,
    out_of_options,
  };

  /** @brief constructor
  @param[in] gds the gridDynSimulation object to work from
  @param[in] sd the solverInterface object to work from
  */
  powerFlowErrorRecovery (gridDynSimulation *gds, std::shared_ptr<solverInterface> sd);

  /** @brief virtual destructor*/
  virtual ~powerFlowErrorRecovery ();

  /** @brief attempt the various fixes in order
  @param[in] optional error code value
  @return recovery_return_codes::more_options if attemptFix can be called again without reset
  recovery_return_codes::out_of_options if no more fix attemps are available
  */
  virtual recovery_return_codes attemptFix (int error_code = 0);

  /** @brief reset the fix counter so it can try again*/
  void reset ();
  /** @brief update recovery mechanism to use a different solver
  @param[in] sd the new solver Data object to use
  */
  void updateInfo (std::shared_ptr<solverInterface> sd);

  /** @brief return the number of attempts taken so far
  @return the number of attempts
  */
  int attempts () const;
protected:
  int attempt_number = 0;        //!< the current attempt number
  gridDynSimulation *sim;        //!< the gridDynsimulation to work from
  std::shared_ptr<solverInterface> solver;       //!< the solverInterface to use

  bool powerFlowFix1 ();
  bool powerFlowFix2 ();
  bool powerFlowFix3 ();
  bool powerFlowFix4 ();
  bool powerFlowFix5 ();

  bool lowVoltageFix ();
};


#endif
