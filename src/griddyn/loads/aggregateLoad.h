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

#ifndef COMPOSITELOAD_H_
#define COMPOSITELOAD_H_

#include "loads/zipLoad.h"
namespace griddyn
{
namespace loads
{
/** @brief class implementing a generic composite load
 the load can consume other loads already as part of a bus or have its own loads
*/
class aggregateLoad : public zipLoad
{
protected:
  bool consumeSimpleLoad = false;                       //!< flag indicating consumption of existing loads
  std::vector<zipLoad *> subLoads;                     //!< vector of subLoads
  std::vector<double> fraction;                         //!< the overall load fraction of each of the loads
private:
  // double sumFrac = 1.0;
public:
  //!< default constructor
  aggregateLoad (const std::string &objName = "compositeLoad_$");

  virtual coreObject * clone (coreObject *obj = nullptr) const override;
  virtual void pFlowObjectInitializeA (coreTime time0, std::uint32_t flags) override;
  virtual void pFlowObjectInitializeB () override;
  virtual void dynObjectInitializeA (coreTime time0, std::uint32_t flags) override;
  virtual void dynObjectInitializeB (const IOdata &inputs, const IOdata & desiredOutput, IOdata &fieldSet) override;

  virtual void set (const std::string &param,  const std::string &val) override;
  virtual void set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;

  virtual void add (zipLoad *ld);
  virtual void add (coreObject *obj) override;

  virtual void residual (const IOdata &inputs, const stateData &sD, double resid[], const solverMode &sMode) override;

  virtual void derivative (const IOdata &inputs, const stateData &sD, double deriv[], const solverMode &sMode) override;    //return D[0]=dP/dV D[1]=dP/dtheta,D[2]=dQ/dV,D[3]=dQ/dtheta

  virtual void outputPartialDerivatives (const IOdata &inputs, const stateData &sD, matrixData<double> &md, const solverMode &sMode) override;
  virtual void ioPartialDerivatives (const IOdata &inputs, const stateData &sD, matrixData<double> &md, const IOlocs &inputLocs, const solverMode &sMode) override;
  virtual void jacobianElements  (const IOdata &inputs, const stateData &sD, matrixData<double> &md, const IOlocs &inputLocs, const solverMode &sMode) override;

  virtual void timestep (coreTime time, const IOdata &inputs, const solverMode &sMode) override;

  virtual double getRealPower (const IOdata &inputs, const stateData &sD, const solverMode &sMode) const override;
  virtual double getReactivePower (const IOdata &inputs, const stateData &sD, const solverMode &sMode) const override;
  virtual double getRealPower (double V) const override;
  virtual double getReactivePower (double V) const override;
  virtual double getRealPower () const override;
  virtual double getReactivePower () const override;
};
}//namespace loads
}//namespace griddyn
#endif
