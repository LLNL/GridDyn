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

#ifndef LISTMAINTAINER_H_
#define LISTMAINTAINER_H_


#include <vector>

class gridPrimary;
class stateData;
/** @brief helper class for areas to maintain lists of objects used for execution of each mode*/
class listMaintainer
{
private:
  std::vector<gridPrimary *> preExObjs;          //!< lists of all the objects that request preexecution
  std::vector< std::vector<gridPrimary *>> objectLists;        //!< lists of all the objects with states in a certain mode
  std::vector< std::vector<gridPrimary *>> partialLists;        //!< list of all the non preex object with states in a certain mode
  std::vector< solverMode > sModeLists;			//!< the list of solverModes relevent to each list

public:
  listMaintainer ();

  void makePreList (const std::vector<gridPrimary *> &possObjs);
  void makeList (const solverMode &sMode, const std::vector<gridPrimary *> &possObjs);
  void appendList (const solverMode &sMode, const std::vector<gridPrimary *> &possObjs);

  //void listOperation(const solverMode &sMode);  //A work in progress

  void jacobianElements (const stateData *sD, matrixData<double> *ad, const solverMode &sMode);
  void preEx (const stateData *sD, const solverMode &sMode);
  void residual (const stateData *sD, double resid[], const solverMode &sMode);
  void algebraicUpdate (const stateData *sD, double update[], const solverMode &sMode, double alpha);
  void derivative (const stateData *sD, double deriv[], const solverMode &sMode);

  void delayedResidual (const stateData *sD, double resid[], const solverMode &sMode);
  void delayedDerivative (const stateData *sD, double deriv[], const solverMode &sMode);
  void delayedJacobian (const stateData *sD, matrixData<double> *ad, const solverMode &sMode);
  void delayedAlgebraicUpdate (const stateData *sD, double update[], const solverMode &sMode, double alpha);

  bool isListValid (const solverMode &sMode) const;
  void invalidate (const solverMode &sMode);
  void invalidate ();

  decltype(objectLists[0].cbegin ())cbegin (const solverMode &sMode) const;
  decltype(objectLists[0].cbegin ())cend (const solverMode &sMode) const;

  decltype(objectLists[0].begin ())begin (const solverMode &sMode);
  decltype(objectLists[0].begin ())end (const solverMode &sMode);

  decltype(objectLists[0].rbegin ())rbegin (const solverMode &sMode);
  decltype(objectLists[0].rbegin ())rend (const solverMode &sMode);
  
};


#endif