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

#ifndef OTHERGENMODEL_H_
#define OTHERGENMODEL_H_

#include "submodels/gridDynGenModel.h"
#include "saturation.h"

/** @brief model simulation implementing a simple inverter model
 the gridDynGenModel implements a very basic inverter model modeling the generator as a transmission line
with very fast angle adjustments to keep the mechanical input power balanced
*/
class gridDynGenModelInverter : public gridDynGenModel
{
public:
protected:
  double maxAngle = 89.0 * kPI / 180.0;      //!< maximum firing angle
  double minAngle = -89.0 * kPI / 180.0;      //!< minimum firing angle
  gridBus *bus = nullptr;                     //!< reference to the connected bus;
public:
  //!< @brief default constructor
  gridDynGenModelInverter (const std::string &objName = "genModel_#");
  virtual gridCoreObject * clone (gridCoreObject *obj = nullptr) const override;
  //!< @brief virtual destructor
  virtual ~gridDynGenModelInverter ();
  virtual void objectInitializeA (double time0, unsigned long flags) override;
  virtual void objectInitializeB (const IOdata &args, const IOdata &outputSet, IOdata &inputSet) override;

  virtual int set (const std::string &param, const std::string &val) override;
  virtual int set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;

  virtual stringVec localStateNames () const override;
  // dynamics

  virtual void residual (const IOdata &args, const stateData *sD, double resid[], const solverMode &sMode) override;

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
  virtual void rootTest (const IOdata &args, const stateData *sD, double roots[], const solverMode &sMode) override;
  virtual void rootTrigger (double ttime, const IOdata &args, const std::vector<int> &rootMask, const solverMode &sMode) override;
  virtual change_code rootCheck (const IOdata &args, const stateData *sD, const solverMode &sMode, check_level_t level) override;
private:
  void reCalcImpedences ();
  /** @brief compute the real power output
  @param V voltage
  @param Ef Exciter field
  @param cosA  the cosine of the power angle
  @param sinA  the sine of the power angle
  @return the real power output;
  */
  double realPowerCompute (double V, double Ef, double cosA, double sinA) const;
  /** @brief compute the reactive power output
  @param V voltage
  @param Ef Exciter field
  @param cosA  the cosine of the power angle
  @param sinA  the sine of the power angle
  @return the real power output;
  */
  double reactivePowerCompute (double V, double Ef, double cosA, double sinA) const;
  double g = 0;
  double b = (1.0 / 1.05);
};


class gridDynGenModel3 : public gridDynGenModelClassical
{
protected:
  double Xl = 0.0;                                //!< [p.u.] leakage reactance
  double Xdp = 0.35;                       //!< [p.u.] d-axis transient reactance
  double E = 0;                                         //!< constant Eb' field
  double Xq = 0.85;                  //!< [p.u.] q-axis reactance
  double Tdop = 8.0;                //!< [s]    d-axis time constant

public:
  gridDynGenModel3 (const std::string &objName = "genModel3_#");
  virtual gridCoreObject * clone (gridCoreObject *obj = nullptr) const override;
  virtual void objectInitializeA (double time0, unsigned long flags) override;
  virtual void objectInitializeB (const IOdata &args, const IOdata &outputSet, IOdata &inputSet) override;

  virtual int set (const std::string &param, const std::string &val) override;
  virtual int set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;

  virtual stringVec localStateNames () const override;
  // dynamics
  virtual void residual (const IOdata &args, const stateData *sD, double resid[], const solverMode &sMode) override;
  virtual void derivative (const IOdata &args, const stateData *sD, double deriv[], const solverMode &sMode) override;
  virtual void jacobianElements (const IOdata &args, const stateData *sD, arrayData<double> *ad, const IOlocs &argLocs, const solverMode &sMode) override;
  virtual void algebraicUpdate (const IOdata &args, const stateData *sD, double update[], const solverMode &sMode, double alpha) override;
};

class gridDynGenModel4 : public gridDynGenModel3
{

protected:
  double Xqp = 0.35;               //!< [p.u.] q-axis transient reactance
  double Tqop = 1.0;                //!< [s]    q-axis time constant
  double S10 = 1.0;           //!< the saturation S (1.0) const
  double S12 = 1.0;                    //!< the saturation S(1.2)
  saturation sat;                     //!< saturation function object
public:
  gridDynGenModel4 (const std::string &objName = "genModel4_#");
  virtual gridCoreObject * clone (gridCoreObject *obj = nullptr) const override;
  virtual void objectInitializeA (double time0, unsigned long flags) override;
  virtual void objectInitializeB (const IOdata &args, const IOdata &outputSet, IOdata &inputSet) override;

  virtual int set (const std::string &param, const std::string &val) override;
  virtual int set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;

  virtual stringVec localStateNames () const override;
  // dynamics
  virtual double timestep (double ttime, const IOdata &args, const solverMode &sMode) override;
  virtual void residual (const IOdata &args, const stateData *sD, double resid[], const solverMode &sMode) override;
  virtual void derivative (const IOdata &args, const stateData *sD, double deriv[], const solverMode &sMode) override;
  virtual void jacobianElements (const IOdata &args, const stateData *sD, arrayData<double> *ad, const IOlocs &argLocs, const solverMode &sMode) override;

  virtual void algebraicUpdate  (const IOdata &args, const stateData *sD, double update[], const solverMode &sMode, double alpha) override;
};

class gridDynGenModel5 : public gridDynGenModel4
{

protected:
  double Tqopp = 0.1;
  double Taa = 0.0;
  double Tdopp = 0.8;
  double Xdpp = 0.175;
  double Xqpp = 0.175;
public:
  gridDynGenModel5 (const std::string &objName = "genModel5_#");
  virtual gridCoreObject * clone (gridCoreObject *obj = nullptr) const override;
  virtual void objectInitializeA (double time0, unsigned long flags) override;
  virtual void objectInitializeB (const IOdata &args, const IOdata &outputSet, IOdata &inputSet) override;

  virtual int set (const std::string &param, const std::string &val) override;
  virtual int set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;

  virtual stringVec localStateNames () const override;
  // dynamics
  virtual void residual (const IOdata &args, const stateData *sD, double resid[], const solverMode &sMode) override;
  virtual void derivative (const IOdata &args, const stateData *sD, double deriv[], const solverMode &sMode) override;
  virtual void jacobianElements (const IOdata &args, const stateData *sD,
                                 arrayData<double> *ad,
                                 const IOlocs &argLocs, const solverMode &sMode) override;
  virtual void algebraicUpdate (const IOdata &args, const stateData *sD, double update[], const solverMode &sMode, double alpha) override;

};

class gridDynGenModel5type2 : public gridDynGenModel5
{


protected:
public:
  gridDynGenModel5type2  (const std::string &objName = "genModel5type2_#");
  virtual gridCoreObject * clone (gridCoreObject *obj = nullptr) const override;
  virtual void objectInitializeA (double time0, unsigned long flags) override;
  virtual void objectInitializeB (const IOdata &args, const IOdata &outputSet, IOdata &inputSet) override;

  virtual stringVec localStateNames () const override;
  // dynamics
  virtual void residual (const IOdata &args, const stateData *sD, double resid[], const solverMode &sMode) override;
  virtual void derivative (const IOdata &args, const stateData *sD, double deriv[], const solverMode &sMode) override;
  virtual void jacobianElements (const IOdata &args, const stateData *sD,
                                 arrayData<double> *ad,
                                 const IOlocs &argLocs, const solverMode &sMode) override;
  virtual void algebraicUpdate (const IOdata &args, const stateData *sD, double update[], const solverMode &sMode, double alpha) override;
};

class gridDynGenModel5type3 : public gridDynGenModel3
{

protected:
public:
  gridDynGenModel5type3  (const std::string &objName = "genModel5type3_#");
  virtual gridCoreObject * clone (gridCoreObject *obj = nullptr) const override;
  virtual void objectInitializeA (double time0, unsigned long flags) override;
  virtual void objectInitializeB (const IOdata &args, const IOdata &outputSet, IOdata &inputSet) override;

  virtual stringVec localStateNames () const override;
  // dynamics
  virtual void residual (const IOdata &args, const stateData *sD, double resid[], const solverMode &sMode) override;
  virtual void derivative (const IOdata &args, const stateData *sD, double deriv[], const solverMode &sMode) override;
  virtual void jacobianElements (const IOdata &args, const stateData *sD,
                                 arrayData<double> *ad,
                                 const IOlocs &argLocs, const solverMode &sMode) override;
};
class gridDynGenModel6 : public gridDynGenModel5
{

protected:
public:
  gridDynGenModel6  (const std::string &objName = "genModel6_#");
  virtual gridCoreObject * clone (gridCoreObject *obj = nullptr) const override;
  virtual void objectInitializeA (double time0, unsigned long flags) override;
  virtual void objectInitializeB (const IOdata &args, const IOdata &outputSet, IOdata &inputSet) override;

  virtual stringVec localStateNames () const override;
  // dynamics
  virtual void residual (const IOdata &args, const stateData *sD, double resid[], const solverMode &sMode) override;
  virtual void derivative (const IOdata &args, const stateData *sD, double deriv[], const solverMode &sMode) override;
  virtual void jacobianElements (const IOdata &args, const stateData *sD,
                                 arrayData<double> *ad,
                                 const IOlocs &argLocs, const solverMode &sMode) override;
  virtual void algebraicUpdate  (const IOdata &args, const stateData *sD, double update[], const solverMode &sMode, double alpha) override;
};

class gridDynGenModel6type2 : public gridDynGenModel5type2
{

protected:
public:
  gridDynGenModel6type2 (const std::string &objName = "genModel6type2_#");
  virtual gridCoreObject * clone (gridCoreObject *obj = nullptr) const override;
  virtual void objectInitializeA (double time0, unsigned long flags) override;
  virtual void objectInitializeB (const IOdata &args, const IOdata &outputSet, IOdata &inputSet) override;

  virtual stringVec localStateNames () const override;
  // dynamics
  virtual void residual (const IOdata &args, const stateData *sD, double resid[], const solverMode &sMode) override;
  virtual void derivative (const IOdata &args, const stateData *sD, double deriv[], const solverMode &sMode) override;
  virtual void jacobianElements (const IOdata &args, const stateData *sD,
                                 arrayData<double> *ad,
                                 const IOlocs &argLocs, const solverMode &sMode) override;
  virtual void algebraicUpdate  (const IOdata &args, const stateData *sD, double update[], const solverMode &sMode, double alpha) override;
};

class gridDynGenModelGENROU : public gridDynGenModel5
{

protected:
public:
  gridDynGenModelGENROU (const std::string &objName = "genrou_#");
  virtual gridCoreObject * clone (gridCoreObject *obj = nullptr) const override;
  virtual void objectInitializeA (double time0, unsigned long flags) override;
  virtual void objectInitializeB (const IOdata &args, const IOdata &outputSet, IOdata &inputSet) override;

  virtual stringVec localStateNames () const override;
  // dynamics
  virtual void residual (const IOdata &args, const stateData *sD, double resid[], const solverMode &sMode) override;
  virtual void derivative (const IOdata &args, const stateData *sD, double deriv[], const solverMode &sMode) override;
  virtual void jacobianElements (const IOdata &args, const stateData *sD,
                                 arrayData<double> *ad,
                                 const IOlocs &argLocs, const solverMode &sMode) override;
  virtual void algebraicUpdate  (const IOdata &args, const stateData *sD, double update[], const solverMode &sMode, double alpha) override;
};

class gridDynGenModel8 : public gridDynGenModel6
{

protected:
public:
  gridDynGenModel8 (const std::string &objName = "genModel8_#");
  virtual gridCoreObject * clone (gridCoreObject *obj = nullptr) const override;
  virtual void objectInitializeA (double time0, unsigned long flags) override;
  virtual void objectInitializeB (const IOdata &args, const IOdata &outputSet, IOdata &inputSet) override;

  virtual stringVec localStateNames () const override;
  // dynamics
  virtual void residual (const IOdata &args, const stateData *sD, double resid[], const solverMode &sMode) override;
  virtual void derivative (const IOdata &args, const stateData *sD, double deriv[], const solverMode &sMode) override;
  virtual void jacobianElements (const IOdata &args, const stateData *sD,
                                 arrayData<double> *ad,
                                 const IOlocs &argLocs, const solverMode &sMode ) override;
};


#endif //GRIDDYNGENMODEL_H_
