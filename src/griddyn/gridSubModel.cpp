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

#include "gridSubModel.h"

#include "measurement/objectGrabbers.h"

namespace griddyn {
gridSubModel::gridSubModel(const std::string& objName): gridComponent(objName)
{
    opFlags.set(no_powerflow_operations);
    m_outputSize = 1;
}

void gridSubModel::pFlowInitializeA(coreTime time, std::uint32_t flags)
{
    gridComponent::pFlowInitializeA(time, flags);
}

void gridSubModel::pFlowInitializeB()
{
    gridComponent::pFlowInitializeB();
}
void gridSubModel::dynInitializeA(coreTime time, std::uint32_t flags)
{
    if (isEnabled()) {
        dynObjectInitializeA(time, flags);

        auto& so = offsets.getOffsets(cLocalSolverMode);
        if (getSubObjects().empty()) {
            so.localLoadAll(true);
        } else {
            loadStateSizes(cLocalSolverMode);
        }

        so.setOffset(0);
        prevTime = time;
        updateFlags(true);
        setupDynFlags();
    }
}

void gridSubModel::dynInitializeB(const IOdata& inputs,
                                  const IOdata& desiredOutput,
                                  IOdata& fieldSet)
{
    if (isEnabled()) {
        // make sure the state vectors are sized properly
        auto ns = offsets.local().local.totalSize();
        m_state.resize(ns, 0);
        m_dstate_dt.clear();
        m_dstate_dt.resize(ns, 0);

        dynObjectInitializeB(inputs, desiredOutput, fieldSet);
        if (updatePeriod < maxTime) {
            enable_updates();
            setUpdateTime(prevTime + updatePeriod);
            alert(this, UPDATE_REQUIRED);
        }
        opFlags.set(dyn_initialized);
    }
}

double gridSubModel::get(const std::string& param, units::unit unitType) const
{
    auto fptr = getObjectFunction(this, param);
    if (fptr.first) {
        coreObject* tobj = const_cast<gridSubModel*>(this);
        return convert(fptr.first(tobj), fptr.second, unitType, systemBasePower);
    }

    return gridComponent::get(param, unitType);
}
}  // namespace griddyn
