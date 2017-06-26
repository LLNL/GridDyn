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

#include "gridComponent.h"
#include "core/coreExceptions.h"
#include "core/coreObjectTemplates.hpp"
#include "core/objectInterpreter.h"
#include "utilities/matrixData.hpp"
#include "utilities/stackInfo.h"
#include "utilities/stringOps.h"
#include "utilities/vectorOps.hpp"
#include <cassert>
#include <cstdio>
#include <iostream>
#include <map>

namespace griddyn
{
// this functions is here since it depends on gridComponent information
Lp offsetTable::getLocations (const stateData &sD,
                              double d[],
                              const solverMode &sMode,
                              const gridComponent *comp) const
{
    Lp Loc = getLocations (sD, sMode,comp);
    if ((sMode.local) || (sD.empty ()))
    {
        Loc.destLoc = d;
        Loc.destDiffLoc = d + Loc.algSize;
    }
    else if (isDAE (sMode))
    {
        Loc.destLoc = d + Loc.algOffset;
        Loc.destDiffLoc = d + Loc.diffOffset;
    }
    else if (hasAlgebraic (sMode))
    {
        Loc.destLoc = d + Loc.algOffset;
        Loc.destDiffLoc = nullptr;
    }
    else if (hasDifferential (sMode))
    {
        Loc.destDiffLoc = d + Loc.diffOffset;
        Loc.destLoc = nullptr;
    }
    else
    {
        Loc.destLoc = d;
        Loc.destDiffLoc = d + Loc.algSize;
    }
    return Loc;
}

// this functions is here since it depends on gridComponent information
Lp offsetTable::getLocations (const stateData &sD, const solverMode &sMode, const gridComponent *comp) const
{
    Lp Loc;
    Loc.algOffset = offsetContainer[sMode.offsetIndex].algOffset;
    Loc.diffOffset = offsetContainer[sMode.offsetIndex].diffOffset;
    Loc.diffSize = offsetContainer[sMode.offsetIndex].total.diffSize;
    Loc.algSize = offsetContainer[sMode.offsetIndex].total.algSize;
    if ((sMode.local) || (sD.empty ()))
    {
        Loc.time = comp->prevTime;
        Loc.algStateLoc = comp->m_state.data ();
        Loc.diffStateLoc = comp->m_state.data () + Loc.algSize;
        Loc.dstateLoc = comp->m_dstate_dt.data () + Loc.algSize;
        if (Loc.algOffset == kNullLocation)
        {
            Loc.algOffset = 0;
        }
        if (Loc.diffOffset == kNullLocation)
        {
            Loc.diffOffset = Loc.algSize;
        }
    }
    else if (isDAE (sMode))
    {
        Loc.time = sD.time;
        Loc.algStateLoc = sD.state + Loc.algOffset;
        Loc.diffStateLoc = sD.state + Loc.diffOffset;
        Loc.dstateLoc = sD.dstate_dt + Loc.diffOffset;
    }
    else if (hasAlgebraic (sMode))
    {
        Loc.time = sD.time;
        if (sD.state != nullptr)
        {
            Loc.algStateLoc = sD.state + Loc.algOffset;
        }
        else
        {
            Loc.algStateLoc = sD.algState + Loc.algOffset;
        }
        if ((isDynamic (sMode)) && (sD.pairIndex != kNullLocation))
        {
            if (sD.diffState != nullptr)
            {
                Loc.diffStateLoc = sD.diffState + offsetContainer[sD.pairIndex].diffOffset;
            }
            else if (sD.fullState != nullptr)
            {
                Loc.diffStateLoc = sD.fullState + offsetContainer[sD.pairIndex].diffOffset;
            }

            if (sD.dstate_dt != nullptr)
            {
                Loc.dstateLoc = sD.dstate_dt + offsetContainer[sD.pairIndex].diffOffset;
            }
        }
        else
        {
            Loc.diffStateLoc = comp->m_state.data () + offsetContainer[0].diffOffset;
            Loc.dstateLoc = comp->m_dstate_dt.data () + offsetContainer[0].diffOffset;
        }
        Loc.destDiffLoc = nullptr;
    }
    else if (hasDifferential (sMode))
    {
        Loc.time = sD.time;
        if (sD.state != nullptr)
        {
            Loc.diffStateLoc = sD.state + Loc.diffOffset;
        }
        else
        {
            Loc.diffStateLoc = sD.diffState + Loc.diffOffset;
        }
        Loc.dstateLoc = sD.dstate_dt + Loc.diffOffset;
        if (sD.pairIndex != kNullLocation)
        {
            if (sD.algState != nullptr)
            {
                Loc.algStateLoc = sD.algState + offsetContainer[sD.pairIndex].algOffset;
            }
            else if (sD.fullState != nullptr)
            {
                Loc.algStateLoc = sD.fullState + offsetContainer[sD.pairIndex].algOffset;
            }
        }
        else
        {
            Loc.algStateLoc = comp->m_state.data () + offsetContainer[0].algOffset;
        }
        Loc.destLoc = nullptr;
    }
    else
    {
        Loc.time = comp->prevTime;
        Loc.algStateLoc = comp->m_state.data ();
        Loc.diffStateLoc = comp->m_state.data () + Loc.algSize;
        Loc.dstateLoc = comp->m_dstate_dt.data () + Loc.algSize;
        if (Loc.algOffset == kNullLocation)
        {
            Loc.algOffset = 0;
        }
        if (Loc.diffOffset == kNullLocation)
        {
            Loc.diffOffset = Loc.algSize;
        }
    }
    return Loc;
}

gridComponent::gridComponent (const std::string &objName) : coreObject (objName)
{
    offsets.setAlgOffset (0, cLocalSolverMode);
}

gridComponent::~gridComponent ()
{
    for (auto &so : subObjectList)
    {
        removeReference (so, this);
    }
}

coreObject *gridComponent::clone (coreObject *obj) const
{
    auto nobj = cloneBase<gridComponent, coreObject> (this, obj);
    if (nobj == nullptr)
    {
        return obj;
    }
    nobj->m_inputSize = m_inputSize;
    nobj->m_outputSize = m_outputSize;
    nobj->opFlags = opFlags;
    nobj->systemBaseFrequency = systemBaseFrequency;
    nobj->systemBasePower = systemBasePower;

    if (nobj->subObjectList.empty ())
    {
        for (const auto &subobj : subObjectList)
        {
            try
            {
                nobj->add (subobj->clone ());
            }
            catch (const unrecognizedObjectException &)
            {
                // this likely means that the parent will take care of it itself
            }
        }
    }
    else
    {
        auto csz = nobj->subObjectList.size ();
        // clone the subObjects
        for (size_t ii = 0; ii < subObjectList.size (); ++ii)
        {
            if (subObjectList[ii]->locIndex != kNullLocation)
            {
                bool fnd = false;
                for (size_t kk = 0; kk < csz; ++kk)
                {
                    if (nobj->subObjectList[kk]->locIndex == subObjectList[ii]->locIndex)
                    {
                        if (typeid (nobj->subObjectList[kk]) ==
                            typeid (subObjectList[ii]))  // make sure the types are same before cloning
                        {
                            subObjectList[ii]->clone (nobj->subObjectList[kk]);
                            fnd = true;
                            break;
                        }
                    }
                }
                if (!fnd)
                {
                    try
                    {
                        nobj->add (subObjectList[ii]->clone ());
                    }
                    catch (const unrecognizedObjectException &)
                    {
                        // this likely means that the parent will take care of it itself
                    }
                }
            }
            else
            {
                if (ii >= csz)
                {
                    try
                    {
                        nobj->add (subObjectList[ii]->clone ());
                    }
                    catch (const unrecognizedObjectException &)
                    {
                        // this likely means that the parent will take care of it itself
                    }
                }
                else
                {
                    if (typeid (subObjectList[ii]) == typeid (nobj->subObjectList[ii]))
                    {
                        subObjectList[ii]->clone (nobj->subObjectList[ii]);
                    }
                    else
                    {
                        try
                        {
                            nobj->add (subObjectList[ii]->clone ());
                        }
                        catch (const unrecognizedObjectException &)
                        {
                            // this likely means that the parent will take care of it itself
                        }
                    }
                }
            }
        }
    }

    return nobj;
}

void gridComponent::updateObjectLinkages (coreObject *newRoot)
{
    for (auto &subobj : getSubObjects ())
    {
        subobj->updateObjectLinkages (newRoot);
    }
}

void gridComponent::pFlowInitializeA (coreTime time0, std::uint32_t flags)
{
    if (isEnabled ())
    {
        pFlowObjectInitializeA (time0, flags);
        prevTime = time0;
        updateFlags (false);
        setupPFlowFlags ();
    }
}

void gridComponent::pFlowInitializeB ()
{
    if (isEnabled ())
    {
        pFlowObjectInitializeB ();
        opFlags.set (pFlow_initialized);
    }
}

void gridComponent::dynInitializeA (coreTime time0, std::uint32_t flags)
{
    if (isEnabled ())
    {
        dynObjectInitializeA (time0, flags);
        prevTime = time0;
        updateFlags (true);
        setupDynFlags ();
    }
}

void gridComponent::dynInitializeB (const IOdata &inputs, const IOdata &desiredOutput, IOdata &fieldSet)
{
    if (isEnabled ())
    {
        dynObjectInitializeB (inputs, desiredOutput, fieldSet);
        if (updatePeriod < maxTime)
        {
            setUpdateTime (prevTime + updatePeriod);
            enable_updates ();
        }
        opFlags.set (dyn_initialized);
    }
}

void gridComponent::pFlowObjectInitializeA (coreTime time0, std::uint32_t flags)
{
    for (auto &subobj : subObjectList)
    {
        subobj->pFlowInitializeA (time0, flags);
    }
}

void gridComponent::pFlowObjectInitializeB ()
{
    for (auto &subobj : subObjectList)
    {
        subobj->pFlowInitializeB ();
    }
}

void gridComponent::dynObjectInitializeA (coreTime time0, std::uint32_t flags)
{
    for (auto &subobj : subObjectList)
    {
        subobj->dynInitializeA (time0, flags);
    }
}

void gridComponent::dynObjectInitializeB (const IOdata &inputs, const IOdata &desiredOutput, IOdata &fieldSet)
{
    for (auto &subobj : subObjectList)
    {
        if (!subobj->checkFlag (separate_processing))
        {
            subobj->dynInitializeB (inputs, desiredOutput, fieldSet);
        }
    }
}

count_t gridComponent::stateSize (const solverMode &sMode)
{
    const auto &so = offsets.getOffsets(sMode);
    if (!(so.stateLoaded))
    {
        loadSizes (sMode, false);
    }
    count_t ssize = (hasAlgebraic (sMode)) ? (so.total.algSize + so.total.vSize + so.total.aSize) : 0;
    if (hasDifferential (sMode))
    {
        ssize += so.total.diffSize;
    }
    return ssize;
}

count_t gridComponent::stateSize (const solverMode &sMode) const
{
    auto &so = offsets.getOffsets (sMode);
    count_t ssize = (hasAlgebraic (sMode)) ? (so.total.algSize + so.total.vSize + so.total.aSize) : 0;
    if (hasDifferential (sMode))
    {
        ssize += so.total.diffSize;
    }
    return ssize;
}

count_t gridComponent::totalAlgSize (const solverMode &sMode)
{
    auto &so = offsets.getOffsets (sMode);
    if (!(so.stateLoaded))
    {
        loadSizes (sMode, false);
    }
    return so.total.algSize + so.total.vSize + so.total.aSize;
}

count_t gridComponent::totalAlgSize (const solverMode &sMode) const
{
    auto &so = offsets.getOffsets (sMode);
    return (so.total.algSize + so.total.vSize + so.total.aSize);
}

count_t gridComponent::algSize (const solverMode &sMode)
{
    auto &so = offsets.getOffsets (sMode);
    if (!(so.stateLoaded))
    {
        loadSizes (sMode, false);
    }
    return so.total.algSize;
}

count_t gridComponent::algSize (const solverMode &sMode) const
{
    const auto &so = offsets.getOffsets (sMode);
    return so.total.algSize;
}

count_t gridComponent::diffSize (const solverMode &sMode)
{
    auto &so = offsets.getOffsets (sMode);
    if (!(so.stateLoaded))
    {
        loadSizes (sMode, false);
    }
    return so.total.diffSize;
}

count_t gridComponent::diffSize (const solverMode &sMode) const
{
    auto &so = offsets.getOffsets (sMode);
    return so.total.diffSize;
}

count_t gridComponent::rootSize (const solverMode &sMode)
{
    auto &so = offsets.getOffsets (sMode);
    if (!(so.rjLoaded))
    {
        loadSizes (sMode, true);
    }
    return so.total.algRoots + so.total.diffRoots;
}

count_t gridComponent::rootSize (const solverMode &sMode) const
{
    auto &so = offsets.getOffsets (sMode);

	return so.total.algRoots + so.total.diffRoots;
}

count_t gridComponent::jacSize (const solverMode &sMode)
{
    auto &so = offsets.getOffsets (sMode);
    if (!(so.rjLoaded))
    {
        loadSizes (sMode, true);
    }
    return so.total.jacSize;
}

count_t gridComponent::jacSize (const solverMode &sMode) const
{
    auto &so = offsets.getOffsets (sMode);
    return  so.total.jacSize;
}

count_t gridComponent::voltageStateCount (const solverMode &sMode)
{
    auto &so = offsets.getOffsets (sMode);
    if (!(so.stateLoaded))
    {
        loadSizes (sMode, false);
    }
    return so.total.vSize;
}

count_t gridComponent::voltageStateCount (const solverMode &sMode) const
{
    auto &so = offsets.getOffsets (sMode);
    return so.total.vSize;
}

count_t gridComponent::angleStateCount (const solverMode &sMode)
{
    auto &so = offsets.getOffsets (sMode);
    if (!(so.stateLoaded))
    {
        loadSizes (sMode, false);
    }
    return so.total.aSize;
}

count_t gridComponent::angleStateCount (const solverMode &sMode) const
{
    auto &so = offsets.getOffsets (sMode);
    return so.total.aSize;
}

const solverOffsets &gridComponent::getOffsets (const solverMode &sMode) const { return offsets.getOffsets (sMode); }
void gridComponent::setOffsets (const solverOffsets &newOffsets, const solverMode &sMode)
{
    offsets.setOffsets (newOffsets, sMode);

    if (!subObjectList.empty ())
    {
        solverOffsets no (newOffsets);
        no.localIncrement (offsets.getOffsets (sMode));
        for (auto &subobj : subObjectList)
        {
            if (subobj->isEnabled ())
            {
				subobj->setOffsets (no, sMode);
                no.increment (subobj->offsets.getOffsets (sMode));
            }
        }
    }
}

void gridComponent::setOffset (index_t newOffset, const solverMode &sMode)
{
    if (!subObjectList.empty ())
    {
        for (auto &subobj: subObjectList)
        {
            if (subobj->isEnabled ())
            {
				subobj->setOffset (newOffset, sMode);
                newOffset += subobj->stateSize (sMode);
            }
        }
    }
    offsets.setOffset (newOffset, sMode);
}

bool gridComponent::isLoaded (const solverMode &sMode, bool dynOnly) const
{
    return (dynOnly) ? offsets.isrjLoaded (sMode) : offsets.isLoaded (sMode);
}

/* *INDENT-OFF* */
static const std::map<std::string, operation_flags> user_settable_flags{
  {"use_bus_frequency", uses_bus_frequency},
  {"late_b_initialize", late_b_initialize},
  {"error", error_flag},
  {"no_gridobject_set", no_gridobject_set},
  {"disable_flag_update", disable_flag_updates},
  {"flag_update_required", flag_update_required},
  {"pflow_init_required", pflow_init_required},
  {"sampled_only", no_dyn_states},
};

// there isn't that many flags that we want to be user settable, most are controlled by the model so allowing them
// to be set by an external function
// might not be the best thing
void gridComponent::setFlag (const std::string &flag, bool val)
{
    auto ffind = user_settable_flags.find (flag);
    if (ffind != user_settable_flags.end ())
    {
        opFlags.set (ffind->second, val);
        if (flag == "sampled_only")
        {
            if (opFlags[pFlow_initialized])
            {
                offsets.unload ();
            }
        }
    }
    else if (flag == "connected")
    {
        if (val)
        {
            if (!isConnected ())
            {
                reconnect ();
            }
        }
        else if (isConnected ())
        {
            disconnect ();
        }
    }
    else if (flag == "disconnected")
    {
        if (val)
        {
            if (isConnected ())
            {
                disconnect ();
            }
        }
        else if (!isConnected ())
        {
            reconnect ();
        }
    }
    else if (subObjectSet (flag, val))
    {
        return;
    }
    else
    {
        coreObject::setFlag (flag, val);
    }
}

static const std::vector<index_t> parentSettableFlags{sampled_only, no_gridobject_set, separate_processing};

void gridComponent::parentSetFlag (index_t flagID, bool val, coreObject *checkParent)
{
    if (isSameObject (getParent (), checkParent))
    {
        if (std::binary_search (parentSettableFlags.begin (), parentSettableFlags.end (), flagID))
        {
            opFlags[flagID] = val;
        }
    }
}

static const std::map<std::string, operation_flags> flagmap{{"constraints", has_constraints},
                                                            {"roots", has_roots},
                                                            {"alg_roots", has_alg_roots},
                                                            {"voltage_adjustments", has_powerflow_adjustments},
                                                            {"preex", preEx_requested},
                                                            {"use_bus_frequency", uses_bus_frequency},
                                                            {"pflow_states", has_pflow_states},
                                                            {"dyn_states", has_dyn_states},
                                                            {"differential_states", has_differential_states},
                                                            {"not_cloneable", not_cloneable},
                                                            {"remote_voltage_control", remote_voltage_control},
                                                            {"local_voltage_control", local_voltage_control},
                                                            {"indirect_voltage_control", indirect_voltage_control},
                                                            {"remote_power_control", remote_voltage_control},
                                                            {"local_power_control", local_power_control},
                                                            {"indirect_power_control", indirect_power_control},
                                                            {"pflow_initialized", pFlow_initialized},
                                                            {"dyn_initialized", dyn_initialized},
                                                            {"armed", object_armed_flag},
                                                            {"late_b_initialize", late_b_initialize},
                                                            {"object_flag1", object_flag1},
                                                            {"object_flag2", object_flag2},
                                                            {"object_flag3", object_flag3},
                                                            {"object_flag4", object_flag4},
                                                            {"object_flag5", object_flag5},
                                                            {"object_flag6", object_flag6},
                                                            {"object_flag7", object_flag7},
                                                            {"object_flag8", object_flag8},
                                                            {"object_flag9", object_flag9},
                                                            {"object_flag10", object_flag10},
                                                            {"object_flag11", object_flag11},
                                                            {"object_flag12", object_flag12},
                                                            {"state_change", state_change_flag},
                                                            {"object_change", object_change_flag},
                                                            {"constraint_change", constraint_change_flag},
                                                            {"root_change", root_change_flag},
                                                            {"jacobian_count_change", jacobian_count_change_flag},
                                                            {"slack_bus_change", slack_bus_change},
                                                            {"voltage_control_change", voltage_control_change},
                                                            {"error", error_flag},
                                                            {"connectivity_change", connectivity_change_flag},
                                                            {"no_pflow_states", no_pflow_states},
                                                            {"disconnected", disconnected},
                                                            {"no_dyn_states", no_dyn_states},
                                                            {"sampled_only", no_dyn_states},
                                                            {"disable_flag_update", disable_flag_updates},
                                                            {"flag_update_required", flag_update_required},
                                                            {"differential_output", differential_output},
                                                            {"multipart_calculation_capable",
                                                             multipart_calculation_capable},
                                                            {"pflow_init_required", pflow_init_required},
                                                            {"dc_only", dc_only},
                                                            {"dc_capable", dc_capable},
                                                            {"dc_terminal2", dc_terminal2},
                                                            {"separate_processing", separate_processing},
                                                            {"three_phase_only", three_phase_only},
                                                            {"three_phase_capable", three_phase_capable},
                                                            {"three_phase_terminal2", three_phase_terminal2}};


bool gridComponent::getFlag (const std::string &param) const
{
    auto ffind = flagmap.find (param);
    if (ffind != flagmap.end ())
    {
        return opFlags[ffind->second];
    }
    return coreObject::getFlag (param);
}

bool gridComponent::checkFlag (index_t flagID) const { return opFlags.test (flagID); }
bool gridComponent::hasStates (const solverMode &sMode) const { return (stateSize (sMode) > 0); }
bool gridComponent::isArmed () const { return opFlags[object_armed_flag]; }
bool gridComponent::isConnected () const { return !(opFlags[disconnected]); }
void gridComponent::reconnect () { opFlags.set (disconnected, false); }
void gridComponent::disconnect () { opFlags.set (disconnected); }
static const stringVec locNumStrings{"status", "basefrequency", "basepower"};
static const stringVec locStrStrings{"status"};

void gridComponent::getParameterStrings (stringVec &pstr, paramStringType pstype) const
{
    getParamString<gridComponent, coreObject> (this, pstr, locNumStrings, locStrStrings, {}, pstype);
}

void gridComponent::set (const std::string &param, const std::string &val)
{
    if (opFlags[no_gridobject_set])
    {
        throw (unrecognizedParameter (param));
    }

    if (param == "status")
    {
        auto v2 = convertToLowerCase (val);
        if ((v2 == "on") || (v2 == "in") || (v2 == "enabled"))
        {
            if (!isEnabled ())
            {
                enable ();
                if ((opFlags[has_pflow_states]) || (opFlags[has_dyn_states]))
                {
                    alert (this, STATE_COUNT_CHANGE);
                }
            }
        }
        else if ((v2 == "off") || (v2 == "out") || (v2 == "disabled"))
        {
            if (isEnabled ())
            {
                if ((opFlags[has_pflow_states]) || (opFlags[has_dyn_states]))
                {
                    alert (this, STATE_COUNT_CHANGE);
                }
                disable ();
            }
        }
        else if (v2 == "connected")
        {
            if (!isConnected ())
            {
                reconnect ();
            }
        }
        else if (v2 == "disconnected")
        {
            if (isConnected ())
            {
                disconnect ();
            }
        }
    }
    else if (param == "flags")
    {
        setMultipleFlags (this, val);
    }
    else if (subObjectSet (param, val))
    {
        return;
    }
    else
    {
        coreObject::set (param, val);
    }
}

auto hasParameterPath (const std::string &param) { return (param.find_last_of (":?") != std::string::npos); }
bool gridComponent::subObjectSet (const std::string &param, double val, gridUnits::units_t unitType)
{
    if (hasParameterPath (param))
    {
        objInfo pinfo (param, this);
        if (pinfo.m_obj != nullptr)
        {
            if (pinfo.m_unitType != gridUnits::units_t::defUnit)
            {
                pinfo.m_obj->set (pinfo.m_field, val, pinfo.m_unitType);
            }
            else
            {
                pinfo.m_obj->set (pinfo.m_field, val, unitType);
            }
            return true;
        }
        throw (unrecognizedParameter (param));
    }
    return false;
}

bool gridComponent::subObjectSet (const std::string &param, const std::string &val)
{
    if (hasParameterPath (param))
    {
        objInfo pinfo (param, this);
        if (pinfo.m_obj != nullptr)
        {
            pinfo.m_obj->set (pinfo.m_field, val);
        }
        else
        {
            throw (unrecognizedParameter (param));
        }
        return true;
    }
    return false;
}

bool gridComponent::subObjectSet (const std::string &flag, bool val)
{
    if (hasParameterPath (flag))
    {
        objInfo pinfo (flag, this);
        if (pinfo.m_obj != nullptr)
        {
            pinfo.m_obj->setFlag (pinfo.m_field, val);
        }
        else
        {
            throw (unrecognizedParameter (flag));
        }
        return true;
    }
    return false;
}

double gridComponent::subObjectGet (const std::string &param, gridUnits::units_t unitType) const
{
    if (hasParameterPath (param))
    {
        objInfo pinfo (param, this);
        if (pinfo.m_obj != nullptr)
        {
            if (pinfo.m_unitType != gridUnits::units_t::defUnit)
            {
                return pinfo.m_obj->get (pinfo.m_field, pinfo.m_unitType);
            }
            return pinfo.m_obj->get (pinfo.m_field, unitType);
        }
        throw (unrecognizedParameter (param));
    }
    return kNullVal;
}

void gridComponent::set (const std::string &param, double val, gridUnits::units_t unitType)
{
    if (opFlags[no_gridobject_set])
    {
        throw (unrecognizedParameter (param));
    }

    if ((param == "enabled") || (param == "status"))
    {
        if (val > 0.1)
        {
            if (!isEnabled ())
            {
                enable ();
                if (opFlags[has_dyn_states])
                {
                    alert (this, STATE_COUNT_CHANGE);
                }
            }
        }
        else
        {
            if (isEnabled ())
            {
                if (opFlags[has_dyn_states])
                {
                    alert (this, STATE_COUNT_CHANGE);
                }
                disable ();
            }
        }
    }
    else if (param == "connected")
    {
        if (val > 0.1)
        {
            if (!isConnected ())
            {
                reconnect ();
            }
        }
        else
        {
            if (isConnected ())
            {
                disconnect ();
            }
        }
    }
    else if ((param == "basepower")||(param=="basemw")||(param=="basemva"))
    {
        systemBasePower = gridUnits::unitConversion (val, unitType, gridUnits::MW);
        setAll ("all", "basepower", systemBasePower);
    }
    else if ((param == "basefreq") || (param == "basefrequency")||(param=="systembasefrequency"))
    {
        systemBaseFrequency = gridUnits::unitConversionFreq (val, unitType, gridUnits::rps);
        setAll ("all", "basefreq", systemBasePower);
    }
    else if (subObjectSet (param, val, unitType))
    {
        return;
    }
    else
    {
        coreObject::set (param, val, unitType);
    }
}

void gridComponent::setAll (const std::string &type,
                         const std::string &param,
                         double val,
                         gridUnits::units_t unitType)
{
    if ((type == "all") || (type == "sub") || (type == "object"))
    {
        for (auto &subobj : subObjectList)
        {
			subobj->set (param, val, unitType);
        }
    }
}

double gridComponent::get (const std::string &param, gridUnits::units_t unitType) const
{
    double out = kNullVal;
    if (param == "basefrequency")
    {
        out = gridUnits::unitConversionFreq (systemBaseFrequency, gridUnits::rps, unitType);
    }
    else if (param == "basepower")
    {
        out = gridUnits::unitConversion (systemBasePower, gridUnits::MVAR, unitType, systemBasePower);
    }
    else if (param == "subobjectcount")
    {
        out = static_cast<double> (subObjectList.size ());
    }
    else
    {
        out = subObjectGet (param, unitType);
        if (out == kNullVal)
        {
            out = coreObject::get (param, unitType);
        }
    }
    return out;
}

void gridComponent::addSubObject (gridComponent *comp)
{
    if (comp == nullptr)
    {
        return;
    }
    for (auto &sobj : subObjectList)
    {
        if (isSameObject (sobj, comp))
        {
            return;
        }
    }
    comp->setParent (this);
	comp->addOwningReference ();
	comp->systemBaseFrequency = systemBaseFrequency;
	comp->systemBasePower = systemBasePower;
    subObjectList.push_back (comp);
}

void gridComponent::remove (coreObject *obj)
{
    if (!subObjectList.empty ())
    {
        auto rmobj = std::find_if (subObjectList.begin (), subObjectList.end (),
                                   [obj](coreObject *so) { return isSameObject (so, obj); });
        if (rmobj != subObjectList.end ())
        {
            removeReference (*rmobj, this);
            subObjectList.erase (rmobj);
        }
    }
}

void gridComponent::reset (reset_levels level)
{
    for (auto &subobj : subObjectList)
    {
        subobj->reset (level);
    }
}

change_code gridComponent::powerFlowAdjust (const IOdata &inputs, std::uint32_t flags, check_level_t level)
{
    auto ret = change_code::no_change;

    for (auto &subobj : subObjectList)
    {
        if (!(subobj->checkFlag (has_powerflow_adjustments)))
        {
            continue;
        }
        auto iret = subobj->powerFlowAdjust (inputs, flags, level);
        if (iret > ret)
        {
            ret = iret;
        }
    }
    return ret;
}

void gridComponent::setState (coreTime time, const double state[], const double dstate_dt[], const solverMode &sMode)
{
    prevTime = time;
    if (!hasStates (sMode))  // use the const version of stateSize
    {
        return;
    }
    const auto &so = offsets.getOffsets (sMode);
    const auto &localStates = (subObjectList.empty ()) ? (so.total) : (so.local);

    if (hasAlgebraic (sMode))
    {
        if (localStates.algSize > 0)
        {
            std::copy (state + so.algOffset, state + so.algOffset + localStates.algSize, m_state.data ());
        }
    }
    if (localStates.diffSize > 0)
    {
        if (isDifferentialOnly (sMode))
        {
            std::copy (state + so.diffOffset, state + so.diffOffset + localStates.diffSize,
                       m_state.data () + algSize (cLocalSolverMode));
            std::copy (dstate_dt + so.diffOffset, dstate_dt + so.diffOffset + localStates.diffSize,
                       m_dstate_dt.data () + algSize (cLocalSolverMode));
        }
        else
        {
            std::copy (state + so.diffOffset, state + so.diffOffset + localStates.diffSize,
                       m_state.data () + localStates.algSize);
            std::copy (dstate_dt + so.diffOffset, dstate_dt + so.diffOffset + localStates.diffSize,
                       m_dstate_dt.data () + localStates.algSize);
        }
    }

    for (auto &sub : subObjectList)
    {
        sub->setState (time, state, dstate_dt, sMode);
    }
}
// for saving the state
void gridComponent::guessState (coreTime time, double state[], double dstate_dt[], const solverMode &sMode)
{
    if (!hasStates (sMode))
    {
        return;
    }
    const auto &so = offsets.getOffsets (sMode);
    const auto &localStates = (subObjectList.empty ()) ? (so.total) : (so.local);

    if (hasAlgebraic (sMode))
    {
        if (localStates.algSize > 0)
        {
            assert (so.algOffset != kNullLocation);
            std::copy (m_state.begin (), m_state.begin () + localStates.algSize, state + so.algOffset);
        }
    }
    if (localStates.diffSize > 0)
    {
        if (isDifferentialOnly (sMode))
        {
            assert (so.diffOffset != kNullLocation);
            index_t localAlgSize = algSize (cLocalSolverMode);
            std::copy (m_state.begin () + localAlgSize, m_state.begin () + localAlgSize + localStates.diffSize,
                       state + so.diffOffset);
            std::copy (m_dstate_dt.data () + localAlgSize,
                       m_dstate_dt.data () + localAlgSize + localStates.diffSize, dstate_dt + so.diffOffset);
        }
        else
        {
            if (so.diffOffset == kNullLocation)
            {
                printf ("%s::%s in mode %d %d ds=%d, do=%d\n", getParent ()->getName ().c_str (),
                        getName ().c_str (), static_cast<int> (isLocal (sMode)), static_cast<int> (isDAE (sMode)),
                        static_cast<int> (so.total.diffSize), static_cast<int> (so.diffOffset));
                printStackTrace ();
            }
            assert (so.diffOffset != kNullLocation);
            count_t stateCount = localStates.algSize + localStates.diffSize;
            std::copy (m_state.begin () + localStates.algSize, m_state.begin () + stateCount,
                       state + so.diffOffset);
            std::copy (m_dstate_dt.data () + localStates.algSize, m_dstate_dt.data () + stateCount,
                       dstate_dt + so.diffOffset);
        }
    }

    for (auto &sub : subObjectList)
    {
        sub->guessState (time, state, dstate_dt, sMode);
    }
}

void gridComponent::setupPFlowFlags ()
{
    auto ss = stateSize (cPflowSolverMode);
    opFlags.set (has_pflow_states, (ss > 0));
	//load the subobject pflow states;
	for (auto &sub : subObjectList)
	{
		if (sub->checkFlag(has_pflow_states))
		{
			opFlags.set(has_subobject_pflow_states);
			return;
		}
	}
}

void gridComponent::setupDynFlags ()
{
    auto ss = stateSize (cDaeSolverMode);

    opFlags.set (has_dyn_states, (ss > 0));
    const auto &so = offsets.getOffsets (cDaeSolverMode);
    if (so.total.algRoots > 0)
    {
        opFlags.set (has_alg_roots);
        opFlags.set (has_roots);
    }
    else if (so.total.diffRoots > 0)
    {
        opFlags.reset (has_alg_roots);
        opFlags.set (has_roots);
    }
    else
    {
        opFlags.reset (has_alg_roots);
        opFlags.reset (has_roots);
    }
}

double gridComponent::getState (index_t offset) const
{
    if ((offset != kNullLocation) && (offset < static_cast<index_t> (m_state.size ())))
    {
        return m_state[offset];
    }
    return kNullVal;
}

void gridComponent::loadSizesSub (const solverMode &sMode, bool dynOnly)
{
    auto &so = offsets.getOffsets (sMode);
    if (dynOnly)
    {
        so.total.algRoots = so.local.algRoots;
        so.total.diffRoots = so.local.diffRoots;
        so.total.jacSize = so.local.jacSize;
    }
    else
    {
        so.localLoad (false);
    }
    for (auto &sub : subObjectList)
    {
        if (sub->isEnabled ())
        {
            if (!(sub->isLoaded (sMode, dynOnly)))
            {
                sub->loadSizes (sMode, dynOnly);
            }
            if (sub->checkFlag (sampled_only))
            {
                continue;
            }
            if (dynOnly)
            {
                so.addRootAndJacobianSizes (sub->offsets.getOffsets (sMode));
            }
            else
            {
                so.addSizes (sub->offsets.getOffsets (sMode));
            }
        }
    }
    if (!dynOnly)
    {
        so.stateLoaded = true;
    }
    so.rjLoaded = true;
}

void gridComponent::loadSizes (const solverMode &sMode, bool dynOnly)
{
    if (isLoaded (sMode, dynOnly))
    {
        return;
    }
    auto &so = offsets.getOffsets (sMode);
    if (!isEnabled ())
    {
        so.reset ();
        so.stateLoaded = true;
        so.rjLoaded = true;
        return;
    }
    if ((!isDynamic (sMode)) && (opFlags[no_pflow_states]))
    {
        so.stateReset ();
        so.stateLoaded = true;
        return;
    }
    if ((isDynamic (sMode)) && (opFlags[no_dyn_states]))
    {
        so.stateReset ();
        so.stateLoaded = true;
    }
    auto &lc = offsets.local ();
    if (dynOnly)
    {
        if (!isLocal (sMode))  // don't reset if it is the local offsets
        {
            so.rootAndJacobianCountReset ();
        }
    }
    else
    {
        if (!(so.stateLoaded))
        {
            if (!isLocal (sMode))  // don't reset if it is the local offsets
            {
                so.stateReset ();
            }
            if (hasAlgebraic (sMode))
            {
                so.local.aSize = lc.local.aSize;
                so.local.vSize = lc.local.vSize;
                so.local.algSize = lc.local.algSize;
            }
            if (hasDifferential (sMode))
            {
                so.local.diffSize = lc.local.diffSize;
            }
        }
    }
    if (!(so.rjLoaded))
    {
        so.local.algRoots = lc.local.algRoots;
        so.local.diffRoots = lc.local.diffRoots;
        so.local.jacSize = lc.local.jacSize;
    }
    if (opFlags[sampled_only])  // no states
    {
        if (sMode == cLocalSolverMode)
        {
            for (auto &sub : subObjectList)
            {
                sub->setFlag ("sampled_only");
            }
        }
        else
        {
            so.local.reset ();
            so.total.reset ();
        }
    }
    if (subObjectList.empty ())
    {
        if (dynOnly)
        {
            so.total.algRoots = so.local.algRoots;
            so.total.diffRoots = so.local.diffRoots;
            so.total.jacSize = so.local.jacSize;
            so.rjLoaded = true;
        }
        else
        {
            so.localLoad (true);
        }
    }
    else
    {
        loadSizesSub (sMode, dynOnly);
    }
}

void gridComponent::getTols (double tols[], const solverMode &sMode)
{
    for (auto &subObj : subObjectList)
    {
        if (subObj->isEnabled ())
        {
			subObj->getTols (tols, sMode);
        }
    }
}

void gridComponent::getVariableType (double sdata[], const solverMode &sMode)
{
    auto &so = offsets.getOffsets (sMode);
    if (subObjectList.empty ())
    {
        if (so.total.algSize > 0)
        {
            auto offset = so.algOffset;
            for (count_t kk = 0; kk < so.total.algSize; ++kk)
            {
                sdata[offset + kk] = ALGEBRAIC_VARIABLE;
            }
        }
        if (so.total.diffSize > 0)
        {
            auto offset = so.diffOffset;
            for (count_t kk = 0; kk < so.total.diffSize; ++kk)
            {
                sdata[offset + kk] = DIFFERENTIAL_VARIABLE;
            }
        }
    }
    else
    {
        if (so.local.algSize > 0)
        {
            auto offset = so.algOffset;
            for (count_t kk = 0; kk < so.local.algSize; ++kk)
            {
                sdata[offset + kk] = ALGEBRAIC_VARIABLE;
            }
        }
        if (so.local.diffSize > 0)
        {
            auto offset = so.diffOffset;
            for (count_t kk = 0; kk < so.local.diffSize; ++kk)
            {
                sdata[offset + kk] = DIFFERENTIAL_VARIABLE;
            }
        }
        for (auto &subobj : subObjectList)
        {
            if (subobj->isEnabled ())
            {
                subobj->getVariableType (sdata, sMode);
            }
        }
    }
}

void gridComponent::getConstraints (double constraints[], const solverMode &sMode)
{
    for (auto &subobj : subObjectList)
    {
        if ((subobj->isEnabled ()) && (subobj->checkFlag (has_constraints)))
        {
			subobj->getConstraints (constraints, sMode);
        }
    }
}

void gridComponent::setRootOffset (index_t newRootOffset, const solverMode &sMode)
{
    offsets.setRootOffset (newRootOffset, sMode);
    auto &so = offsets.getOffsets (sMode);
    auto nR = so.local.algRoots + so.local.diffRoots;
    for (auto &ro : subObjectList)
    {
        ro->setRootOffset (newRootOffset + nR, sMode);
        nR += ro->rootSize (sMode);
    }
}

static const stringVec emptyStr{};

stringVec gridComponent::localStateNames () const { return emptyStr; }
index_t gridComponent::findIndex (const std::string &field, const solverMode &sMode) const
{
    auto &so = offsets.getOffsets (sMode);
    if (field.compare (0, 5, "state") == 0)
    {
        auto num = static_cast<index_t> (stringOps::trailingStringInt (field, 0));
        if (stateSize (sMode) > num)
        {
            if (so.algOffset != kNullLocation)
            {
                return so.algOffset + num;
            }
            return kNullLocation;
        }
        return kInvalidLocation;
    }
    if (field.compare (0, 3, "alg") == 0)
    {
        auto num = static_cast<index_t> (stringOps::trailingStringInt (field, 0));
        if (so.total.algSize > num)
        {
            if (so.algOffset != kNullLocation)
            {
                return so.algOffset + num;
            }
            return kNullLocation;
        }
        if (!opFlags[dyn_initialized])
        {
            return kNullLocation;
        }
        return kInvalidLocation;
    }
    if (field.compare (0, 4, "diff") == 0)
    {
        auto num = static_cast<index_t> (stringOps::trailingStringInt (field, 0));
        if (so.total.diffSize > num)
        {
            if (so.diffOffset != kNullLocation)
            {
                return so.diffOffset + num;
            }
            return kNullLocation;
        }
        if (!opFlags[dyn_initialized])
        {
            return kNullLocation;
        }
        return kInvalidLocation;
    }
    auto stateNames = localStateNames ();
    for (index_t nn = 0; nn < static_cast<index_t> (stateNames.size ()); ++nn)
    {
        if (field == stateNames[nn])
        {
            auto &lc = offsets.local ();
            if (nn < lc.local.algSize)
            {
                if (so.algOffset != kNullLocation)
                {
                    return so.algOffset + nn;
                }
                return kNullLocation;
            }
            if (nn - lc.local.algSize < lc.local.diffSize)
            {
                if (so.diffOffset != kNullLocation)
                {
                    return so.diffOffset + nn - lc.local.algSize;
                }
                return kNullLocation;
            }
            if (!opFlags[dyn_initialized])
            {
                return kNullLocation;
            }
            return kInvalidLocation;
        }
    }
    for (auto &subobj : subObjectList)
    {
        auto ret = subobj->findIndex (field, sMode);
        if (ret != kInvalidLocation)
        {
            return ret;
        }
    }
    return kInvalidLocation;
}

void gridComponent::getStateName (stringVec &stNames, const solverMode &sMode, const std::string &prefix) const
{
    auto &so = offsets.getOffsets (sMode);
    auto mxsize = offsets.maxIndex (sMode);
    std::string prefix2 = prefix + getName () + ':';

    if (mxsize > 0)
    {
        ensureSizeAtLeast (stNames, mxsize + 1);
    }
    else
    {
        return;
    }
    auto stateNames = localStateNames ();
    auto stsize = static_cast<index_t> (stateNames.size ());
    decltype (mxsize) ss = 0;
    if (hasAlgebraic (sMode))
    {
        for (index_t kk = 0; kk < so.local.vSize; kk++)
        {
            if (stsize > ss)
            {
                stNames[so.vOffset + kk] = prefix2 + stateNames[ss];
                ++ss;
            }
            else
            {
                stNames[so.vOffset + kk] = prefix2 + "voltage_state_" + std::to_string (kk);
            }
        }
        ss = offsets.local ().local.vSize;
        for (index_t kk = 0; kk < so.local.aSize; kk++)
        {
            if (stsize > ss)
            {
                stNames[so.aOffset + kk] = prefix2 + stateNames[ss];
                ++ss;
            }
            else
            {
                stNames[so.aOffset + kk] = prefix2 + "angle_state_" + std::to_string (kk);
            }
        }
        ss = offsets.local ().local.vSize + offsets.local ().local.aSize;
        for (index_t kk = 0; kk < so.local.algSize; kk++)
        {
            if (stsize > ss)
            {
                stNames[so.algOffset + kk] = prefix2 + stateNames[ss];
                ++ss;
            }
            else
            {
                stNames[so.algOffset + kk] = prefix2 + "alg_state_" + std::to_string (kk);
            }
        }
    }
    if (!isAlgebraicOnly (sMode))
    {
        if (so.local.diffSize > 0)
        {
            ss = offsets.local ().local.algSize + offsets.local ().local.vSize + offsets.local ().local.aSize;
            for (index_t kk = 0; kk < so.local.diffSize; kk++)
            {
                if (stsize > ss)
                {
                    stNames[so.diffOffset + kk] = prefix2 + stateNames[ss];
                    ++ss;
                }
                else
                {
                    stNames[so.diffOffset + kk] = prefix2 + "diff_state_" + std::to_string (kk);
                }
            }
        }
    }

    for (auto &subobj : subObjectList)
    {
        subobj->getStateName (stNames, sMode, prefix2 + ':');
    }
}

void gridComponent::updateFlags (bool dynamicsFlags)
{
    for (auto &subobj : subObjectList)
    {
        if (subobj->isEnabled ())
        {
            opFlags |= subobj->cascadingFlags ();
        }
    }
    if (dynamicsFlags)
    {
        setupDynFlags ();
    }
    else
    {
        setupPFlowFlags ();
    }

    opFlags.reset (flag_update_required);
}

void gridComponent::updateLocalCache (const IOdata &inputs, const stateData &sD, const solverMode &sMode)
{
    for (auto &sub : subObjectList)
    {
        sub->updateLocalCache (inputs, sD, sMode);
    }
}

coreObject *gridComponent::find (const std::string &object) const
{
    auto foundobj = std::find_if (subObjectList.begin (), subObjectList.end (),
                                  [object](gridComponent *comp) { return (object == comp->getName ()); });
    if (foundobj != subObjectList.end ())
    {
        return *foundobj;
    }
    // return nullptr if this is an indexed name
    auto rlc2 = object.find_last_of ("#$!");
    if (rlc2 != std::string::npos)
    {
        return nullptr;
    }
    return coreObject::find (object);
}

coreObject *gridComponent::getSubObject (const std::string &typeName, index_t objectNum) const
{
    if ((typeName == "sub") || (typeName == "subobject") || (typeName == "object"))
    {
		if (isValidIndex(objectNum,subObjectList))
        {
            return subObjectList[objectNum];
        }
    }
    return nullptr;
}

coreObject *gridComponent::findByUserID (const std::string &typeName, index_t searchID) const
{
    if ((typeName == "sub") || (typeName == "subobject") || (typeName == "object"))
    {
        auto foundobj = std::find_if (subObjectList.begin (), subObjectList.end (),
                                      [searchID](gridComponent *comp) { return (comp->getUserID () == searchID); });
        if (foundobj == subObjectList.end ())
        {
            return nullptr;
        }
        return *foundobj;
    }
    return coreObject::findByUserID (typeName, searchID);
}

void gridComponent::timestep (coreTime time, const IOdata &inputs, const solverMode &sMode)
{
    prevTime = time;

    for (auto &subobj : subObjectList)
    {
        if (subobj->currentTime () < time)
        {
            if (!subobj->checkFlag (separate_processing))
            {
                subobj->timestep (time, inputs, sMode);
            }
        }
    }
}

void gridComponent::ioPartialDerivatives (const IOdata & /*inputs*/,
                                       const stateData & /*sD*/,
                                       matrixData<double> & /*md*/,
                                       const IOlocs & /*inputLocs*/,
                                       const solverMode & /*sMode*/)
{
    /* there is no way to determine partial derivatives of the output with respect to input in a default manner
    therefore the default is no dependencies
    */
}

void gridComponent::outputPartialDerivatives (const IOdata & /*inputs*/,
                                           const stateData & /*sD*/,
                                           matrixData<double> &md,
                                           const solverMode &sMode)
{
    /* assume the output is a state and compute accordingly*/
    for (index_t kk = 0; kk < m_outputSize; ++kk)
    {
        index_t oLoc = getOutputLoc (sMode, kk);
        md.assignCheckCol (kk, oLoc, 1.0);
    }
}

count_t gridComponent::outputDependencyCount (index_t num, const solverMode &sMode) const
{
    /* assume the output is a state and act accordingly*/

    index_t oLoc = getOutputLoc (sMode, num);
    return (oLoc == kInvalidLocation) ? 0 : 1;
}

void gridComponent::preEx (const IOdata &inputs, const stateData &sD, const solverMode &sMode)
{
    for (auto &subobj : subObjectList)
    {
        if (!(subobj->checkFlag (preEx_requested)))
        {
            continue;
        }
        if (!subobj->checkFlag (separate_processing))
        {
            subobj->preEx (inputs, sD, sMode);
        }
    }
}

void gridComponent::residual (const IOdata &inputs, const stateData &sD, double resid[], const solverMode &sMode)
{
    for (auto &sub : subObjectList)
    {
        if (!sub->checkFlag (separate_processing))
        {
            if (sub->stateSize (sMode) > 0)
            {
                sub->residual (inputs, sD, resid, sMode);
            }
        }
    }
}

void gridComponent::derivative (const IOdata &inputs, const stateData &sD, double deriv[], const solverMode &sMode)
{
    for (auto &sub : subObjectList)
    {
        if (!sub->checkFlag (separate_processing))
        {
            if (sub->diffSize (sMode) > 0)
            {
                sub->derivative (inputs, sD, deriv, sMode);
            }
        }
    }
}

void gridComponent::algebraicUpdate (const IOdata &inputs,
                                  const stateData &sD,
                                  double update[],
                                  const solverMode &sMode,
                                  double alpha)
{
    for (auto &sub : subObjectList)
    {
        if (!sub->checkFlag (separate_processing))
        {
            if (sub->algSize (sMode) > 0)
            {
                sub->algebraicUpdate (inputs, sD, update, sMode, alpha);
            }
        }
    }
}

void gridComponent::jacobianElements (const IOdata &inputs,
                                   const stateData &sD,
                                   matrixData<double> &md,
                                   const IOlocs &inputLocs,
                                   const solverMode &sMode)
{
    for (auto &sub : subObjectList)
    {
        if (!sub->checkFlag (separate_processing))
        {
            if (sub->stateSize (sMode) > 0)
            {
                sub->jacobianElements (inputs, sD, md, inputLocs, sMode);
            }
        }
    }
}
void gridComponent::rootTest (const IOdata &inputs, const stateData &sD, double roots[], const solverMode &sMode)
{
    for (auto &subobj : subObjectList)
    {
        if (!subobj->checkFlag (separate_processing))
        {
            if (!(subobj->checkFlag (has_roots)))
            {
                continue;
            }
            subobj->rootTest (inputs, sD, roots, sMode);
        }
    }
}

void gridComponent::rootTrigger (coreTime time,
                              const IOdata &inputs,
                              const std::vector<int> &rootMask,
                              const solverMode &sMode)
{
    for (auto &subobj : subObjectList)
    {
        if (!(subobj->checkFlag (has_roots)))
        {
            continue;
        }
        if (!subobj->checkFlag (separate_processing))
        {
            subobj->rootTrigger (time, inputs, rootMask, sMode);
        }
    }
}

change_code
gridComponent::rootCheck (const IOdata &inputs, const stateData &sD, const solverMode &sMode, check_level_t level)
{
    auto ret = change_code::no_change;

    for (auto &subobj : subObjectList)
    {
        if (!(subobj->checkFlag (has_roots)))
        {
            continue;
        }
        if (!subobj->checkFlag (separate_processing))
        {
            ret = std::max (subobj->rootCheck (inputs, sD, sMode, level), ret);
        }
    }
    return ret;
}

double
gridComponent::getOutput (const IOdata & /*inputs*/, const stateData &sD, const solverMode &sMode, index_t outputNum) const
{
    if (outputNum >= m_outputSize)
    {
        return kNullVal;
    }
    Lp Loc = offsets.getLocations (sD, sMode, this);
    if (opFlags[differential_output])
    {
        if (Loc.diffSize > outputNum)
        {
			assert(Loc.diffStateLoc != nullptr);
            return Loc.diffStateLoc[outputNum];
        }
        return kNullVal;
    }
    // if differential flag was not specified try algebraic state values then differential

    if (Loc.algSize > outputNum)
    {
		assert(Loc.algStateLoc != nullptr);
        return Loc.algStateLoc[outputNum];
    }
    if (Loc.diffSize + Loc.algSize > outputNum)
    {
		assert(Loc.diffStateLoc != nullptr);
        return Loc.diffStateLoc[outputNum - Loc.algSize];
    }
    return (static_cast<index_t> (m_state.size ()) > outputNum) ? m_state[outputNum] : kNullVal;
}

double gridComponent::getOutput (index_t num) const
{
    return getOutput (noInputs, emptyStateData, cLocalSolverMode, num);
}

IOdata gridComponent::getOutputs (const IOdata &inputs, const stateData &sD, const solverMode &sMode) const
{
    IOdata mout (m_outputSize);
    for (count_t pp = 0; pp < m_outputSize; ++pp)
    {
        mout[pp] = getOutput (inputs, sD, sMode, pp);
    }
    return mout;
}

// static IOdata kNullVec;

double
gridComponent::getDoutdt (const IOdata & /*inputs*/, const stateData &sD, const solverMode &sMode, index_t num) const
{
    if (num >= m_outputSize)
    {
        return kNullVal;
    }
    Lp Loc = offsets.getLocations (sD, sMode, this);
    if (opFlags[differential_output])
    {
		assert(Loc.dstateLoc != nullptr);
        return Loc.dstateLoc[num];
    }

    if (Loc.algSize > num)
    {
        return 0.0;
    }
    if (Loc.diffSize + Loc.algSize > num)
    {
		assert(Loc.dstateLoc != nullptr);
        return Loc.dstateLoc[num - Loc.algSize];
    }
    return 0.0;
}

index_t gridComponent::getOutputLoc (const solverMode &sMode, index_t num) const
{
    if (num >= m_outputSize)
    {
        return kNullLocation;
    }

    if (opFlags[differential_output])
    {
        if (num < diffSize (sMode))
        {
            return offsets.getDiffOffset (sMode) + num;
        }
        num -= diffSize (sMode);
        return offsets.getAlgOffset (sMode) + num - diffSize (sMode);
    }

    const auto &so = offsets.getOffsets (sMode);
    if (so.total.algSize > num)
    {
        return so.algOffset + num;
    }
    if (so.total.diffSize + so.total.algSize > num)
    {
        return so.diffOffset - so.total.algSize + num;
    }

    return kNullLocation;
}

IOlocs gridComponent::getOutputLocs (const solverMode &sMode) const
{
    IOlocs oloc (m_outputSize);

    if (!isLocal (sMode))
    {
        for (count_t pp = 0; pp < m_outputSize; ++pp)
        {
            oloc[pp] = getOutputLoc (sMode, pp);
        }
    }
    else
    {
        for (count_t pp = 0; pp < m_outputSize; ++pp)
        {
            oloc[pp] = kNullLocation;
        }
    }
    return oloc;
}

void gridComponent::setParameter (index_t param, double /*value*/) { throw (unrecognizedParameter ("param"+std::to_string(param))); }
double gridComponent::getParameter (index_t param) const { throw (unrecognizedParameter ("param" + std::to_string(param))); }
void gridComponent::parameterPartialDerivatives (index_t param,
                                              double /*val*/,
                                              const IOdata & /*inputs*/,
                                              const stateData & /*sD*/,
                                              matrixData<double> & /*md*/,
                                              const solverMode & /*sMode*/)
{
    throw (unrecognizedParameter ("param" + std::to_string(param)));
}

double gridComponent::parameterOutputPartialDerivatives (index_t param,
                                                      double /*val*/,
                                                      index_t /*outputNum*/,
                                                      const IOdata & /*inputs*/,
                                                      const stateData & /*sD*/,
                                                      const solverMode & /*sMode*/)
{
    throw (unrecognizedParameter ("param" + std::to_string(param)));
}

void printStateNames (gridComponent *comp, const solverMode &sMode)
{
    std::vector<std::string> sNames;
    comp->getStateName (sNames, sMode);
    int kk = 0;
    for (auto &sn : sNames)
    {
        std::cout << kk++ << ' ' << sn << '\n';
    }
}

}  // namespace griddyn