/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil;  c-set-offset 'innamespace 0; -*- */
/*
* LLNS Copyright Start
* Copyright (c) 2015, Lawrence Livermore National Security
* This work was performed under the auspices of the U.S. Department
* of Energy by Lawrence Livermore National Laboratory in part under
* Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
* Produced at the Lawrence Livermore National Laboratory.
* All rights reserved.
* For details, see the LICENSE file.
* LLNS Copyright End
*/

#ifndef GRID_HVDC_H_
#define GRID_HVDC_H_


#include "linkModels/subsystem.h"
/** @brief class defining a complete hvdc system including converters dc buses and dc line*/
class hvdc : public subsystem
{
public:
  /** hvdc helper flags*/
  enum hvdc_flags
  {
    reverse_flow = object_flag6,  //!< flag indicating that the flow is reverse standard
  };
protected:
public:
  /** @brief constructor*/
  hvdc (const std::string &objName = "hvdc_$");
  virtual gridCoreObject * clone (gridCoreObject *obj = nullptr) const override;
  // parameter set functions
  virtual int set (const std::string &param,  const std::string &val) override;
  virtual int set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;


  virtual double get (const std::string &param, gridUnits::units_t unitType = gridUnits::defUnit) const override;
  virtual int updateBus (gridBus *bus, index_t busnumber) override;
protected:
  static const int forward = 0;  //!< constant defining forward
  static const int reverse = 1;  //!< constant defining reverse
  /** @brief set the flow direction
  @param[in] direction  the direction of flow desired
  */
  void setFlow (int direction);
};

#endif
