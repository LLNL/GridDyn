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

#ifndef _SUNDIALS_SOLVER_INTERFACE_H_
#define _SUNDIALS_SOLVER_INTERFACE_H_

#include "solverInterface.h"
#include "matrixDataSparse.h"
//SUNDIALS libraries
#include "nvector/nvector_serial.h"
#ifdef HAVE_OPENMP
#include <omp.h>
#include "nvector/nvector_openmp.h"
#define NVECTOR_DESTROY(omp,vec) (omp) ? N_VDestroy_OpenMP (vec) : N_VDestroy_Serial (vec)
#define NVECTOR_NEW(omp,size) (omp) ? N_VNew_OpenMP (size, omp_get_max_threads ()) : N_VNew_Serial (size)
#define NVECTOR_DATA(omp,vec) (omp) ? NV_DATA_OMP (vec) : NV_DATA_S (vec)
#else
#define NVECTOR_DESTROY(omp,vec) N_VDestroy_Serial (vec)
#define NVECTOR_NEW(omp,size) N_VNew_Serial (size)
#define NVECTOR_DATA(omp,vec) NV_DATA_S (vec)
#endif

#include "sundials/sundials_types.h"
#include "sundials/sundials_dense.h"
#ifdef KLU_ENABLE
#include "sundials/sundials_sparse.h"
#endif

#define ONE RCONST (1.0)
#define ZERO RCONST (0.0)


#define MEASURE_TIMINGS 0


#define _unused(x) ((void)(x))

void sundialsErrorHandlerFunc (int error_code, const char *module, const char *function, char *msg, void *user_data);

#ifdef KLU_ENABLE
/** @brief check if the matrix is setup already
 *@param[in] J the matrix to check
 *@return true if the matrix has been loaded already false otherwise
 */
bool isSlsMatSetup (SlsMat J);

/** @brief convert an array data object to a SUNDIALS sparse matrix
@param[in] ad the matrix data to convert
@param[out] J the SUNDIALS matrix to store the data
@param[in] svsize the number of states representing the matrix
*/
void matrixDataToSlsMat (matrixData<double> &ad, SlsMat J,count_t svsize);

#endif
/** brief abstract base class for SUNDIALS based solverInterface objects doesn't really do anything on its own
just provides common functionality to SUNDIALS solverInterface objects
*/
class sundialsInterface : public solverInterface
{
protected:
	count_t maxNNZ = 0;															//!< the maximum number of non-zeros that might be needed

  N_Vector state = nullptr;                                                        //!< state vector
  N_Vector dstate_dt = nullptr;                                                  //!< dstate_dt information
  N_Vector abstols = nullptr;                                                     //!< tolerance vector
  N_Vector consData = nullptr;                                                     //!<constraint type Vector
  N_Vector scale = nullptr;                                                      //!< scaling vector
  N_Vector types = nullptr;						//!< type data
public:
  explicit sundialsInterface (const std::string &objName = "sundials");
  /** @brief constructor loading the solverInterface structure*
  @param[in] gds  the gridDynSimulation to link with
  @param[in] sMode the solverMode for the solver
  */
  sundialsInterface (gridDynSimulation *gds, const solverMode& sMode);
  /** @brief destructor
  */
  ~sundialsInterface ();

  virtual std::shared_ptr<solverInterface> clone(std::shared_ptr<solverInterface> si = nullptr, bool fullCopy = false) const override;
  virtual double * state_data () override;
  virtual double * deriv_data () override;
  virtual const double * state_data() const override;
  virtual const double * deriv_data() const override;
  virtual double * type_data() override;
  virtual const double *type_data() const override;
  virtual void allocate (count_t size, count_t numroots) override;
  virtual void setMaxNonZeros(count_t size) override;
  virtual double get (const std::string &param) const override;
};

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
  kinsolInterface (gridDynSimulation *gds, const solverMode& sMode);
  /** @brief destructor
  */
  ~kinsolInterface ();

  virtual std::shared_ptr<solverInterface> clone(std::shared_ptr<solverInterface> si = nullptr, bool fullCopy = false) const override;
  virtual void allocate (count_t size, count_t numroots = 0) override;
  virtual void initialize (double t0) override;
  virtual void sparseReInit (sparse_reinit_modes mode) override;
  int solve (double tStop, double &tReturn, step_mode stepMode = step_mode::normal) override;
  void setConstraints () override;

  void logSolverStats (print_level logLevel, bool iconly = false) const override;
  void logErrorWeights (print_level /*logLevel*/) const override
  {
  }
  virtual double get (const std::string &param) const override;
  virtual void set(const std::string &param, const std::string &val) override;
  virtual void set(const std::string &param, double val) override;
  //wrapper functions used by kinsol and ida to call the internal functions
  friend int kinsolFunc (N_Vector u, N_Vector f, void *user_data);
  friend int kinsolJacDense (long int N, N_Vector u, N_Vector f, DlsMat J, void *user_data, N_Vector tmp1, N_Vector tmp2);
#ifdef KLU_ENABLE
  friend int kinsolJacSparse (N_Vector u, N_Vector f, SlsMat J, void *user_data, N_Vector tmp1, N_Vector tmp2);

#endif
private:

  FILE *m_kinsolInfoFile=nullptr;                          //!<direct file reference TODO convert to stream vs FILE *

#if MEASURE_TIMINGS > 0
  double kinTime = 0;
  double residTime = 0;
  double jacTime = 0;
  double jac1Time = 0;
  double kinsol1Time = 0;
#endif
};
/** @brief solverInterface interfacing to the SUNDIALS IDA solver
*/
class idaInterface : public sundialsInterface
{
public:
  count_t icCount = 0;
private:
  matrixDataSparse<double> a1;                                                     //!< array structure for holding the Jacobian information

  std::vector<double> tempState;                                          //!<temporary holding location for a state vector
public:
  /** @brief constructor*/
  idaInterface (const std::string &objName = "ida");
  /** @brief alternate constructor
  @param[in] gds  the gridDynSimulation object to connect to
  @param[in] sMode the solverMode to solve For
  */
  idaInterface (gridDynSimulation *gds, const solverMode& sMode);
  /** @brief destructor*/
  ~idaInterface ();

  virtual std::shared_ptr<solverInterface> clone(std::shared_ptr<solverInterface> si = nullptr, bool fullCopy = false) const override;

  virtual void allocate (count_t size, count_t numroots = 0) override;
  void setMaxNonZeros (count_t size) override;
  virtual void initialize (gridDyn_time t0) override;
  virtual void sparseReInit (sparse_reinit_modes mode) override;
  int calcIC (double t0, double tstep0, ic_modes mode, bool constraints) override;
  virtual void getCurrentData () override;
  int solve (double tStop, double &tReturn, step_mode stepMode = step_mode::normal) override;
  virtual void getRoots () override;
  virtual void setRootFinding (count_t numRoots) override;

  void logSolverStats (print_level logLevel, bool iconly = false) const override;
  void logErrorWeights (print_level logLevel) const override;
  double get (const std::string &param) const override;

  void setConstraints () override;
  // declare friend some helper functions
  friend int idaFunc (realtype ttime, N_Vector state, N_Vector dstate_dt, N_Vector resid, void *user_data);
  friend int idaJacDense (long int Neq, realtype ttime, realtype cj, N_Vector state, N_Vector dstate_dt, N_Vector resid, DlsMat J, void *user_data, N_Vector tmp1, N_Vector tmp2, N_Vector tmp3);
#ifdef KLU_ENABLE
  friend int idaJacSparse (realtype ttime, realtype cj, N_Vector state, N_Vector dstate_dt, N_Vector resid, SlsMat J, void *user_data, N_Vector tmp1, N_Vector tmp2, N_Vector tmp3);
#endif
  friend int idaRootFunc (realtype ttime, N_Vector state, N_Vector dstate_dt, realtype *gout, void *user_data);
protected:
  void loadMaskElements ();
};

#ifdef LOAD_CVODE
/** @brief solverInterface interfacing to the SUNDIALS cvode solver
*/
class cvodeInterface : public sundialsInterface
{
public:
  count_t icCount = 0;
private:
  matrixDataSparse<double> a1;                         //!< array structure for holding the Jacobian information
  std::vector<double> tempState;                                                //!<temporary holding location for a state vector
  bool use_bdf = false;
  bool use_newton = false;
  double maxStep = -1.0;
  double minStep = -1.0;
  double step = 0.0;
public:
  /** @brief constructor*/
  explicit cvodeInterface (const std::string &objName = "cvode");
  /** @brief alternate constructor
  @param[in] gds  the gridDynSimulation object to connect to
  @param[in] sMode the solverMode to solve For
  */
  cvodeInterface (gridDynSimulation *gds, const solverMode& sMode);
  /** @brief destructor*/
  ~cvodeInterface ();

  virtual std::shared_ptr<solverInterface> clone(std::shared_ptr<solverInterface> si = nullptr, bool fullCopy = false) const override;
  virtual void allocate (count_t size, count_t numroots = 0) override;
  virtual void initialize (double t0) override;
  void setMaxNonZeros (count_t size) override;
  virtual void sparseReInit (sparse_reinit_modes mode) override;
  virtual void getCurrentData () override;
  int solve (double tStop, double &tReturn, step_mode stepMode = step_mode::normal) override;
  virtual void getRoots () override;
  virtual void setRootFinding (count_t numRoots) override;

  void logSolverStats (print_level logLevel, bool iconly = false) const override;
  void logErrorWeights (print_level logLevel) const override;
  virtual void set (const std::string &param, const std::string &val) override;
  virtual void set (const std::string &param, double val) override;
  double get (const std::string &param) const override;
  // declare friend some helper functions
  friend int cvodeFunc (realtype ttime, N_Vector state, N_Vector dstate_dt, void *user_data);
  friend int cvodeJacDense (long int Neq, realtype ttime, N_Vector state, N_Vector dstate_dt, DlsMat J, void *user_data, N_Vector tmp1, N_Vector tmp2, N_Vector tmp3);
#ifdef KLU_ENABLE
  friend int cvodeJacSparse (realtype ttime, N_Vector state, N_Vector dstate_dt, SlsMat J, void *user_data, N_Vector tmp1, N_Vector tmp2, N_Vector tmp3);
#endif
  friend int cvodeRootFunc (realtype ttime, N_Vector state, realtype *gout, void *user_data);
protected:
  void loadMaskElements ();
};

#endif

#ifdef LOAD_ARKODE

/** @brief solverInterface interfacing to the SUNDIALS arkode solver
*/
class arkodeInterface : public sundialsInterface
{
public:
  count_t icCount = 0;
private:
  matrixDataSparse<double> a1;                                                                                                           //!< array structure for holding the Jacobian information

  std::vector<double> tempState;                                                      //!<temporary holding location for a state vector
  bool use_bdf = false;
  bool use_newton = false;
  double maxStep = -1.0;
  double minStep = -1.0;
  double step = 0.0;
public:
  /** @brief constructor*/
  explicit arkodeInterface (const std::string &objName = "arkode");
  /** @brief alternate constructor
  @param[in] gds  the gridDynSimulation object to connect to
  @param[in] sMode the solverMode to solve For
  */
  arkodeInterface (gridDynSimulation *gds, const solverMode& sMode);
  /** @brief destructor*/
  ~arkodeInterface ();

  virtual std::shared_ptr<solverInterface> clone(std::shared_ptr<solverInterface> si = nullptr, bool fullCopy = false) const override;
  virtual void allocate (count_t size, count_t numroots = 0) override;
  virtual void initialize (double t0) override;
  void setMaxNonZeros (count_t size) override;
  virtual void sparseReInit (sparse_reinit_modes sparseReinitMode) override;
  virtual void getCurrentData () override;
  int solve (double tStop, double &tReturn, step_mode stepMode = step_mode::normal) override;
  virtual void getRoots () override;
  virtual void setRootFinding (count_t numRoots) override;

  void logSolverStats (print_level logLevel, bool iconly = false) const override;
  void logErrorWeights (print_level logLevel) const override;
  virtual void set (const std::string &param, const std::string &val) override;
  virtual void set (const std::string &param, double val) override;
  double get (const std::string &param) const override;
  // declare friend some helper functions
  friend int arkodeFunc (realtype ttime, N_Vector state, N_Vector dstate_dt, void *user_data);
  friend int arkodeJacDense (long int Neq, realtype ttime, N_Vector state, N_Vector dstate_dt, DlsMat J, void *user_data, N_Vector tmp1, N_Vector tmp2, N_Vector tmp3);
#ifdef KLU_ENABLE
  friend int arkodeJacSparse (realtype ttime, N_Vector state, N_Vector dstate_dt, SlsMat J, void *user_data, N_Vector tmp1, N_Vector tmp2, N_Vector tmp3);
#endif
  friend int arkodeRootFunc (realtype ttime, N_Vector state, realtype *gout, void *user_data);
protected:
  void loadMaskElements ();
};


#endif



#endif
