/*
 * LLNS Copyright Start
 * Copyright (c) 2014-2018, Lawrence Livermore National Security
 * This work was performed under the auspices of the U.S. Department
 * of Energy by Lawrence Livermore National Laboratory in part under
 * Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
 * Produced at the Lawrence Livermore National Laboratory.
 * All rights reserved.
 * For details, see the LICENSE file.
 * LLNS Copyright End
 */

#ifndef OUTPUT_ESTIMATOR_H_
#define OUTPUT_ESTIMATOR_H_

#include "griddyn/gridDynDefinitions.hpp"

namespace griddyn
{
namespace fmi
{
/** class to help with estimating outputs based on changes to the states and inputs in intermediate steps.
The output may not be a state variable in these contexts the FMI itself doesn't always stay updated in intermediate
time values Thus the output may need to be estimated for use in the rest of the system.  The estimate is done by
using the Jacobian values for the output from both the input and states and summing them with the actual difference
between the last known value
*/
class outputEstimator
{
  private:
    coreTime time = timeZero;  //!< the last time of known values
    double prevValue = 0.0;  //!< the previous known value
    std::vector<double> prevInputs;  //!< the last known inputs
    std::vector<double> prevStates;  //!< the last known states

  public:
    std::vector<int> stateDep;  //!< the indices of the state dependencies
    std::vector<int> inputDep;  //!< the indices of the input dependencies
    std::vector<double> stateDiff;  //!< the partial derivaties of the state dependencies
    std::vector<double> inputDiff;  //!< the partial derivatives of the input dependencies
    double timeDiff = 0.0;  //!< the partial derivative with respect to time (if there is a constant ramp
  public:
    outputEstimator () = default;
    /** construct with the known indices of the dependencies*/
    outputEstimator (std::vector<int> sDep, std::vector<int> iDep);
    /** make a guess as to the present output based on time inputs and states*/
    double estimate (coreTime time, const IOdata &inputs, const double state[]);
    /** update the estimator with new values inputs and states*/
    bool update (coreTime time, double val, const IOdata &inputs, const double state[]);
};

}  // namespace fmi
}  // namespace griddyn

#endif