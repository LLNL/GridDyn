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

#ifndef _KINSOL_SOLVER_INTERFACE_H_
#define _KINSOL_SOLVER_INTERFACE_H_

#include "sundialsInterface.h"

namespace griddyn
{
namespace solvers
{
/** @brief solverInterface interfacing to the SUNDIALS kinsol solver
 */
class kinsolInterface : public sundialsInterface
{
  public:
    /** @brief constructor*/
    explicit kinsolInterface (const std::string &objName = "kinsol");
    /** @brief constructor loading the solverInterface structure*
    @param[in] gds  the gridDynSimulation to link with
    @param[in] sMode the solverMode for the solver
    */
    kinsolInterface (gridDynSimulation *gds, const solverMode &sMode);
    /** @brief destructor
     */
    ~kinsolInterface ();

    virtual std::shared_ptr<solverInterface>
    clone (std::shared_ptr<solverInterface> si = nullptr, bool fullCopy = false) const override;
    virtual void allocate (count_t stateCount, count_t numRoots = 0) override;
    virtual void initialize (coreTime time0) override;
    virtual void sparseReInit (sparse_reinit_modes sparseReinitMode) override;
    int solve (coreTime tStop, coreTime &tReturn, step_mode stepMode = step_mode::normal) override;
    void setConstraints () override;

    void logSolverStats (print_level logLevel, bool iconly = false) const override;
    void logErrorWeights (print_level /*logLevel*/) const override {}
    virtual double get (const std::string &param) const override;
    virtual void set (const std::string &param, const std::string &val) override;
    virtual void set (const std::string &param, double val) override;
    // wrapper functions used by kinsol and ida to call the internal functions
    friend int kinsolFunc (N_Vector state, N_Vector resid, void *user_data);
    friend int
    kinsolJacDense (long int Neq, N_Vector state, N_Vector resid, DlsMat J, void *user_data, N_Vector tmp1, N_Vector tmp2);
#ifdef KLU_ENABLE
    friend int kinsolJacSparse (N_Vector state, N_Vector resid, SlsMat J, void *user_data, N_Vector tmp1, N_Vector tmp2);

#endif
  private:
    FILE *m_kinsolInfoFile = nullptr;  //!<direct file reference for input to the solver itself
    int kinsolPrintLevel = 1;  //!< print level for the solver
#if MEASURE_TIMINGS > 0
    double kinTime = 0;
    double residTime = 0;
    double jacTime = 0;
    double jac1Time = 0;
    double kinsol1Time = 0;
#endif
};

}//namespace solvers
}//namespace griddyn

#endif
