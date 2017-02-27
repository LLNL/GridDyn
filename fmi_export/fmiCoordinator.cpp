/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil;  eval: (c-set-offset 'innamespace 0); -*- */
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

#include "fmiCoordinator.h"
#include "fmiEvent.h"
#include "fmiCollector.h"

#include <algorithm>

fmiCoordinator::fmiCoordinator(const std::string &) :coreObject("fmiCoordinator")
{

}

void fmiCoordinator::registerParameter(const std::string &paramName, fmiEvent *evnt)
{
	auto vr = nextVR++;
	paramVR.emplace_back(vr, inputSet{ paramName,evnt });

}

void fmiCoordinator::registerInput(const std::string &inputName, fmiEvent *evnt)
{
	auto vr = nextVR++;
	inputVR.emplace_back(vr, inputSet{inputName,evnt });
}



void fmiCoordinator::registerOutput(const std::string &outputName, int column, fmiCollector *out)
{
	auto vr = nextVR++;
	outputVR.emplace_back(vr, outputSet{ outputName,column, static_cast<index_t>(outputVR.size()),out });

	collectors.push_back(out);
	auto lst = std::unique(collectors.begin(), collectors.end());
	collectors.erase(lst, collectors.end());
	outputPoints.push_back(0.0);
}

void fmiCoordinator::sendInput(index_t vr, double val)
{
	auto res = std::lower_bound(inputVR.begin(), inputVR.end(), vrInputPair(vr, inputSet()), [](const auto &vp1, const auto &vp2) {return (vp1.first < vp2.first); });
	if ((res != inputVR.end()) && (res->first == vr))
	{
		res->second.evnt->setValue(val);
		res->second.evnt->trigger();
		return;
	}
	res = std::lower_bound(paramVR.begin(), paramVR.end(), vrInputPair(vr, inputSet()), [](const auto &vp1, const auto &vp2) {return (vp1.first < vp2.first); });
	if ((res != paramVR.end()) && (res->first == vr))
	{
		res->second.evnt->setValue(val);
		res->second.evnt->trigger();
		return;
	}
}

double fmiCoordinator::getOutput(index_t vr)
{
	auto res = std::lower_bound(outputVR.begin(), outputVR.end(), vrOutputPair(vr, outputSet()), [](const auto &vp1, const auto &vp2) {return (vp1.first < vp2.first); });
	if ((res!=outputVR.end())&&(res->first == vr))
	{
		return outputPoints[res->second.outIndex];
	}
	return kNullVal;
}


void fmiCoordinator::updateOutputs(coreTime time)
{
	for (auto col:collectors)
	{
		col->trigger(time);
	}
	for (auto &output : outputVR)
	{
		outputPoints[output.second.outIndex] = output.second.col->getValue(output.second.column);
	}
}

const std::string &fmiCoordinator::getFMIName() const
{
	return getParent()->getName();
}