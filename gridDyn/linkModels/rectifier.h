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

#ifndef GRID_DC_RECTIFIER_H_
#define GRID_DC_RECTIFIER_H_

#include "linkModels/gridLink.h"

class rectifier : public gridLink
{
  friend class gridBus;
  friend class gridSimulation;
  friend class gridDynSimulation;
public:
  /* enum rectifier_flags
   {
      fixed_target_power = object_flag5,
   };*/
protected:
  double alphaMax = kBigNum;                //!<max firing angle [rad]
  double alphaMin = -kBigNum;                //!<min firing angle [rad]

  double Idcmax = kBigNum;                //!<max rectifier reference current
  double Idcmin = -kBigNum;                //!<min rectifier reference current

  double mp_Ki = 0;                    //!<integral gain
  double mp_Kp = 1.0;                         //!<proportional gain

  double Idc;
public:
  rectifier (double rP, double xP);
  rectifier ();
  //gridLink(double max_power,gridBus *bus1, gridBus *bus2);
  virtual ~rectifier ();
  virtual gridCoreObject * clone (gridCoreObject *obj = nullptr, bool fullCopy = false);


  virtual double getMaxTransfer ();

  //virtual void pFlowCheck (std::vector<violation> &Violation_vector);
  virtual int updateBus (gridBus *bus, int busnumber);
  virtual void followNetwork (int network);
  virtual void computePower ();
  virtual void computePower (const stateData *sD, const solverMode &sMode);
  virtual void pFlowInitializeA (double time0, double abstime0, unsigned long flags);
  virtual void dynInitializeA (double time0, double abstime0, unsigned long flags);

  virtual void loadSizes (const solverMode &sMode, bool dynOnly);

  virtual double timestep (double ttime, const solverMode &sMode);

  virtual double quickupdateP ()
  {
    return 0;
  }

  virtual int set (std::string param, std::string val);
  virtual int set (std::string param, double val, gridUnits::units_t unitType = gridUnits::defUnit);

  //initializeB dynamics
  //virtual void dynInitializeA (double time0, double abstime0, unsigned long flags);
  virtual void ioPartialDerivatives (int busId, const stateData *sD, arrayData *ad, const std::vector<int> &argLocs, const solverMode &sMode);
  virtual void outputPartialDerivatives (int busId, const stateData *sD, arrayData *ad, const solverMode &sMode);

  void jacobianElements (const stateData *sD, arrayData *ad, const solverMode &sMode);
  void residual (const stateData *sD, double resid[], const solverMode &sMode);
  void setState (double ttime, const double state[], const double dstate_dt[], const solverMode &sMode);
  void guess (double ttime, double state[], double dstate_dt[], const solverMode &sMode);
  //for computing all the jacobian elements at once
  virtual void fixPower (double power, int terminal);

};


#endif
