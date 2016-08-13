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

#ifndef GRIDDYNGENMODEL_H_
#define GRIDDYNGENMODEL_H_

#include "gridObjects.h"

const index_t genModelEftInLocation = 2;
const index_t genModelPmechInLocation = 3;
class gridBus;

/** @brief model simulation implementing a generator model into Griddyn
 the gridDynGenModel implements a very basic generator model with 4 states the real and reactive
currents as algebraic states and the generator rotational speed and angle otherwise known as the second order model
*/
class gridDynGenModel : public gridSubModel
{
public:
  /** @brief set of flags used by genModels for variations in computation
  */
  enum genModel_flags
  {

    use_saturation_flag = object_flag2, //!< flag indicating that the simulation should use a saturation model
    use_speed_field_adjustment = object_flag3,  //!< flag indicating that the simulation should use a speed field adjustment
    use_frequency_impedance_correction = object_flag4,  //!< flag indicating that the model should use frequency impedence corrections
    internal_frequency_calculation = object_flag5,
    at_angle_limits = object_flag6,
  };
protected:
  double machineBasePower = 100;  //!< [p.u.]  the operating base of the generator
  double Xd = 1.05;            //!< [p.u.] d-axis reactance
  double Rs = 0.0;            //!< [p.u.] generator resistance
  gridBus *bus = nullptr;               //!< reference to the connected bus;
public:
  //!< @brief default constructor
  gridDynGenModel (const std::string &objName = "genModel_#");
  virtual gridCoreObject * clone (gridCoreObject *obj = nullptr) const override;
  //!< @brief virtual destructor
  virtual ~gridDynGenModel ();

  virtual void objectInitializeB (const IOdata &args, const IOdata &outputSet,  IOdata &inputSet) override;

  virtual int set (const std::string &param,  const std::string &val) override;
  virtual int set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;

  virtual IOdata getOutputs (const IOdata &args, const stateData *sD, const solverMode &sMode) override;

  virtual double getOutput (const IOdata &args, const stateData *sD, const solverMode &sMode, index_t numOut = 0) const override;

  virtual void ioPartialDerivatives (const IOdata &args, const stateData *sD, arrayData<double> *ad, const IOlocs &argLocs, const solverMode &sMode) override;

  virtual double getFreq (const stateData *sD, const solverMode &sMode, index_t *FreqOffset = nullptr) const;
  virtual double getAngle (const stateData *sD, const solverMode &sMode, index_t *AngleOffset = nullptr) const;
};


class gridDynGenModelClassical : public gridDynGenModel
{
public:
  /** @brief set of flags used by genModels for variations in computation
  */

protected:
  double H = 5.0;                   //!< [p.u.] inertial constant
  double D = 0.04;                   //!< [p.u.] damping
  double Vd = 0;                              //!<the computed d axis voltage
  double Vq = 0;                              //!< the computed q axis voltage
  double mp_Kw = 13.0;               //!<speed gain for the damping system
  count_t seqId = 0;                  //!<the sequence Id the voltages were computed for
public:
  //!< @brief default constructor
  gridDynGenModelClassical (const std::string &objName = "genModelClassic_#");
  virtual gridCoreObject * clone (gridCoreObject *obj = nullptr) const override;
  //!< @brief virtual destructor
  virtual ~gridDynGenModelClassical ();
  virtual void objectInitializeA (double time0, unsigned long flags) override;
  virtual void objectInitializeB (const IOdata &args, const IOdata &outputSet, IOdata &inputSet) override;

  virtual int set (const std::string &param, const std::string &val) override;
  virtual int set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;

  virtual stringVec localStateNames () const override;
  // dynamics

  virtual void residual (const IOdata &args, const stateData *sD, double resid[], const solverMode &sMode) override;
  virtual void derivative (const IOdata &args, const stateData *sD, double deriv[], const solverMode &sMode) override;
  virtual IOdata getOutputs (const IOdata &args, const stateData *sD, const solverMode &sMode) override;

  virtual double getOutput (const IOdata &args, const stateData *sD, const solverMode &sMode, index_t numOut = 0) const override;

  virtual void jacobianElements (const IOdata &args, const stateData *sD, arrayData<double> *ad, const IOlocs &argLocs, const solverMode &sMode) override;
  virtual void outputPartialDerivatives (const IOdata &args, const stateData *sD, arrayData<double> *ad, const solverMode &sMode) override;
  virtual void ioPartialDerivatives (const IOdata &args, const stateData *sD, arrayData<double> *ad, const IOlocs &argLocs, const solverMode &sMode) override;

  virtual void algebraicUpdate (const IOdata &args, const stateData *sD, double update[], const solverMode &sMode, double alpha) override;
  /** helper function to get omega and its state location
  */
  virtual double getFreq (const stateData *sD, const solverMode &sMode, index_t *FreqOffset = nullptr) const override;
  virtual double getAngle (const stateData *sD, const solverMode &sMode, index_t *AngleOffset = nullptr) const override;
  virtual void updateLocalCache (const IOdata &args, const stateData *sD, const solverMode &sMode) override;
protected:
  void computeInitialAngleAndCurrent (const IOdata &args, const IOdata &outputSet, double R1, double X1);
};
#endif //GRIDDYNGENMODEL_H_
