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

#ifndef FAULT_RESET_RECOVERY_H_
#define FAULT_RESET_RECOVERY_H_



#include <memory>
#include <vector>

namespace griddyn
{
class gridDynSimulation;
class solverInterface;

enum class reset_levels;

/** @brief the purpose of this class is to try to recover a valid initial condition for dynamic simulations*/
class faultResetRecovery
{
protected:
  std::vector<double> initVolts;
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
  faultResetRecovery (gridDynSimulation *gds, std::shared_ptr<solverInterface> sd);

  /** @brief virtual destructor*/
  virtual ~faultResetRecovery ();

  /** @brief attempt the various fixes in order
  @return recovery_return_codes::more_options if attemptFix can be called again without reset
  recovery_return_codes::out_of_options if no more fix attempts are available
  */
  virtual int attemptFix ();

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
  bool hasMoreFixes () const;
protected:
  int attempt_number = 0;        //!< the current attempt number
  gridDynSimulation *sim;        //!< the gridDynsimulation to work from
  std::shared_ptr<solverInterface> solver;       //!< the solverInterface to use

  int faultResetFix1 ();
  int faultResetFix2 (reset_levels rlevel);
  int faultResetFix3 ();
  int faultResetFix4 ();
};

}//namespace griddyn
#endif

