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

#ifndef _IDA_SOLVER_INTERFACE_H_
#define _IDA_SOLVER_INTERFACE_H_

#include "sundialsInterface.h"

namespace griddyn
{
namespace solvers
{
/** @brief solverInterface interfacing to the SUNDIALS IDA solver
 */
class idaInterface : public sundialsInterface
{
  public:
    count_t icCount = 0; //!< the number of times the initical condition function was called

  private:
    matrixDataSparse<double> a1;  //!< array structure for holding the Jacobian information

    std::vector<double> tempState;  //!<temporary holding location for a state vector
  public:
    /** @brief constructor*/
    explicit idaInterface (const std::string &objName = "ida");
    /** @brief alternate constructor
    @param[in] gds  the gridDynSimulation object to connect to
    @param[in] sMode the solverMode to solve For
    */
    idaInterface (gridDynSimulation *gds, const solverMode &sMode);
    /** @brief destructor*/
    ~idaInterface ();

	virtual std::unique_ptr<solverInterface> clone(bool fullCopy = false) const override;

	virtual void cloneTo(solverInterface *si, bool fullCopy = false) const override;

    virtual void allocate (count_t size, count_t numRoots = 0) override;
    void setMaxNonZeros (count_t size) override;
    virtual void initialize (coreTime t0) override;
    virtual void sparseReInit (sparse_reinit_modes mode) override;
    int calcIC (coreTime time0, coreTime tstep0, ic_modes mode, bool constraints) override;
    virtual void getCurrentData () override;
    int solve (coreTime tStop, coreTime &tReturn, step_mode stepMode = step_mode::normal) override;
    virtual void getRoots () override;
    virtual void setRootFinding (count_t numRoots) override;

    void logSolverStats (print_level logLevel, bool iconly = false) const override;
    void logErrorWeights (print_level logLevel) const override;
    double get (const std::string &param) const override;

    void setConstraints () override;
    // declare friend some helper functions
    friend int idaFunc (realtype time, N_Vector state, N_Vector dstate_dt, N_Vector resid, void *user_data);

    friend int idaJac (realtype time,
                             realtype cj,
                             N_Vector state,
                             N_Vector dstate_dt,
                             N_Vector resid,
                             SUNMatrix J,
                             void *user_data,
                             N_Vector tmp1,
                             N_Vector tmp2,
                             N_Vector tmp3);

    friend int idaRootFunc (realtype time, N_Vector state, N_Vector dstate_dt, realtype *gout, void *user_data);

  protected:
    void loadMaskElements ();
};

}//namespace solvers
}//namespace griddyn

#endif
