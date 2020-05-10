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

#include "core/objectFactory.hpp"
#include "core/objectInterpreter.h"
#include "elementReaderTemplates.hpp"
#include "fileInput.h"
#include "formatInterpreters/readerElement.h"
#include "gmlc/utilities/stringOps.h"
#include "griddyn/gridBus.h"
#include "readElement.h"
#include "readerHelper.h"

namespace griddyn {
using namespace readerConfig;

static const IgnoreListType busIgnore{"area"};
static const std::string busComponentName = "bus";
// "aP" is the XML element passed from the reader
gridBus* readBusElement(std::shared_ptr<readerElement>& element,
                        readerInfo& ri,
                        coreObject* searchObject)
{
    gridParameter param;
    auto riScope = ri.newScope();

    // boiler plate code to setup the object from references or new object
    // check for the area field

    gridBus* bus = ElementReaderSetup(
        element, static_cast<gridBus*>(nullptr), busComponentName, ri, searchObject);

    std::string valType = getElementField(element, "type", defMatchType);
    if (!valType.empty()) {
        valType = ri.checkDefines(valType);
        auto cloc = valType.find_first_of(",;");
        if (cloc != std::string::npos) {
            std::string A = valType.substr(0, cloc);
            std::string B = valType.substr(cloc + 1);
            gmlc::utilities::stringOps::trimString(A);
            gmlc::utilities::stringOps::trimString(B);
            try {
                bus->set("type", A);
            }
            catch (const std::invalid_argument&) {
                WARNPRINT(READER_WARN_IMPORTANT, "Bus type parameter not found " << A);
            }
            try {
                bus->set("type", B);
            }
            catch (const std::invalid_argument&) {
                WARNPRINT(READER_WARN_IMPORTANT, "Bus type parameter not found " << B);
            }
        } else {
            try  // type can mean two different things to a bus -either the actual type of the bus
                 // object or the
            // state type of the bus this catch will
            // will dismabiguate them since in a majority of cases we are not changing the type of
            // the object only how it interprets the state
            {
                bus->set("type", valType);
            }
            catch (const std::invalid_argument&)  // either invalidParameterValue or
                                                  // unrecognizedParameter depending on the actual
                                                  // model used
            {
                if (!(coreObjectFactory::instance()->isValidType(busComponentName, valType))) {
                    WARNPRINT(READER_WARN_IMPORTANT, "Bus type parameter not found " << valType);
                }
            }
        }
    }
    loadElementInformation(bus, element, busComponentName, ri, busIgnore);

    LEVELPRINT(READER_NORMAL_PRINT, "loaded Bus " << bus->getName());

    ri.closeScope(riScope);
    return bus;
}

}  // namespace griddyn
