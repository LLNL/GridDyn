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
#include "arrayDataSparse.h"
//sundials libraries
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

void sundialsErrorHandlerFunc (int error_code, const char *module, const char *function, char *msg, void *user_data);

#ifdef KLU_ENABLE

bool isSlsMatSetup (SlsMat J);

/** @brief make a solver from a string
@param[in] type the type of solverInterface to create
@return a shared_ptr to a solverInterface object
*/
void arrayDataToSlsMat (arrayData<double> *at, SlsMat J,count_t svsize);

#endif
/** brief abstract base class for sundials based solverInterface objects doesn't really do anything on its own
just provides common functionality to sundials solverInterface objects
*/
class sundialsInterface : public solverInterface
{
protected:
  int jacCallCount = 0;
  count_t maxNNZ = 0;
  N_Vector state = nullptr;                                                        //!< state vector

  N_Vector dstate_dt = nullptr;                                                                                                 //!< dstate_dt information
  N_Vector abstols = nullptr;                                                     //!< tolerance vector
  N_Vector consData = nullptr;                                                     //!<constraint type Vector
public:
  sundialsInterface ();
  /** @brief constructor loading the solverInterface structure*
  @param[in] gds  the gridDynSimulation to link with
  @param[in] sMode the solverMode for the solver
  */
  sundialsInterface (gridDynSimulation *gds, const solverMode& sMode);
  /** @brief destructor
  */
  ~sundialsInterface ();
  double * state_data () override;
  double * deriv_data () override;
  int allocate (count_t size, count_t numroots = 0) override;
  void setMaxNonZeros (count_t size) override;
  int initialize (double t0) override;
  int sparseReInit (sparse_reinit_modes mode) override;
  int solve (double tStop, double &tReturn, step_mode stepMode = step_mode::normal) override;
  void setConstraints () override;

  void logSolverStats (int logLevel, bool iconly = false) const override;
  void logErrorWeights (int /*logLevel*/) const override
  {
  }
  double get (const std::string &param) const override;
};

/** @brief solverInterface interfacing to the sundials kinsol solver
*/
class kinsolInterface : public solverInterface
{
public:
  /** @brief constructor*/
  kinsolInterface ();
  /** @brief constructor loading the solverInterface structure*
  @param[in] gds  the gridDynSimulation to link with
  @param[in] sMode the solverMode for the solver
  */
  kinsolInterface (gridDynSimulation *gds, const solverMode& sMode);
  /** @brief destructor
  */
  ~kinsolInterface ();
  double * state_data () override;
  int allocate (count_t size, count_t numroots = 0) override;
  void setMaxNonZeros (count_t size) override;
  int initialize (double t0) override;
  int sparseReInit (sparse_reinit_modes mode) override;
  int solve (double tStop, double &tReturn, step_mode stepMode = step_mode::normal) override;
  void setConstraints () override;

  void logSolverStats (int logLevel, bool iconly = false) const override;
  void logErrorWeights (int /*logLevel*/) const override
  {
  }
  double get (const std::string &param) const override;
  //wrapper functions used by kinsol and ida to call the internal functions
  friend int kinsolFunc (N_Vector u, N_Vector f, void *user_data);
  friend int kinsolJacDense (long int N, N_Vector u, N_Vector f, DlsMat J, void *user_data, N_Vector tmp1, N_Vector tmp2);
#ifdef KLU_ENABLE
  friend int kinsolJacSparse (N_Vector u, N_Vector f, SlsMat J, void *user_data, N_Vector tmp1, N_Vector tmp2);

#endif
private:
  int jacCallCount = 0;
  count_t maxNNZ = 0;
  N_Vector state = nullptr;                                                        //!< state vector

  N_Vector abstols = nullptr;                                                     //!< tolerance vector
  N_Vector consData = nullptr;                                                     //!<constraint type Vector
  N_Vector scale = nullptr;                                                                                           //!< scaling vector
  FILE *m_kinsolInfoFile;                          //!<direct file reference TODO convert to stream vs FILE *
  double solveTime = 0;                                                         //!< storage for the time the solver is called
#if MEASURE_TIMING > 0
  double kinTime = 0;
  double residTime = 0;
  double jacTime = 0;
#endif
};
/** @brief solverInterface interfacing to the sundials ida solver
*/
class idaInterface : public solverInterface
{
public:
  count_t icCount = 0;
private:
  arrayDataSparse a1;                                                                                                           //!< array structure for holding the jacobian information
  N_Vector state = nullptr;                                                  //!< state vector
  N_Vector deriv = nullptr;                                                   //!< derivative vector
  N_Vector abstols = nullptr;                                               //!< tolerance vector
  N_Vector types = nullptr;                                                 //!< inequalities and state types
  N_Vector consData = nullptr;                                               //!<constraint type Vector
  N_Vector scale = nullptr;
  N_Vector eweight = nullptr;
  N_Vector ele = nullptr;
  std::vector<double> tempState;                                          //!<temporary holding location for a state vector
public:
  /** @brief constructor*/
  idaInterface ();
  /** @brief alternate constructor
  @param[in] gds  the gridDynSimulation object to connect to
  @param[in] sMode the solverMode to solve For
  */
  idaInterface (gridDynSimulation *gds, const solverMode& sMode);
  /** @brief destructor*/
  ~idaInterface ();

  double * state_data () override;
  double * deriv_data () override;
  double * type_data () override;

  int allocate (count_t size, count_t numroots = 0) override;
  void setMaxNonZeros (count_t size) override;
  int initialize (double t0) override;
  int sparseReInit (sparse_reinit_modes mode) override;
  int calcIC (double t0, double tstep0, ic_modes mode, bool constraints) override;
  int getCurrentData () override;
  int solve (double tStop, double &tReturn, step_mode stepMode = step_mode::normal) override;
  int getRoots () override;
  int setRootFinding (count_t numRoots) override;

  void logSolverStats (int logLevel, bool iconly = false) const override;
  void logErrorWeights (int logLevel) const override;
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
/** @brief solverInterface interfacing to the sundials cvode solver
*/
class cvodeInterface : public solverInterface
{
public:
  count_t icCount = 0;
private:
  arrayDataSparse a1;                                                                                                           //!< array structure for holding the jacobian information
  N_Vector state = nullptr;                                                        //!< state vector
  N_Vector deriv = nullptr;                                                         //!< derivative vector
  N_Vector abstols = nullptr;                                                     //!< tolerance vector
  N_Vector types = nullptr;                                                       //!< inequalities and state types
  N_Vector consData = nullptr;                                                     //!<constraint type Vector
  N_Vector scale = nullptr;
  N_Vector eweight = nullptr;
  N_Vector ele = nullptr;
  std::vector<double> tempState;                                                //!<temporary holding location for a state vector
  bool use_bdf = false;
  bool use_newton = false;
public:
  /** @brief constructor*/
  cvodeInterface ();
  /** @brief alternate constructor
  @param[in] gds  the gridDynSimulation object to connect to
  @param[in] sMode the solverMode to solve For
  */
  cvodeInterface (gridDynSimulation *gds, const solverMode& sMode);
  /** @brief destructor*/
  ~cvodeInterface ();

  double * state_data () override;
  double * deriv_data () override;
  double * type_data () override;

  int allocate (count_t size, count_t numroots = 0) override;
  int initialize (double t0) override;
  void setMaxNonZeros (count_t size) override;
  int sparseReInit (sparse_reinit_modes mode) override;
  int getCurrentData () override;
  int solve (double tStop, double &tReturn, step_mode stepMode = step_mode::normal) override;
  int getRoots () override;
  int setRootFinding (count_t numRoots) override;

  void logSolverStats (int logLevel, bool iconly = false) const override;
  void logErrorWeights (int logLevel) const override;
  virtual int set (const std::string &param, const std::string &val) override;
  virtual int set (const std::string &param, double val) override;
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

/** @brief solverInterface interfacing to the sundials arkode solver
*/
class arkodeInterface : public solverInterface
{
public:
  count_t icCount = 0;
private:
  arrayDataSparse a1;                                                                                                           //!< array structure for holding the jacobian information
  N_Vector state = nullptr;                                                              //!< state vector
  N_Vector deriv = nullptr;                                                               //!< derivative vector
  N_Vector abstols = nullptr;                                                           //!< tolerance vector
  N_Vector types = nullptr;                                                             //!< inequalities and state types
  N_Vector consData = nullptr;                                                           //!<constraint type Vector
  N_Vector scale = nullptr;
  N_Vector eweight = nullptr;
  N_Vector ele = nullptr;
  std::vector<double> tempState;                                                      //!<temporary holding location for a state vector
  bool use_bdf = false;
  bool use_newton = false;
public:
  /** @brief constructor*/
  arkodeInterface ();
  /** @brief alternate constructor
  @param[in] gds  the gridDynSimulation object to connect to
  @param[in] sMode the solverMode to solve For
  */
  arkodeInterface (gridDynSimulation *gds, const solverMode& sMode);
  /** @brief destructor*/
  ~arkodeInterface ();
  double * state_data () override;
  double * deriv_data () override;
  double * type_data () override;

  int allocate (count_t size, count_t numroots = 0) override;
  int initialize (double t0) override;
  void setMaxNonZeros (count_t size) override;
  int sparseReInit (sparse_reinit_modes sparseReinitMode) override;
  int getCurrentData () override;
  int solve (double tStop, double &tReturn, step_mode stepMode = step_mode::normal) override;
  int getRoots () override;
  int setRootFinding (count_t numRoots) override;

  void logSolverStats (int logLevel, bool iconly = false) const override;
  void logErrorWeights (int logLevel) const override;
  virtual int set (const std::string &param, const std::string &val) override;
  virtual int set (const std::string &param, double val) override;
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


