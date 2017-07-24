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

#ifndef GRIDDYN_EXPORT_INTERNAL_H_
#define GRIDDYN_EXPORT_INTERNAL_H_
#include "gridComponent.h"
#include "solvers/solverMode.hpp"
typedef void * gridDynObject;
gridDynObject creategridDynObject(griddyn::gridComponent *comp);
griddyn::gridComponent *getComponentPointer(gridDynObject obj);

class solverKeyInfo
{
public:
	griddyn::solverMode sMode_; //!< solverMode
	std::vector<double> stateBuffer;
	std::vector<double> dstateBuffer;
	std::vector<std::string> stateNames;
	solverKeyInfo() {};
	solverKeyInfo(griddyn::solverMode sMode) :sMode_(sMode) {};
};

void setUpSolverKeyInfo(solverKeyInfo *key, griddyn::gridComponent *comp);

void TranslateToLocal(const std::vector<double> &orig, double *newData, const griddyn::gridComponent *comp, const griddyn::solverMode &sMode);
void CopyFromLocal(std::vector<double> &dest, const double *localData, const griddyn::gridComponent *comp, const griddyn::solverMode &sMode);
#endif
