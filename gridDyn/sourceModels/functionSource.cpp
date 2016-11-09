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

#include "sourceModels/otherSources.h"
#include "gridCoreTemplates.h"

functionSource::functionSource(const std::string &objName) :gridSource(objName)
{

}

gridCoreObject * functionSource::clone(gridCoreObject *obj) const
{
	functionSource *gS = cloneBase<functionSource, gridSubModel>(this, obj);
	if (gS == nullptr)
	{
		return obj;
	}
	gS->sourceFunc = sourceFunc;
	return gS;
}

IOdata functionSource::getOutputs(const IOdata & /*args*/, const stateData *sD, const solverMode &)
{
	return{ sourceFunc(sD->time) };
}
double functionSource::getOutput(const IOdata & /*args*/, const stateData *sD, const solverMode &, index_t num) const
{
	return (num == 0) ? sourceFunc(sD->time) : kNullVal;
}

double functionSource::getOutput(index_t num) const
{

	return (num == 0) ? sourceFunc(prevTime) : kNullVal;
}


double functionSource::getDoutdt(const stateData *sD, const solverMode &, index_t num)
{
	return  (num == 0) ? ((sourceFunc(sD->time + 1e-7) - sourceFunc(sD->time)) / 1e-7) : 0.0;
}

void functionSource::setFunction(std::function<double(double)> calcFunc)
{
	sourceFunc = calcFunc;
}