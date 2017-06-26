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

#ifndef GRABBER_INTERPRETER_H_
#define GRABBER_INTERPRETER_H_

#include "utilities/stringConversion.h"

#include "core/objectInterpreter.h"
#include "utilities/functionInterpreter.h"
#include <cmath>
#include <functional>
#include <memory>
#include <type_traits>

namespace griddyn
{
const std::string mdiv1String = "*/^";
const std::string mdiv2String = "*%^";
const std::string psubString = "+-";

template <class baseX, class opX, class funcX>
class grabberInterpreter
{
    static_assert (std::is_base_of<baseX, opX>::value,
                   "Operations class and base class must have a parent child relationship");
    static_assert (std::is_base_of<baseX, funcX>::value,
                   "functions class and base class must have a parent child relationship");

  private:
    std::function<std::unique_ptr<baseX> (const std::string &, coreObject *)> createX;

  public:
    explicit grabberInterpreter (std::function<std::unique_ptr<baseX> (const std::string &, coreObject *)> fc)
        : createX (fc)
    {
    }
    std::unique_ptr<baseX> interpretGrabberBlock (const std::string &command, coreObject *obj)
    {
        // this is to resolve an issue with URI specified parameters and the division operator but still allow
        // normal division operator in most cases  for URI specified the % must be used for division or the
        // function form
        const std::string &mdivstr = (command.find_last_of ('?') == std::string::npos) ? mdiv1String : mdiv2String;
        std::unique_ptr<baseX> ggb = nullptr;
        std::unique_ptr<baseX> ggbA = nullptr;

        // check for functions
        size_t endParenthesisLocation = std::string::npos;
        size_t firstParenthesisLocation = command.find_first_of ('(', 0);

        if (firstParenthesisLocation != std::string::npos)
        {
            endParenthesisLocation = pChunckEnd (command, firstParenthesisLocation + 1);
            if (endParenthesisLocation == std::string::npos)
            {
                endParenthesisLocation = command.length ();
            }
        }
        size_t operatorLocation;
        if ((firstParenthesisLocation == 0) && (endParenthesisLocation == command.length () - 1))
        {
            ggb = interpretGrabberBlock (command.substr (1, endParenthesisLocation - 1), obj);
        }
        else if (((operatorLocation = command.find_first_of (psubString, 1)) != std::string::npos) &&
                 (operatorLocation < firstParenthesisLocation))
        {
            ggb = addSubGrabberBlocks (command, obj, operatorLocation);
        }
        else if (((operatorLocation = command.find_first_of (mdivstr, 1)) != std::string::npos) &&
                 (operatorLocation < firstParenthesisLocation))
        {
            ggb = multDivGrabberBlocks (command, obj, operatorLocation);
        }
        else if ((operatorLocation = command.find_first_of (psubString, endParenthesisLocation + 1)) !=
                 std::string::npos)
        {
            ggb = addSubGrabberBlocks (command, obj, operatorLocation);
        }
        else if ((operatorLocation = command.find_first_of (mdivstr, endParenthesisLocation + 1)) !=
                 std::string::npos)
        {
            ggb = multDivGrabberBlocks (command, obj, operatorLocation);
        }
        else
        {
            if (firstParenthesisLocation != std::string::npos)
            {
                std::string cmdBlock = command.substr (0, firstParenthesisLocation);
                if (isFunctionName (cmdBlock))
                {
                    std::string fcallstr =
                      stringOps::trim (command.substr (firstParenthesisLocation + 1,
                                                       endParenthesisLocation - firstParenthesisLocation - 1));
                    ggbA = interpretGrabberBlock (fcallstr, obj);
                    if (ggbA)
                    {
                        ggb = std::make_unique<funcX> (std::move (ggbA), cmdBlock);
                    }
                }
                else  // not a function call so must be units description
                {
                    ggb = singleBlockInterpreter (command, obj);
                }
            }
            else
            {
                ggb = singleBlockInterpreter (command, obj);
            }
        }

        return ggb;
    }

  private:
    std::unique_ptr<baseX> singleBlockInterpreter (const std::string &command, coreObject *obj)
    {
        coreObject *mobj = obj;
        std::string field (command);
        gridUnits::units_t outUnit = gridUnits::defUnit;
        // get the object which to grab from
        size_t fieldSeperatorLocation = command.find_last_of (":?");
        if (fieldSeperatorLocation != std::string::npos)
        {
            mobj = locateObject (command, obj);
            if (mobj == nullptr)
            {
                return nullptr;
            }
            field = command.substr (fieldSeperatorLocation + 1, std::string::npos);
        }

        size_t openParenthesisLocation = field.find_first_of ('(');
        if (openParenthesisLocation != std::string::npos)
        {
            size_t closeParenthesisLocation = field.find_first_of (')', openParenthesisLocation);
            std::string unitName =
              field.substr (openParenthesisLocation + 1, closeParenthesisLocation - openParenthesisLocation - 1);
            outUnit = gridUnits::getUnits (unitName);
            field = field.substr (0, openParenthesisLocation);
        }
        std::unique_ptr<baseX> ggb = createX (field, mobj);
        if (ggb)
        {
            ggb->outputUnits = outUnit;
        }

        return ggb;
    }

    std::unique_ptr<baseX> addSubGrabberBlocks (const std::string &command, coreObject *obj, size_t rlc)
    {
        using namespace stringOps;
        std::unique_ptr<baseX> ggb = nullptr;
        std::string Ablock = command.substr (0, rlc);
        trimString (Ablock);
        char op = command[rlc];
        std::string Bblock = command.substr (rlc + 1, std::string::npos);
        trimString (Bblock);
        // check if either Ablock or Bblock is a constant
        double valA = numeric_conversionComplete (Ablock, kNullVal);
        std::unique_ptr<baseX> ggbA = (valA == kNullVal) ? interpretGrabberBlock (Ablock, obj) : nullptr;

        double valB = numeric_conversionComplete (Bblock, kNullVal);
        std::unique_ptr<baseX> ggbB = (valB == kNullVal) ? interpretGrabberBlock (Bblock, obj) : nullptr;

        if (ggbA)  // we know Ablock is std::make_shared<grabber>
        {
            if (ggbB)  // both are grabber blocks
            {
                ggb = std::make_unique<opX> (std::move (ggbA), std::move (ggbB), command.substr (rlc, 1));
            }
            else if (valB != kNullVal)
            {
                ggb = std::move (ggbA);
                if (op == '+')
                {
                    ggb->bias = valB;
                }
                else
                {
                    ggb->bias = -valB;
                }
            }
            else
            {
                return nullptr;  // we can't interpret this
            }
        }
        else if (valA != kNullVal)
        {
            if (ggbB)  // both are grabber blocks
            {
                ggb = std::move (ggbB);
                ggb->bias = valA;
                if (op == '-')
                {
                    ggb->gain = -1.0;
                }
            }
            else if (valB != kNullVal)  // both are numeric
            {
                ggb = std::make_unique<baseX> ("constant", obj);
                if (op == '+')
                {
                    ggb->bias = valA + valB;
                }
                else
                {
                    ggb->bias = valA - valB;
                }
            }
            else
            {
                return nullptr;  // we can't interpret this
            }
        }
        else
        {
            return nullptr;
        }
        return ggb;
    }

    std::unique_ptr<baseX> multDivGrabberBlocks (const std::string &command, coreObject *obj, size_t rlc)
    {
        using namespace stringOps;
        std::unique_ptr<baseX> ggb = nullptr;

        std::string Ablock = command.substr (0, rlc);
        trimString (Ablock);
        char op = command[rlc];
        std::string Bblock = command.substr (rlc + 1, std::string::npos);
        trimString (Bblock);
        // check if either Ablock or Bblock is a constant
        double valA = numeric_conversionComplete (Ablock, kNullVal);
        std::unique_ptr<baseX> ggbA = (valA == kNullVal) ? interpretGrabberBlock (Ablock, obj) : nullptr;

        double valB = numeric_conversionComplete (Bblock, kNullVal);
        std::unique_ptr<baseX> ggbB = (valB == kNullVal) ? interpretGrabberBlock (Bblock, obj) : nullptr;

        if (ggbA)  // we know Ablock is std::make_shared<grabber
        {
            if (ggbB)  // both are grabber blocks
            {
                ggb = std::make_unique<opX> (std::move (ggbA), std::move (ggbB), command.substr (rlc, 1));
            }
            else if (valB != kNullVal)
            {
                if (op == '*')
                {
                    ggb = std::move (ggbA);
                    ggb->gain = valB;
                }
                else if ((op == '/') || (op == '%'))
                {
                    ggb = std::move (ggbA);
                    ggb->gain = 1.0 / valB;
                }
                else
                {
                    // set up power function
                    ggbB = std::make_unique<baseX> ("constant", obj);
                    ggbB->bias = valB;
                    ggb = std::make_unique<opX> (std::move (ggbA), std::move (ggbB), command.substr (rlc, 1));
                }
            }
            else
            {
                return nullptr;  // we can't interpret this
            }
        }
        else if (valA != kNullVal)
        {
            if (ggbB)  // B is a block
            {
                if (op == '*')
                {
                    ggb = std::move (ggbB);
                    ggb->gain = valA;
                }
                else
                {
                    // set up division by the grabber not a constant
                    ggbA = std::make_unique<baseX> ("constant", obj);
                    ggbA->bias = valA;
                    ggb = std::make_unique<opX> (std::move (ggbA), std::move (ggbB), command.substr (rlc, 1));
                }
            }
            else if (valB != kNullVal)  // both are numeric
            {
                ggb = std::make_unique<baseX> ("constant", obj);
                if (op == '*')
                {
                    ggb->bias = valA * valB;
                }
                else if ((op == '/') || (op == '%'))
                {
                    if (valB != 0)
                    {
                        ggb->bias = valA / valB;
                    }
                    else
                    {
                        ggb->bias = kNullVal;
                    }
                }
                else
                {
                    ggb->bias = pow (valA, valB);
                }
            }
            else
            {
                return nullptr;  // we can't interpret this
            }
        }
        else
        {
            return nullptr;
        }
        return ggb;
    }

    // TODO: merge this function with a version in the stringOps library
    size_t pChunckEnd (const std::string &cmd, size_t start)
    {
        int open = 1;
        size_t rlc = start;
        while (open > 0)
        {
            rlc = cmd.find_first_of ("()", rlc + 1);
            if (rlc == std::string::npos)
            {
                break;
            }
            if (cmd[rlc] == '(')
            {
                ++open;
            }
            else
            {
                --open;
            }
        }
        return rlc;
    }
};

}  // namespace griddyn
#endif
