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

#include "core/coreExceptions.h"
#include "core/objectInterpreter.h"
#include "elementReaderTemplates.hpp"
#include "fileInput.h"
#include "formatInterpreters/readerElement.h"
#include "griddyn/Link.h"
#include "griddyn/gridBus.h"
#include "readElement.h"
#include "readerHelper.h"

namespace griddyn {
using namespace readerConfig;

static const IgnoreListType linkIgnoreElements{"to", "from"};
static const std::string linkComponentName = "link";
// aP is the link element
Link* readLinkElement(std::shared_ptr<readerElement>& element,
                      readerInfo& ri,
                      coreObject* searchObject,
                      bool warnlink)
{
    auto riScope = ri.newScope();

    // run the boilerplate code to setup the object
    Link* lnk = ElementReaderSetup(
        element, static_cast<Link*>(nullptr), linkComponentName, ri, searchObject);

    // from bus
    std::string busname = getElementField(element, "from", defMatchType);
    if (busname.empty()) {
        if (warnlink) {
            WARNPRINT(READER_WARN_IMPORTANT, "link must specify a 'from' bus");
        }
    } else if (searchObject != nullptr) {
        busname = ri.checkDefines(busname);
        auto obj = locateObject(busname, searchObject);
        auto bus = dynamic_cast<gridBus*>(obj);
        if (bus != nullptr) {
            try {
                lnk->updateBus(bus, 1);
            }
            catch (const objectAddFailure& oaf) {
                WARNPRINT(READER_WARN_IMPORTANT, "unable to load 'from' bus " << busname);
            }
        } else if (warnlink) {
            WARNPRINT(READER_WARN_IMPORTANT, "link must specify a 'from' bus");
        }
    }

    // to bus
    busname = getElementField(element, "to", defMatchType);
    if (busname.empty()) {
        if (warnlink) {
            WARNPRINT(READER_WARN_IMPORTANT, "link must specify a 'to' bus");
        }
    } else if (searchObject != nullptr) {
        busname = ri.checkDefines(busname);
        auto obj = locateObject(busname, searchObject);
        auto bus = dynamic_cast<gridBus*>(obj);
        if (bus != nullptr) {
            try {
                lnk->updateBus(bus, 2);
            }
            catch (const objectAddFailure& oaf) {
                WARNPRINT(READER_WARN_IMPORTANT, "unable to load 'to' bus " << busname);
            }
        } else if (warnlink) {
            WARNPRINT(READER_WARN_IMPORTANT, "link must specify a 'to' bus");
        }
    }

    // properties from link attributes

    loadElementInformation(lnk, element, linkComponentName, ri, linkIgnoreElements);

    LEVELPRINT(READER_NORMAL_PRINT, "loaded link " << lnk->getName());

    ri.closeScope(riScope);
    return lnk;
}

}  // namespace griddyn
