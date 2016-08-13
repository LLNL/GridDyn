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

#ifndef CONTINGENCY_H_
#define CONTINGENCY_H_


#include <string>
#include <vector>
#include <memory>

//limit violation definitions
#define NO_VIOLATION 0
#define VOLTAGE_OVER_LIMIT_VIOLATION 10
#define VOLTAGE_UNDER_LIMIT_VIOLATION 11
#define MVA_EXCEED_RATING_A 13
#define MVA_EXCEED_RATING_B 14
#define MVA_EXCEED_ERATING 15

#define MINIMUM_ANGLE_EXCEEDED 20
#define MAXIMUM_ANGLE_EXCEEDED 21
#define MINIMUM_CURRENT_EXCEEDED 30
#define MAXIMUM_CURRENT_EXCEEDED 31

class gridDynSimulation;

class violation
{
public:
  std::string m_objectName;        //the  name of the object with the violation
  double level;        //the value of the paremeter exceededing some limit
  double limit;        //the limit value
  double percentViolation;        //the violation percent;
  int contingency_id;        //usually added later or ignored
  int violationCode;      //a code representing the type of violation
  int severity = 0;       //a code indicating the severity of the violation
  violation (const std::string &name = "", int code = 0) : m_objectName (name), violationCode (code)
  {

  }
};

class gridEvent;

class contingency
{
public:
  static int contCount;         //static variable counting the number of created lines
  std::string  name;
  int id;
  std::vector < std::shared_ptr < gridEvent >> eventList;
  std::vector<violation> Violations;
  double PI = 0.0;     //performance index score
  double lowV = 0.0;     //minimum voltage
  std::vector<double> Vlist;
  std::vector<double> Lflow;
protected:
  gridDynSimulation *gds = nullptr;
public:
  contingency ();
  contingency (std::vector<char> &buffer);
  void runContingency ();
  void serialize (std::vector<char> &buffer);
  void setContingencyRoot (gridDynSimulation *gdSim);
};

#endif