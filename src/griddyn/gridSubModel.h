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

#ifndef GRIDSUBMODEL_H_
#define GRIDSUBMODEL_H_
#include "gridComponent.h"

namespace griddyn
{
/** @brief base class for any model can act as a component of another model
* gridSubModel class defines the interface for models which can act as components of other models such as Exciter, or Governor
most of the differential equations are contained in submodels.  The interface is meant to be flexible so unlike gridSecondary models there is no predefined interface, but at the same time
many of the function calls are intended to be the same,  The main difference being there is only one initialize function, they can operate in power flow but those objects just call initialize twice

**/
class gridSubModel : public gridComponent
{
protected:
  double m_output = 0.0;              //!< storage location for the current output
public:
  /** @brief default constructor*/
  explicit gridSubModel (const std::string &objName = "submodel_#");

  virtual void pFlowInitializeA(coreTime time, std::uint32_t flags) override final;

  virtual void pFlowInitializeB() override final;

  virtual void dynInitializeA (coreTime time, std::uint32_t flags) override final;
  
  virtual void dynInitializeB (const IOdata &inputs, const IOdata &desiredOutput, IOdata &fieldSet) override final;

  virtual double get(const std::string &param, gridUnits::units_t unitType = gridUnits::defUnit) const override;
};

}//namespace griddyn

#endif
