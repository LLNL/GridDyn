/*
 * Copyright (c) 2014-2020, Lawrence Livermore National Security
 * See the top-level NOTICE for additional details. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "elementReaderTemplates.hpp"
#include "formatInterpreters/readerElement.h"
#include "gmlc/utilities/stringConversion.h"
#include "griddyn/events/Event.h"
#include "readElement.h"
#include "readerHelper.h"
#include "units/units.hpp"
#include <iostream>

namespace griddyn {
using namespace readerConfig;

static const std::string eventNameString("event");

static const IgnoreListType eventIgnoreStrings{"file",
                                               "name",
                                               "column",
                                               "value",
                                               "va",
                                               "units",
                                               "description",
                                               "period",
                                               "time",
                                               "t",
                                               "offset",
                                               "units",
                                               "gain",
                                               "bias",
                                               "field",
                                               "target",
                                               "type"};

void readEventElement(std::shared_ptr<readerElement>& aP,
                      EventInfo& gdEI,
                      readerInfo& ri,
                      coreObject* obj)
{
    using namespace gmlc::utilities;
    if (aP->getName() != "event") {
        if (aP->getName() != "scenario")
        {
            gdEI.type = aP->getName();
        }
    }
    // get the event strings that may be present
    auto eventString = aP->getMultiText(", ");
    if (!eventString.empty()) {
        gdEI.loadString(eventString, obj);
    }

    // check for the field attributes
    std::string name = getElementField(aP, "name", defMatchType);
    if (!name.empty()) {
        name = ri.checkDefines(name);
        gdEI.name = name;
    }

    // check for the field attributes
    std::string type = getElementField(aP, "type", defMatchType);
    if (!type.empty()) {
        gdEI.type = ri.checkDefines(type);
    }

    // check for the field attributes
    auto targetList = getElementFieldMultiple(aP, "target", defMatchType);
    for (auto& ss : targetList) {
        ss = ri.checkDefines(ss);
        gdEI.targetObjs.push_back(locateObject(ss, obj));
    }

    auto fieldList = getElementFieldMultiple(aP, "field", defMatchType);
    for (auto& ss : fieldList) {
        ss = ri.checkDefines(ss);
        gdEI.fieldList.push_back(ss);
    }
    auto unitList = getElementFieldMultiple(aP, "units", defMatchType);
    for (auto& ss : unitList) {
        ss = ri.checkDefines(ss);
        gdEI.units.push_back(units::unit_cast_from_string(ss));
    }

    gdEI.description = getElementField(aP, "description", defMatchType);

    std::string field = getElementField(aP, "period", defMatchType);
    if (!field.empty()) {
        gdEI.period = interpretString(field, ri);
    }

    field = getElementFieldOptions(aP, {"t", "time"}, defMatchType);
    if (!field.empty()) {
        gdEI.time = str2vector<coreTime>(ri.checkDefines(field), negTime);
    }
    else
    {
        if (gdEI.time.empty() && aP->getName() == "scenario")
        {
            gdEI.time.push_back(-1.0);
        }
    }

    field = getElementFieldOptions(aP, {"value", "val"}, defMatchType);
    if (!field.empty()) {
        gdEI.value = str2vector(ri.checkDefines(field), kNullVal);
    }

    name = getElementField(aP, "file", defMatchType);
    if (!name.empty()) {
        ri.checkFileParam(name);
        gdEI.file = name;
    }

    field = getElementField(aP, "column", defMatchType);
    if (!field.empty()) {
        gdEI.columns.push_back(static_cast<int>(interpretString(field, ri)));
    }
}

int loadEventElement(std::shared_ptr<readerElement>& element, coreObject* obj, readerInfo& ri)
{
    int ret = FUNCTION_EXECUTION_SUCCESS;
    element->bookmark();
    EventInfo gdEI;
    readEventElement(element, gdEI, ri, obj);
    auto gdE = make_event(gdEI, obj);
    if (!gdE) {
        WARNPRINT(READER_WARN_IMPORTANT, "unable to create an event of type " << gdEI.type);
        return FUNCTION_EXECUTION_FAILURE;
    }
    setAttributes(gdE.get(), element, eventNameString, ri, eventIgnoreStrings);
    setParams(gdE.get(), element, eventNameString, ri, eventIgnoreStrings);

    if (!(gdE->isArmed())) {
        WARNPRINT(READER_WARN_IMPORTANT, "event for " << gdEI.name << ":unable to load event");
        ret = FUNCTION_EXECUTION_FAILURE;
    } else {
        auto owner = gdE->getOwner();
        if (owner != nullptr) {  // check if the event has a specified owner
            auto spE = std::shared_ptr<Event>(std::move(gdE));
            try {  // this could throw in which case we can't move the object into it first
                owner->addHelper(spE);
            }
            catch (const objectAddFailure&) {
                WARNPRINT(READER_WARN_IMPORTANT,
                          "Event: " << spE->getName() << " unable to be added to "
                                    << owner->getName());
                ri.events.push_back(std::move(spE));
            }
        } else {  // if it doesn't just put it on the stack and deal with it later
            ri.events.push_back(std::move(gdE));
        }
    }
    element->restore();
    return ret;
}

}  // namespace griddyn
