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

#ifndef GRIDDYNSTABILIZER_H_
#define GRIDDYNSTABILIZER_H_

#include "gridSubModel.h"

namespace griddyn
{
class Stabilizer : public gridSubModel
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
  explicit Stabilizer (const std::string &objName = "pss_#");
  virtual coreObject * clone (coreObject *obj = nullptr) const override;
  virtual ~Stabilizer ();
  virtual void dynObjectInitializeB (const IOdata &inputs, const IOdata &desiredOutput, IOdata &fieldSet) override;

  virtual void set (const std::string &param,  const std::string &val) override;
  virtual void set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;

  virtual void residual (const IOdata &inputs, const stateData &sD, double resid[],  const solverMode &sMode) override;
  virtual void jacobianElements (const IOdata &inputs, const stateData &sD,
                                 matrixData<double> &md,
                                 const IOlocs &inputLocs, const solverMode &sMode) override;

  virtual void derivative  (const IOdata &inputs, const stateData &sD, double deriv[], const solverMode &sMode) override;

  virtual index_t findIndex (const std::string &field, const solverMode &sMode) const override;
};

}//namespace griddyn

#endif //GRIDDYNPSS_H_
