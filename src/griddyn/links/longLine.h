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

#ifndef LONG_LINE_H_
#define LONG_LINE_H_

#include "links/subsystem.h"
#include "links/acLine.h"

namespace griddyn
{
namespace links
{
/** @brief class defining a long line model
 the model splits the line into a number of short line segments with buses in between
*/
class longLine : public subsystem
{
protected:

  double segmentationLength = 50;		//!< [km] the length of each segment
  double fault = -1.0;					//!< fault location along the line keep at <0 for no fault
  double r = 0.0;						//!< [puOhm] per unit resistance
  double x = 0.0;						//!< [puOhm] per unit reactance
  double mp_G = 0.0;					//!< [puMW] per unit shunt conductance (g/2 on each end of the line)
  double mp_B = 0.0;					//!< [puMW] per unit shunt capacitance (jb/2 on each end of the line)
  double length = 0.0;					//!< [km] transmission line length
private:
  int faultLink = -1;  //!< link number of the fault if one is present
public:
  /** @brief default constructor*/
  longLine (const std::string &objName = "longLine_$" );
  virtual coreObject * clone (coreObject *obj = nullptr) const override;
  // add components
  virtual void add (coreObject *obj) override final;  //there shouldn't be any additional adds
  // remove components
  virtual void remove (coreObject *obj) override final; //there shouldn't be any removes all models are controlled internally

  virtual void pFlowObjectInitializeA (coreTime time0, std::uint32_t flags) override;

  virtual void set (const std::string &param,  const std::string &val) override;
  virtual void set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;

  virtual double get (const std::string &param, gridUnits::units_t unitType = gridUnits::defUnit) const override;
private:
  /** @brief helper function to generate all the internal links*/
  void generateIntermediateLinks ();
};

}//namespace links
}//namespace griddyn
#endif
