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

#ifndef LISTMAINTAINER_H_
#define LISTMAINTAINER_H_


#include <vector>
#include "gridObjectsHelperClasses.h"

template<class Y>
class matrixData;

namespace griddyn
{

class gridPrimary;
class stateData;

/** @brief helper class for areas to maintain lists of objects used for execution of each mode*/
class listMaintainer
{
public:
	bool parResid = false;
	bool parJac = false;
private:
  std::vector<gridPrimary *> preExObjs;          //!< lists of all the objects that request pre-execution
  std::vector< std::vector<gridPrimary *>> objectLists;        //!< lists of all the objects with states in a certain mode
  std::vector< std::vector<gridPrimary *>> partialLists;        //!< list of all the non preEx object with states in a certain mode
  std::vector< solverMode > sModeLists;			//!< the list of solverModes relevant to each list

public:
  listMaintainer ();

  void makePreList (const std::vector<gridPrimary *> &possObjs);
  void makeList (const solverMode &sMode, const std::vector<gridPrimary *> &possObjs);
  void appendList (const solverMode &sMode, const std::vector<gridPrimary *> &possObjs);

  //void listOperation(const solverMode &sMode);  //A work in progress

  void jacobianElements (const IOdata &inputs, const stateData &sD, matrixData<double> &md, const IOlocs &inputLocs, const solverMode &sMode);
  void preEx (const IOdata &inputs, const stateData &sD, const solverMode &sMode);
  void residual (const IOdata &inputs, const stateData &sD, double resid[], const solverMode &sMode);
  void algebraicUpdate (const IOdata &inputs, const stateData &sD, double update[], const solverMode &sMode, double alpha);
  void derivative (const IOdata &inputs, const stateData &sD, double deriv[], const solverMode &sMode);

  void delayedResidual (const IOdata &inputs, const stateData &sD, double resid[], const solverMode &sMode);
  void delayedDerivative (const IOdata &inputs, const stateData &sD, double deriv[], const solverMode &sMode);
  void delayedJacobian (const IOdata &inputs, const stateData &sD, matrixData<double> &md,const IOlocs &inputLocs, const solverMode &sMode);
  void delayedAlgebraicUpdate (const IOdata &inputs, const stateData &sD, double update[], const solverMode &sMode, double alpha);

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

}//namespace griddyn
#endif