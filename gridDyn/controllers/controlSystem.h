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

#ifndef CONTROLSYSTEM_H_
#define CONTROLSYSTEM_H_

#include "gridObjects.h"

#include "arrayDataSparse.h"

class basicBlock;

class controlSystem : public gridSubModel
{
protected:
  std::vector<basicBlock *> blocks;
  arrayDataSparse inputMult;
  arrayDataSparse outputMult;
  arrayDataSparse connections;

  std::vector<double> blockOutputs;

public:
  controlSystem (const std::string &objName = "control_system_#");
  virtual ~controlSystem ();

  virtual gridCoreObject * clone (gridCoreObject *obj = nullptr) const override;
  virtual void objectInitializeA (double time0, unsigned long flags) override;
  virtual void objectInitializeB (const IOdata &args, const IOdata &outputSet, IOdata &inputSet) override;

  virtual int add (gridCoreObject *obj) override;
  virtual int add (basicBlock *blk);

  virtual int set (const std::string &param, const std::string &val) override;
  virtual int set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;
  virtual index_t findIndex (const std::string &field, const solverMode &sMode) const override;

  virtual void residual (const IOdata &args, const stateData *sD, double resid[], const solverMode &sMode) override;

  virtual void jacobianElements (const IOdata &args, const stateData *sD,
                                 arrayData<double> *ad,
                                 const IOlocs &argLocs, const solverMode &sMode) override;

  virtual double timestep  (double ttime, const IOdata &args, const solverMode &sMode) override;

  virtual void rootTest (const IOdata &args, const stateData *sD, double roots[], const solverMode &sMode) override;
  virtual void rootTrigger (double ttime, const IOdata &args, const std::vector<int> &rootMask, const solverMode &sMode) override;
  virtual change_code rootCheck (const IOdata &args, const stateData *sD, const solverMode &sMode, check_level_t level) override;
  //virtual void setTime(double time){prevTime=time;};



};
#endif
