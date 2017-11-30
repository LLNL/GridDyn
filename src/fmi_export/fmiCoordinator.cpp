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
#include "utilities/mapOps.hpp"

namespace griddyn
{
namespace fmi
{
fmiCoordinator::fmiCoordinator(const std::string &) :coreObject("fmiCoordinator")
{

}

void fmiCoordinator::registerParameter(const std::string &paramName, fmiEvent *evnt)
{
	auto vr = nextVR++;
	paramVR.emplace_back(vr, inputSet{ paramName,evnt });
	vrNames.emplace(paramName, vr);

}

void fmiCoordinator::registerInput(const std::string &inputName, fmiEvent *evnt)
{
	auto vr = nextVR++;
	inputVR.emplace_back(vr, inputSet{inputName,evnt });
	vrNames.emplace(inputName, vr);
}



void fmiCoordinator::registerOutput(const std::string &outputName, int column, fmiCollector *out)
{
	auto vr = nextVR++;
	outputVR.emplace_back(vr, outputSet{ outputName,column, static_cast<index_t>(outputVR.size()),out });
	vrNames.emplace(outputName, vr);
	collectors.push_back(out);
	auto lst = std::unique(collectors.begin(), collectors.end());
	collectors.erase(lst, collectors.end());
	outputPoints.push_back(0.0);
}

bool fmiCoordinator::sendInput (index_t vr, double val)
{
    auto res = std::lower_bound (inputVR.begin (), inputVR.end (), vrInputPair (vr, inputSet ()),
                                 [](const auto &vp1, const auto &vp2) { return (vp1.first < vp2.first); });
    if ((res != inputVR.end ()) && (res->first == vr))
    {
        res->second.evnt->setValue (val);
        res->second.evnt->trigger ();
        return true;
    }
    res = std::lower_bound (paramVR.begin (), paramVR.end (), vrInputPair (vr, inputSet ()),
                            [](const auto &vp1, const auto &vp2) { return (vp1.first < vp2.first); });
    if ((res != paramVR.end ()) && (res->first == vr))
    {
        res->second.evnt->setValue (val);
        res->second.evnt->trigger ();
        return true;
    }
	LOG_WARNING("invalid value reference " + std::to_string(vr));
    return false;
}

index_t fmiCoordinator::findVR(const std::string &varName) const
{
	return mapFind(vrNames, varName, kNullLocation);
}


bool fmiCoordinator::sendInput(index_t vr, const char *s)
{
    auto res = std::lower_bound(paramVR.begin(), paramVR.end(), vrInputPair(vr, inputSet()),
        [](const auto &vp1, const auto &vp2) { return (vp1.first < vp2.first); });
    if ((res != paramVR.end()) && (res->first == vr)&&(res->second.evnt->eventType==fmi::fmiEvent::fmiEventType::string_parameter))
    {
        res->second.evnt->updateStringValue(s);
        res->second.evnt->trigger();
        return true;
    }
    LOG_WARNING("invalid value reference " + std::to_string(vr));
    return false;
}

double fmiCoordinator::getOutput (index_t vr)
{
    auto res = std::lower_bound (outputVR.begin (), outputVR.end (), vrOutputPair (vr, outputSet ()),
                                 [](const auto &vp1, const auto &vp2) { return (vp1.first < vp2.first); });
    if ((res != outputVR.end ()) && (res->first == vr))
    {
        return outputPoints[res->second.outIndex];
    }
    auto res2 = std::lower_bound (paramVR.begin (), paramVR.end (), vrInputPair (vr, inputSet ()),
                                  [](const auto &vp1, const auto &vp2) { return (vp1.first < vp2.first); });
    if ((res2 != paramVR.end ()) && (res2->first == vr))
    {
        return res2->second.evnt->query ();
    }
    auto res3 = std::lower_bound (inputVR.begin (), inputVR.end (), vrInputPair (vr, inputSet ()),
                                  [](const auto &vp1, const auto &vp2) { return (vp1.first < vp2.first); });
    if ((res3 != inputVR.end ()) && (res3->first == vr))
    {
        return res3->second.evnt->query ();
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

void fmiCoordinator::addHelper(std::shared_ptr<helperObject> ho)
{
	std::lock_guard<std::mutex> hLock(helperProtector);
	helpers.push_back(std::move(ho));
}

bool fmiCoordinator::isStringParameter(const vrInputPair &param)
{
    return (param.second.evnt->eventType == fmi::fmiEvent::fmiEventType::string_parameter);
}

}//namespace fmi
}//namespace griddyn