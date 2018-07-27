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

#include "readElement.h"

#include "elementReaderTemplates.hpp"
#include "readerHelper.h"
#include "utilities/string_viewOps.h"
#include <cmath>

namespace griddyn
{
using namespace readerConfig;

static const IgnoreListType ignoreConditionVariables{"condition"};
bool checkCondition (utilities::string_view cond, readerInfo &ri, coreObject *parentObject);
// "aP" is the XML element passed from the reader
void loadConditionElement (std::shared_ptr<readerElement> &element, readerInfo &ri, coreObject *parentObject)
{
    auto riScope = ri.newScope ();

    loadDefines (element, ri);
    loadDirectories (element, ri);

    bool eval = false;
    std::string condString = getElementField (element, "condition", defMatchType);

    if (!condString.empty ())
    {
        // deal with &gt for > and &lt for < if necessary
        condString = stringOps::xmlCharacterCodeReplace (condString);
        eval = checkCondition (condString, ri, parentObject);
    }
    else
    {
        WARNPRINT (READER_WARN_IMPORTANT, "no condition specified");
        eval = false;
    }

    if (eval)
    {
        readImports (element, ri, parentObject, false);
        // load the subobjects of gen
        loadSubObjects (element, ri, parentObject);
        // get all element fields
        paramLoopElement (parentObject, element, "local", ri, ignoreConditionVariables);
        readImports (element, ri, parentObject, true);
    }

    ri.closeScope (riScope);
}

template <class X>
bool compare (const X &val1, const X &val2, char op1, char op2)
{
    switch (op1)
    {
    case '>':
        return (op2 == '=') ? (val1 >= val2) : (val1 > val2);
    case '<':
        return (op2 == '=') ? (val1 <= val2) : (val1 < val2);
    case '!':
        return (val1 != val2);
    case '=':
        return (val1 == val2);
    default:
        throw (std::invalid_argument ("invalid comparison operator"));
    }
}

bool checkCondition (utilities::string_view cond, readerInfo &ri, coreObject *parentObject)
{
    using utilities::string_view;
    using namespace utilities::string_viewOps;
    trim (cond);
    bool rev = false;
    if ((cond[0] == '!') || (cond[0] == '~'))
    {
        rev = true;
        cond = cond.substr (1, string_view::npos);
    }
    size_t pos = cond.find_first_of ("><=!");
    bool eval = false;
    char A, B;
    string_view BlockA, BlockB;

    if (pos == string_view::npos)
    {
        A = '!';
        B = '=';
        BlockA = trim (cond);
        BlockB = "0";
        if (BlockA.compare (0, 6, "exists") == 0)
        {
            auto a1 = BlockA.find_first_of ('(', 6);
            auto a2 = BlockA.find_last_of (')');
            auto check = BlockA.substr (a1 + 1, a2 - a1 - 1);
            coreObject *obj = locateObject (check.to_string (), parentObject, false);
            return (rev) ? (obj == nullptr) : (obj != nullptr);
        }
    }
    else
    {
        A = cond[pos];
        B = cond[pos + 1];
        BlockA = trim (cond.substr (0, pos));
        BlockB = (B == '=') ? trim (cond.substr (pos + 2)) : trim (cond.substr (pos + 1));
    }

    ri.setKeyObject (parentObject);
    double aval = interpretString (BlockA.to_string (), ri);
    double bval = interpretString (BlockB.to_string (), ri);

    if (!std::isnan (aval) && !std::isnan (bval))
    {
        try
        {
            eval = compare (aval, bval, A, B);
        }
        catch (const std::invalid_argument &)
        {
            WARNPRINT (READER_WARN_IMPORTANT, "invalid comparison operator");
        }
    }
    else if (std::isnan (aval) && (std::isnan (bval)))
    {  // do a string comparison
        std::string astr = ri.checkDefines (BlockA.to_string ());
        std::string bstr = ri.checkDefines (BlockB.to_string ());

        try
        {
            eval = compare (astr, bstr, A, B);
        }
        catch (const std::invalid_argument &)
        {
            WARNPRINT (READER_WARN_IMPORTANT, "invalid comparison terms");
        }
    }
    else
    {  // mixed string and number comparison
        WARNPRINT (READER_WARN_IMPORTANT, "invalid comparison terms");
    }

    ri.setKeyObject (nullptr);
    return (rev) ? !eval : eval;
}

}  // namespace griddyn
