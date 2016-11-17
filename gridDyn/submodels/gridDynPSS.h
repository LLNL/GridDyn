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

#ifndef GRIDDYNPSS_H_
#define GRIDDYNPSS_H_

#include "gridObjects.h"


class gridDynPSS : public gridSubModel
{
public:
protected:
  double mp_Tw;
  double mp_Teps;
  double mp_Kw;
  double mp_Kp;
  double mp_Kv;
  double mp_Smax;
  double mp_Smin;
public:
  explicit gridDynPSS (const std::string &objName = "pss_#");
  virtual gridCoreObject * clone (gridCoreObject *obj = nullptr) const override;
  virtual ~gridDynPSS ();
  virtual void objectInitializeB (const IOdata &args, const IOdata &outputSet, IOdata &inputSet) override;

  virtual void set (const std::string &param,  const std::string &val) override;
  virtual void set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;

  virtual void residual (const IOdata &args, const stateData *sD, double resid[],  const solverMode &sMode) override;
  virtual void jacobianElements (const IOdata &args, const stateData *sD,
                                 matrixData<double> &ad,
                                 const IOlocs &argLocs, const solverMode &sMode) override;

  virtual void derivative  (const IOdata &args, const stateData *sD, double deriv[], const solverMode &sMode) override;

  virtual index_t findIndex (const std::string &field, const solverMode &sMode) const override;
};


#endif //GRIDDYNPSS_H_
