/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil;  eval: (c-set-offset 'innamespace 0); -*-
 */
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

#include "fncsCollector.h"

#include "core/helperTemplates.h"
#include "fncsLibrary.h"
#include "fncsSupport.h"
#include "stringOps.h"

fncsCollector::fncsCollector(coreTime time0, coreTime period): collector(time0, period) {}

fncsCollector::fncsCollector(const std::string& collectorName): collector(collectorName) {}

fncsCollector::~fncsCollector() {}

std::shared_ptr<collector> fncsCollector::clone(std::shared_ptr<collector> gr) const
{
    auto nrec = cloneBase<fncsCollector, collector>(this, gr);
    if (!nrec) {
        return gr;
    }

    return nrec;
}

void fncsCollector::dataPointAdded(const collectorPoint& cp)
{
    if (!cp.colname.empty()) {
        fncsRegister::instance()->registerPublication(cp.colname,
                                                      fncsRegister::dataType::fncsDouble);
    }
}

change_code fncsCollector::trigger(coreTime time)
{
    auto out = collector::trigger(time);

    auto colNames = getColumnDescriptions();
    std::vector<bool> subscribe(colNames.size(), true);

    for (size_t ii = 0; ii < complexPairs.size(); ++ii) {
        auto& n1 = complexPairs[ii].first;
        auto& n2 = complexPairs[ii].second;
        int index1 = -1;
        int index2 = -1;
        for (int pp = 0; pp < static_cast<int>(colNames.size()); ++pp) {
            if (n1 == colNames[pp]) {
                index1 = pp;
            }
            if (n2 == colNames[pp]) {
                index2 = pp;
            }
        }
        if ((index1 >= 0) && (index2 >= 0)) {
            subscribe[index1] = false;
            subscribe[index2] = false;
        }
        fncsSendComplex(cnames[ii], data[index1], data[index2]);
    }

    for (size_t ii = 0; ii < data.size(); ++ii) {
        if (subscribe[ii]) {
            fncsSendVal(colNames[ii], data[ii]);
        }
    }
    return out;
}

void fncsCollector::set(const std::string& param, double val)
{
    collector::set(param, val);
}

void fncsCollector::set(const std::string& param, const std::string& val)
{
    using namespace stringOps;
    if (param == "complex") {
        auto asLoc = val.find("as");
        cnames.push_back(trim(val.substr(asLoc + 2)));
        auto commaLoc = val.find_first_of(',');
        complexPairs.emplace_back(trim(val.substr(0, commaLoc)),
                                  trim(val.substr(commaLoc + 1, asLoc - 1 - commaLoc)));
        fncsRegister::instance()->registerPublication(cnames.back(),
                                                      fncsRegister::dataType::fncsComplex);
    } else {
        collector::set(param, val);
    }
}

const std::string fncsName("fncs");

const std::string& fncsCollector::getSinkName() const
{
    return fncsName;
}
