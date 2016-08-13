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

#ifndef LONG_LINE_H_
#define LONG_LINE_H_

#include "linkModels/subsystem.h"
#include "linkModels/acLine.h"
/** @brief class defining a long line model
 the model splits the line into a number of short line segments with buses in between
*/
class longLine : public subsystem
{
protected:
  double segmentationLength = 50;  //!< the length of each segment
  double fault = -1.0;
  double r = 0.0;
  double x = 0.0;
  double mp_G = 0.0;
  double mp_B = 0.0;
  double length = 0.0;
private:
  int faultLink = -1;  //!< link number of the fault if one is present
public:
  /** @brief default constructor*/
  longLine (const std::string &objName = "longLine_$" );
  virtual gridCoreObject * clone (gridCoreObject *obj = nullptr) const override;
  // add components
  virtual int add (gridCoreObject *obj) final override;  //there shouldn't be any additional adds
  // remove components
  virtual int remove (gridCoreObject *obj) final override; //there shouldn't be any removes all models are controlled internally

  virtual void pFlowObjectInitializeA (double time0, unsigned long flags) override;

  virtual int set (const std::string &param,  const std::string &val) override;
  virtual int set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;

  virtual double get (const std::string &param, gridUnits::units_t unitType = gridUnits::defUnit) const override;
private:
  /** @brief helper function to generate all the internal links*/
  void generateIntermediateLinks ();
};
#endif
