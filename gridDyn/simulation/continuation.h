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

#ifndef CONTINUATION_H_
#define CONTINUATION_H_

#include "gridEvent.h"
#include "collector.h"
#include <string>
#include <vector>



class gridDynSimulation;

class parameterSequence
{
public:
  gridCoreObject *m_target;
  std::string m_field;
  double m_startValue;
  double m_stepSize;
  gridUnits::units_t m_unitType = gridUnits::defUnit;
protected:
  int m_currentStep = 0;
  gridCoreObject *m_obj = nullptr;
public:
  parameterSequence ();
  bool setTarget (gridCoreObject *gdo, const std::string var = "");
  void setValue (double start, double step) const;
  void step ();
  void step (int stepNumber);
};

class continuationSequence
{
public:
  static int contCount;           //static variable counting the number of created lines
  std::string  name;
  int id;
  std::vector < std::shared_ptr < parameterSequence >> SequenceList;         //!< vector storing sequence objects
  std::vector < std::shared_ptr < gridRecorder >> recordList;                 //!< vector storing recorder objects
protected:
  int m_currentStep = 0;
public:
  continuationSequence ();
  void step ();
  void step (int StepNumber);
};

#endif
