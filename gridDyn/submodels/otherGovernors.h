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

#ifndef GRIDDYN_OTHER_GOVERNORS_H_
#define GRIDDYN_OTHER_GOVERNORS_H_

#include "submodels/gridDynGovernor.h"

class gridDynGovernorIeeeSimple : public gridDynGovernor
{
public:
protected:
  double T3;                  //!< [s]    servo motor time constant
  double Pup;                 //!< [pu] upper ramp limit
  double Pdown;               //!< [pu] lower ramp limit
public:
  gridDynGovernorIeeeSimple (const std::string &objName = "govIeeeSimple_#");
  virtual gridCoreObject * clone (gridCoreObject *obj = nullptr) const override;
  virtual ~gridDynGovernorIeeeSimple ();
  virtual void objectInitializeA (double time0, unsigned long flags) override;
  virtual void objectInitializeB (const IOdata &args, const IOdata &outputSet, IOdata &inputSet) override;

  virtual void set (const std::string &param, const std::string &val) override;
  virtual void set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;
  virtual index_t findIndex (const std::string &field, const solverMode &sMode) const override;
  virtual void residual (const IOdata &args, const stateData *sD, double resid[], const solverMode &sMode) override;
  virtual void derivative (const IOdata &args, const stateData *sD, double deriv[], const solverMode &sMode) override;
  virtual void jacobianElements (const IOdata &args, const stateData *sD,
                                 matrixData<double> *ad,
                                 const IOlocs &argLocs, const solverMode &sMode) override;
  virtual void timestep  (double ttime, const IOdata &args, const solverMode &sMode) override;
  virtual void rootTest (const IOdata &args, const stateData *sD, double roots[], const solverMode &sMode) override;
  virtual void rootTrigger (double ttime, const IOdata &args, const std::vector<int> &rootMask, const solverMode &sMode) override;
  //virtual void setTime(double time){prevTime=time;};
};

class gridDynGovernorReheat : public gridDynGovernor
{

public:
protected:
  double T3;                  //!< [s]    Transient gain time constant
  double T4;                          //!< [s]    Power fraction time constant
  double T5;                          //!< [s]    Reheat time constant
public:
  gridDynGovernorReheat (const std::string &objName = "govReheat_#");
  virtual gridCoreObject * clone (gridCoreObject *obj = nullptr) const override;
  virtual ~gridDynGovernorReheat ();
  virtual void objectInitializeB (const IOdata &args, const IOdata &outputSet, IOdata &inputSet) override;
  virtual void set (const std::string &param, const std::string &val) override;
  virtual void set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;
  virtual index_t findIndex (const std::string &field, const solverMode &sMode) const override;
  virtual void residual (const IOdata &args, const stateData *sD, double resid[], const solverMode &sMode) override;
  virtual void derivative (const IOdata &args, const stateData *sD, double deriv[], const solverMode &sMode) override;
  virtual void jacobianElements (const IOdata &args, const stateData *sD,
                                 matrixData<double> *ad,
                                 const IOlocs &argLocs, const solverMode &sMode) override;

  //virtual void setTime (double time) const{prevTime=time;};
};


class gridDynGovernorTgov1 : public gridDynGovernorIeeeSimple
{

public:
protected:
  double Dt = 0.0;              //!<speed damping constant
public:
  gridDynGovernorTgov1 (const std::string &objName = "govTgov1_#");
  virtual gridCoreObject * clone (gridCoreObject *obj = nullptr) const override;
  virtual ~gridDynGovernorTgov1 ();
  virtual void objectInitializeB (const IOdata &args, const IOdata &outputSet, IOdata &inputSet) override;

  virtual void set (const std::string &param, const std::string &val) override;
  virtual void set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;
  virtual index_t findIndex (const std::string &field, const solverMode &sMode) const override;

  virtual void residual (const IOdata &args, const stateData *sD, double resid[], const solverMode &sMode) override;
  virtual void derivative (const IOdata &args, const stateData *sD, double deriv[], const solverMode &sMode) override;
  virtual void jacobianElements (const IOdata &args, const stateData *sD,
                                 matrixData<double> *ad, const IOlocs &argLocs, const solverMode &sMode) override;
  virtual void timestep  (double ttime, const IOdata &args, const solverMode &sMode) override;
  virtual void rootTest (const IOdata &args, const stateData *sD, double roots[], const solverMode &sMode) override;
  virtual void rootTrigger (double ttime, const IOdata &args, const std::vector<int> &rootMask, const solverMode &sMode) override;

};

class gridDynGovernorHydro : public gridDynGovernorIeeeSimple
{

public:
protected:
  double Tw;              //!< [s] spill tube time constant
public:
  gridDynGovernorHydro (const std::string &objName = "govHydro_#");
  virtual gridCoreObject * clone (gridCoreObject *obj = nullptr) const override;
  virtual ~gridDynGovernorHydro ();

  virtual void objectInitializeB (const IOdata &args, const IOdata &outputSet, IOdata &inputSet) override;

  virtual void set (const std::string &param, const std::string &val) override;
  virtual void set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;
  virtual index_t findIndex (const std::string &field, const solverMode &sMode) const override;


  virtual void residual (const IOdata &args, const stateData *sD, double resid[], const solverMode &sMode) override;
  //only called if the genModel is not present

  virtual void jacobianElements (const IOdata &args, const stateData *sD,
                                 matrixData<double> *ad,
                                 const IOlocs &argLocs, const solverMode &sMode) override;

};

class gridDynGovernorSteamNR : public gridDynGovernorIeeeSimple
{

public:
protected:
  double Tch;             //!< [s] steam reheat chest time constant
public:
  gridDynGovernorSteamNR  (const std::string &objName = "govSteamNR_#");
  virtual gridCoreObject * clone (gridCoreObject *obj = nullptr) const override;
  virtual ~gridDynGovernorSteamNR ();
  virtual void objectInitializeB (const IOdata &args, const IOdata &outputSet, IOdata &inputSet) override;

  virtual void set (const std::string &param, const std::string &val) override;
  virtual void set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;
  virtual index_t findIndex (const std::string &field, const solverMode &sMode) const override;


  virtual void residual (const IOdata &args, const stateData *sD, double resid[], const solverMode &sMode) override;
  //only called if the genModel is not present
  virtual void jacobianElements (const IOdata &args, const stateData *sD,
                                 matrixData<double> *ad,
                                 const IOlocs &argLocs, const solverMode &sMode) override;

};

class gridDynGovernorSteamTCSR : public gridDynGovernorSteamNR
{

public:
protected:
  double Trh;              //!< [s] steam reheat chest time constant
  double  Tco;              //!< [s] steam reheat chest time constant
  double  Fch;              //!< [s] steam reheat chest time constant
  double  Fip;             //!< [s] steam reheat chest time constant
  double  Flp;             //!< [s] steam reheat chest time constant
public:
  gridDynGovernorSteamTCSR  (const std::string &objName = "govSteamTCSR_#");
  virtual gridCoreObject * clone (gridCoreObject *obj = nullptr) const override;
  virtual ~gridDynGovernorSteamTCSR ();

  virtual void objectInitializeB (const IOdata &args, const IOdata &outputSet, IOdata &inputSet) override;

  virtual void set (const std::string &param, const std::string &val) override;
  virtual void set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;
  virtual index_t findIndex (const std::string &field, const solverMode &sMode) const override;

  virtual void residual (const IOdata &args, const stateData *sD, double resid[], const solverMode &sMode) override;

  virtual void jacobianElements (const IOdata &args, const stateData *sD,
                                 matrixData<double> *ad,
                                 const IOlocs &argLocs, const solverMode &sMode ) override;

};



#endif
