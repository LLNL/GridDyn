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

#include "elementReaderTemplates.hpp"
#include "fileInput.h"
#include "readElement.h"
#include "readerHelper.h"

#include "utilities/stringConversion.h"
#include <numeric>

namespace griddyn
{
using namespace readerConfig;

int readElementInteger (std::shared_ptr<readerElement> &element,
                        const std::string &name,
                        readerInfo &ri,
                        int defValue);

static const IgnoreListType ignoreArrayVariables{"count", "loopvariable", "interval", "start", "stop"};
// "aP" is the XML element passed from the reader
void readArrayElement (std::shared_ptr<readerElement> &element, readerInfo &ri, coreObject *parentObject)
{
    auto riScope = ri.newScope ();
    std::vector<int> indices;

    loadDefines (element, ri);
    loadDirectories (element, ri);
    // loop through the other children
    //  cd = aP->FirstChildElement (false);
    std::string lvar = getElementField (element, "loopvariable", defMatchType);
    if (lvar.empty ())
    {
        lvar = "#index";
    }

	ri.setKeyObject(parentObject);
    int count = readElementInteger (element, "count", ri, -1);
    int start = readElementInteger (element, "start", ri, 1);
    int stop = readElementInteger (element, "stop", ri, -1);
    int interval = readElementInteger (element, "interval", ri, 1);
	ri.setKeyObject(nullptr);
    if (count > 0)
    {
        indices.resize (count);
        if (interval == 1)
        {
            std::iota (indices.begin (), indices.end (), start);
        }
        else
        {
            int val = start;
            for (auto &iv : indices)
            {
                iv = val;
                val += interval;
            }
        }
    }
    else
    {
        if (stop > start)
        {
            int val = start;
            while (val <= stop)
            {
                indices.push_back (val);
                val += interval;
            }
        }
        else
        {
            WARNPRINT (READER_WARN_IMPORTANT, "unable to create array");
            return;
        }
    }

    // fill the vector

    for (auto ind : indices)
    {
        ri.addDefinition (lvar, std::to_string (ind));
        loadElementInformation (parentObject, element, "array", ri, ignoreArrayVariables);
    }

    ri.closeScope (riScope);
}

int readElementInteger (std::shared_ptr<readerElement> &element,
                        const std::string &name,
                        readerInfo &ri,
                        int defValue)
{
    int ret = defValue;
    auto strVal = getElementField (element, name, defMatchType);
    if (strVal.empty ())
    {
        return ret;
    }

    ret = numeric_conversionComplete<int> (strVal, -kBigINT);
    if (ret == -kBigINT)  // we have a more complicated string
    {
        double val = interpretString (strVal, ri);
        if ((val > 0) && (static_cast<int> (val) < kBigINT))
        {
            ret = static_cast<int> (val);
        }
        else
        {
            ret = defValue;
            WARNPRINT (READER_WARN_IMPORTANT, "Unable to interpret start variable");
        }
    }

    return ret;
}

}//namespace griddyn