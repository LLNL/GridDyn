/*
 * Copyright (c) 2014-2020, Lawrence Livermore National Security
 * See the top-level NOTICE for additional details. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "../Load.h"

#include "../gridBus.h"
#include "../measurement/objectGrabbers.h"
#include "core/coreExceptions.h"
#include "core/coreObjectTemplates.hpp"
#include "utilities/matrixData.hpp"
#include <cmath>
#include <complex>
#include <iostream>

namespace griddyn {
using namespace units;

std::atomic<count_t> Load::loadCount(0);
Load::Load(const std::string& objName): gridSecondary(objName)
{
    constructionHelper();
}
Load::Load(double rP, double rQ, const std::string& objName): gridSecondary(objName), P(rP), Q(rQ)
{
    constructionHelper();
}

void Load::constructionHelper()
{
    // default values
    setUserID(++loadCount);
    updateName();
}

coreObject* Load::clone(coreObject* obj) const
{
    auto nobj = cloneBase<Load, gridSecondary>(this, obj);
    if (nobj == nullptr) {
        return obj;
    }
    nobj->setLoad(P,
                  Q);  // use the set load function in case we are cloning from a basic object to a
                       // higher level object
    nobj->pfq = pfq;
    return nobj;
}

void Load::setLoad(double level, unit unitType)
{
    setP(convert(level, unitType, puMW, systemBasePower));
}

void Load::setLoad(double Plevel, double Qlevel, unit unitType)
{
    std::cout << "Load::setLoad start" << std::endl;
    setP(convert(Plevel, unitType, puMW, systemBasePower));
    setQ(convert(Qlevel, unitType, puMW, systemBasePower));
    std::cout << "Load::setLoad end" << std::endl;
}

static const stringVec locNumStrings{"p", "q", "pf"};

static const stringVec locStrStrings{

};

static const stringVec flagStrings{"usepowerfactor"};

void Load::getParameterStrings(stringVec& pstr, paramStringType pstype) const
{
    getParamString<Load, gridComponent>(
        this, pstr, locNumStrings, locStrStrings, flagStrings, pstype);
}

void Load::setFlag(const std::string& flag, bool val)
{
    if (flag == "usepowerfactor") {
        if (val) {
            if (!(opFlags[use_power_factor_flag])) {
                opFlags.set(use_power_factor_flag);
                updatepfq();
            }
        } else {
            opFlags.reset(use_power_factor_flag);
        }
    } else {
        gridSecondary::setFlag(flag, val);
    }
}

// set properties
void Load::set(const std::string& param, const std::string& val)
{
    if (param[0] == '#') {
    } else {
        gridSecondary::set(param, val);
    }
}

double Load::get(const std::string& param, unit unitType) const
{
    double val = kNullVal;
    if (param.length() == 1) {
        switch (param[0]) {
            case 'p':
                val = convert(P, puMW, unitType, systemBasePower, localBaseVoltage);
                break;
            case 'q':
                val = convert(Q, puMW, unitType, systemBasePower, localBaseVoltage);
                break;
            default:
                break;
        }
        return val;
    }

    if (param == "pf") {
        val = pfq;
    } else if (auto fptr = getObjectFunction(this, param).first) {
        auto unit = getObjectFunction(this, param).second;
        coreObject* tobj = const_cast<Load*>(this);
        val = convert(fptr(tobj), unit, unitType, systemBasePower, localBaseVoltage);
    } else {
        val = gridSecondary::get(param, unitType);
    }
    return val;
}

void Load::set(const std::string& param, double val, unit unitType)
{
    std::cout << "Load::set start" << std::endl;
    if (param.empty()) {
        std::cout << "Load::set end" << std::endl;
        return;
    }
    if (param.length() == 1) {
        switch (param.front()) {
            case 'p':
                setP(convert(val, unitType, puMW, systemBasePower, localBaseVoltage));
                break;
            case 'q':
                std::cout << "Load::set case q setQ()" << std::endl;
                setQ(convert(val, unitType, puMW, systemBasePower, localBaseVoltage));
                break;
            default:
                std::cout << "Load::set throw" << std::endl;
                throw(unrecognizedParameter(param));
        }
        checkFaultChange();
        std::cout << "Load::set end" << std::endl;
        return;
    }
    if (param.empty()) {
        std::cout << "Load::set end" << std::endl;
        return;
    }
    if (param.back() == '+')  // load increments
    {
        // load increments  allows a delta on the load through the set functions
        if (param == "p+") {
            P += convert(val, unitType, puMW, systemBasePower, localBaseVoltage);
            checkpfq();
        } else if (param == "q+") {
            std::cout << "Load::set q+" << std::endl;
            Q += convert(val, unitType, puMW, systemBasePower, localBaseVoltage);
            std::cout << "Q = " << Q << std::endl;
            updatepfq();
        } else {
            gridSecondary::set(param, val, unitType);
        }
    } else if (param.back() == '*') {
        // load increments  allows a delta on the load through the set functions
        if (param == "p*") {
            P *= val;
            checkpfq();
        } else if (param == "q*") {
            std::cout << "Load::set q*" << std::endl;
            Q *= val;
            std::cout << "Q = " << Q << std::endl;
            updatepfq();
        } else {
            gridSecondary::set(param, val, unitType);
        }
    } else if (param == "load p") {
        setP(convert(val, unitType, puMW, systemBasePower, localBaseVoltage));
    } else if (param == "load q") {
        std::cout << "Load::set load q setQ()" << std::endl;
        setQ(convert(val, unitType, puMW, systemBasePower, localBaseVoltage));
    } else if ((param == "pf") || (param == "powerfactor")) {
        if (val != 0.0) {
            if (std::abs(val) <= 1.0) {
                pfq = std::sqrt(1.0 - val * val) / val;
            } else {
                pfq = 0.0;
            }
        } else {
            pfq = kBigNum;
        }
        opFlags.set(use_power_factor_flag);
    } else if (param == "qratio") {
        pfq = val;
        opFlags.set(use_power_factor_flag);
    } else {
        gridSecondary::set(param, val, unitType);
    }
    std::cout << "Load::set end" << std::endl;
}

void Load::setP(double newP)
{
    P = newP;
    checkpfq();
    checkFaultChange();
}

void Load::setQ(double newQ)
{
    std::cout << "Load::setQ start" << std::endl;
    Q = newQ;
    std::cout << "Q = " << Q << std::endl;
    updatepfq();
    checkFaultChange();
    std::cout << "Load::setQ end" << std::endl;
}

void Load::updatepfq()
{
    std::cout << "Load::updatepfq start" << std::endl;
    if (opFlags[use_power_factor_flag]) {
        pfq = (P == 0.0) ? kBigNum : Q / P;
    }
    std::cout << "pfq = " << pfq << std::endl;
    std::cout << "Load::updatepfq end" << std::endl;
}

void Load::checkpfq()
{
    std::cout << "Load::checkpfq start" << std::endl;
    if (opFlags[use_power_factor_flag]) {
        if (pfq > 1000.0)  // if the pfq is screwy, recalculate, otherwise leave it the same.
        {
            if (P != 0.0) {
                pfq = Q / P;
            }
        }
    }
    std::cout << "pfq = " << pfq << std::endl;
    std::cout << "Load::checkpfq end" << std::endl;
}

void Load::checkFaultChange()
{
    if ((opFlags[pFlow_initialized]) && (bus->getVoltage() < 0.05)) {
        alert(this, POTENTIAL_FAULT_CHANGE);
    }
}

double Load::getRealPower() const
{
    return P;
}
double Load::getReactivePower() const
{
    std::cout << "Load::getReactivePower() start" << std::endl;
    double tmp = Q;
    std::cout << "Load::getReactivePower() end" << std::endl;
    return tmp;
}
double Load::getRealPower(const IOdata& /*inputs*/,
                          const stateData& /*sD*/,
                          const solverMode& /*sMode*/) const
{
    return getRealPower();
}

double Load::getReactivePower(const IOdata& /*inputs*/,
                              const stateData& /*sD*/,
                              const solverMode& /*sMode*/) const
{
    std::cout << "Load::getReactivePower(...) start" << std::endl;
    double tmp = getReactivePower();
    std::cout << "Load::getReactivePower(...) end" << std::endl;
    return tmp;
}

double Load::getRealPower(const double /*V*/) const
{
    return getRealPower();
}
double Load::getReactivePower(double /*V*/) const
{
    std::cout << "Load::getReactivePower(V) start" << std::endl;
    double tmp = getReactivePower();
    std::cout << "Load::getReactivePower(V) end" << std::endl;
    return tmp;
}
count_t Load::outputDependencyCount(index_t /*num*/, const solverMode& /*sMode*/) const
{
    return 0;
}
}  // namespace griddyn
