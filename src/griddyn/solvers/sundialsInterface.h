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

#pragma once

#include "solverInterface.h"
#include "utilities/matrixDataSparse.hpp"
// SUNDIALS libraries
#include "griddyn/griddyn-config.h"  // Needed for SUNDIALS_OPENMP define
#include "nvector/nvector_serial.h"
#ifdef SUNDIALS_OPENMP
#include "nvector/nvector_openmp.h"
#include <omp.h>
#define NVECTOR_DESTROY(omp, vec) (omp) ? N_VDestroy_OpenMP (vec) : N_VDestroy_Serial (vec)
#define NVECTOR_NEW(omp, size) (omp) ? N_VNew_OpenMP (size, omp_get_max_threads ()) : N_VNew_Serial (size)
#define NVECTOR_DATA(omp, vec) (omp) ? NV_DATA_OMP (vec) : NV_DATA_S (vec)
#else
#define NVECTOR_DESTROY(omp, vec) N_VDestroy_Serial (vec)
#define NVECTOR_NEW(omp, size) N_VNew_Serial (size)
#define NVECTOR_DATA(omp, vec) NV_DATA_S (vec)
#endif

#include <sundials/sundials_linearsolver.h>
#include <sundials/sundials_types.h>
#include <sunmatrix/sunmatrix_sparse.h> /* access to sparse SUNMatrix */

#define ONE RCONST (1.0)
#define ZERO RCONST (0.0)

#define MEASURE_TIMINGS 0

#define _unused(x) ((void)(x))

namespace griddyn
{
namespace solvers
{
void sundialsErrorHandlerFunc (int error_code,
                               const char *module,
                               const char *function,
                               char *msg,
                               void *user_data);

#ifdef KLU_ENABLE
/** @brief check if the matrix is setup already
 *@param[in] J the matrix to check
 *@return true if the matrix has been loaded already false otherwise
 */
bool isSUNMatrixSetup (SUNMatrix J);

/** @brief convert an array data object to a SUNDIALS matrix
@param[in] md the matrix data to convert
@param[out] J the SUNDIALS matrix to store the data
@param[in] svsize the number of states representing the matrix
*/
void matrixDataToSUNMatrix (matrixData<double> &md, SUNMatrix J, count_t svsize);

#endif
/** brief abstract base class for SUNDIALS based SolverInterface objects doesn't really do anything on its own
just provides common functionality to SUNDIALS SolverInterface objects
*/
class sundialsInterface : public SolverInterface
{
  protected:
    count_t maxNNZ = 0;  //!< the maximum number of non-zeros that might be needed
    bool use_omp = false;  //!< helper variable to handle omp functionality
    N_Vector state = nullptr;  //!< state vector
    N_Vector dstate_dt = nullptr;  //!< dstate_dt information
    N_Vector abstols = nullptr;  //!< tolerance vector
    N_Vector consData = nullptr;  //!< constraint type Vector
    N_Vector scale = nullptr;  //!< scaling vector
    N_Vector types = nullptr;  //!< type data
    void *solverMem = nullptr;  //!< the memory used by a specific solver internally
    FILE *m_sundialsInfoFile = nullptr;  //!< direct file reference for input to the solver itself
    SUNMatrix J = nullptr;  //!< sundials matrix to use
    SUNLinearSolver LS = nullptr;  //!< the link to the linear solver to use
  public:
    explicit sundialsInterface (const std::string &objName = "sundials");
    /** @brief constructor loading the SolverInterface structure*
    @param[in] gds  the gridDynSimulation to link with
    @param[in] sMode the solverMode for the solver
    */
    sundialsInterface (gridDynSimulation *gds, const solverMode &sMode);
    /** @brief destructor
     */
    virtual ~sundialsInterface ();

    virtual std::unique_ptr<SolverInterface> clone (bool fullCopy = false) const override;

    virtual void cloneTo (SolverInterface *si, bool fullCopy = false) const override;
    virtual double *state_data () noexcept override;
    virtual double *deriv_data () noexcept override;
    virtual const double *state_data () const noexcept override;
    virtual const double *deriv_data () const noexcept override;
    virtual double *type_data () noexcept override;
    virtual const double *type_data () const noexcept override;
    virtual void allocate (count_t stateCount, count_t numRoots) override;
    virtual void setMaxNonZeros (count_t nonZeroCount) override;
    virtual double get (const std::string &param) const override;

    /** @brief get the dedicated memory space of the solver
    @return a void pointer to the memory location of the solver specific memory
    */
    void *getSolverMem () const { return solverMem; }

    friend int sundialsJac (realtype time,
                            realtype cj,
                            N_Vector state,
                            N_Vector dstate_dt,
                            SUNMatrix J,
                            void *user_data,
                            N_Vector tmp1,
                            N_Vector tmp2);

  protected:
    void KLUReInit (sparse_reinit_modes sparseReInitMode);
};

int sundialsJac (realtype time,
                 realtype cj,
                 N_Vector state,
                 N_Vector dstate_dt,
                 SUNMatrix J,
                 void *user_data,
                 N_Vector tmp1,
                 N_Vector tmp2);

}  // namespace solvers
}  // namespace griddyn
