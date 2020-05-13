/*
 * LLNS Copyright Start
 * Copyright (c) 2014-2018, Lawrence Livermore National Security
 * This work was performed under the auspices of the U.S. Department
 * of Energy by Lawrence Livermore National Laboratory in part under
 * Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
 * Produced at the Lawrence Livermore National Laboratory.
 * All rights reserved.
 * For details, see the LICENSE file.
 * LLNS Copyright End
 */

#ifndef GRIDDYN_EXPORT_INTERNAL_H_
#define GRIDDYN_EXPORT_INTERNAL_H_
#include "griddyn/gridComponent.h"
#include "griddyn/solvers/solverMode.hpp"
typedef void* GridDynObject;
/** make a GridDynObject wrapper out of an actual component pointer*/
GridDynObject creategridDynObject(griddyn::gridComponent* comp);
/** get the component pointer from a GridDynObject*/
griddyn::gridComponent* getComponentPointer(GridDynObject obj);
/** get the const component pointer from a const GridDynObject*/
const griddyn::gridComponent* getConstComponentPointer(GridDynObject obj);

/** data class for storing some solver information and data buffers*/
class solverKeyInfo {
  public:
    griddyn::solverMode sMode_;  //!< solverMode
    std::vector<double> stateBuffer;  //!< buffer for storing state data
    std::vector<double> dstateBuffer;  //!< buffer for storing dstate_dt data
    std::vector<std::string> stateNames;  //!< buffer for storing the stateNames
    /** default constructor*/
    solverKeyInfo() = default;
    /** constructor from a solverMode reference*/
    solverKeyInfo(const griddyn::solverMode& sMode): sMode_(sMode){};
};

/** allocate buffers for using a solverKeyInfo object with a gridComponent*/
void setUpSolverKeyInfo(solverKeyInfo* key, griddyn::gridComponent* comp);
/** translate a system state vector to a local state vector*/
void TranslateToLocal(const std::vector<double>& orig,
                      double* newData,
                      const griddyn::gridComponent* comp,
                      const griddyn::solverMode& sMode);
/** translate a local state vector into the appropriate elements of a system state vector*/
void CopyFromLocal(std::vector<double>& dest,
                   const double* localData,
                   const griddyn::gridComponent* comp,
                   const griddyn::solverMode& sMode);
#endif
