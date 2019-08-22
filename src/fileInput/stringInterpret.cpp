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

#include "core/objectInterpreter.h"
#include "fileInput.h"
#include "utilities/functionInterpreter.h"
#include "gmlc/utilities/stringConversion.h"
#include "utilities/string_viewConversion.h"
#include <cctype>
#include <cmath>

namespace griddyn
{
using namespace utilities::string_viewOps;

double interpretStringBlock (string_view command, readerInfo &ri);

void interpretStringBlock (string_view command, readerInfo &ri, std::vector<double> &outputs);

double addSubStringBlocks (string_view command, readerInfo &ri, size_t rlc);
double multDivStringBlocks (string_view command, readerInfo &ri, size_t rlc);
size_t pChunckEnd (string_view command, size_t start);

double InterpretFunction (const std::string &command, readerInfo &ri);
double InterpretFunction (const std::string &command, double val, readerInfo &ri);
double InterpretFunction (const std::string &command, double val1, double val2, readerInfo &ri);

double stringBlocktoDouble (string_view block, readerInfo &ri);
double interpretString_sv (string_view command, readerInfo &ri);

double ObjectQuery (string_view command, coreObject *obj);

double interpretString (const std::string &command, readerInfo &ri) { return interpretString_sv (command, ri); }
double interpretString_sv (string_view command, readerInfo &ri)
{
    size_t rlc, rlcp = 0;
    // check for functions
    auto rlcps = command.find_first_of ('(', 0);
    if (rlcps != std::string::npos)
    {
        rlcp = pChunckEnd (command, rlcps + 1);
        if (rlcp == std::string::npos)
        {
            rlcp = command.length ();
        }
    }
    double val = 0;
    if ((rlcps == 0) && (rlcp == command.length () - 1))
    {  // just remove outer parenthesis and call again
        val = interpretString_sv (command.substr (1, rlcp - 1), ri);
    }
    else if (((rlc = command.find_first_of ("+-", 1)) != std::string::npos) && (rlc < rlcps))
    {
        val = addSubStringBlocks (command, ri, rlc);
    }
    else if (((rlc = command.find_first_of ("*/^%", 1)) != std::string::npos) && (rlc < rlcps))
    {
        val = multDivStringBlocks (command, ri, rlc);
    }
    else if ((rlc = command.find_first_of ("+-", rlcp + 1)) != std::string::npos)
    {
        val = addSubStringBlocks (command, ri, rlc);
    }
    else if ((rlc = command.find_first_of ("*/^%", rlcp + 1)) != std::string::npos)
    {
        val = multDivStringBlocks (command, ri, rlc);
    }
    else
    {
        if (rlcps != std::string::npos)
        {
            auto cmdBlock = command.substr (0, rlcps);

            auto fcallstr = trim (command.substr (rlcps + 1, rlcp - rlcps - 1));
            if (fcallstr.empty ())
            {
                val = InterpretFunction (cmdBlock.to_string (), ri);
            }
            else
            {
                auto cloc = fcallstr.find_first_of (',');
                if (cloc != std::string::npos)
                {
                    auto args = splitlineBracket (fcallstr, ",");
                    trim (args);
                    if (args.size () == 2)
                    {
                        double v1 = stringBlocktoDouble (args[0], ri);
                        double v2 = stringBlocktoDouble (args[1], ri);
                        val = InterpretFunction (cmdBlock.to_string (), v1, v2, ri);
                    }
                    else if (args.size () == 1)  // if the single argument is a function of multiple arguments
                    {
                        if (cmdBlock == "query")
                        {
                            val = ObjectQuery (args[0], ri.getKeyObject ());
                        }
                        else
                        {
                            double v1 = stringBlocktoDouble (args[0], ri);
                            val = InterpretFunction (cmdBlock.to_string (), v1, ri);
                        }
                    }
                    else
                    {
                        printf ("invalid arguments to function %s\n", cmdBlock.to_string ().c_str ());
                    }
                }
                else
                {
                    if (cmdBlock == "query")
                    {
                        val = ObjectQuery (fcallstr, ri.getKeyObject ());
                    }
                    else
                    {
                        val = stringBlocktoDouble (fcallstr, ri);

                        if (!std::isnan (val))
                        {
                            val = InterpretFunction (cmdBlock.to_string (), val, ri);
                        }
                    }
                }
            }
        }
        else
        {
            val = interpretStringBlock (command, ri);
        }
    }

    return val;
}

double interpretStringBlock (string_view command, readerInfo &ri)
{
    auto val = numeric_conversionComplete<double> (command, std::nan ("0"));
    if (std::isnan (val))
    {
        std::string ncommand = ri.checkDefines (command.to_string ());
        // iterate the process until the variable is no longer modified and still fails conversion to numerical
        if (command != ncommand)
        {
            val = numeric_conversionComplete<double> (ncommand, std::nan ("0"));
            if (std::isnan (val))
            {
                val = interpretString (ncommand, ri);
            }
        }
    }
    return val;
}

void interpretStringBlock (string_view command, readerInfo &ri, std::vector<double> &outputs)
{
    auto strSplit = split (command);
    trim (strSplit);
    outputs.resize (strSplit.size ());
    std::transform (strSplit.begin (), strSplit.end (), outputs.begin (),
                    [&ri](auto &str) { return interpretStringBlock (str, ri); });
    // for (size_t kk = 0; kk < strSplit.size (); ++kk)
    //{
    //    outputs[kk] = interpretStringBlock (strSplit[kk], ri);
    //}
}

double addSubStringBlocks (string_view command, readerInfo &ri, size_t rlc)
{
    char op = command[rlc];

    // check if either Ablock or Bclock is a constant
    auto Ablock = trim (command.substr (0, rlc));
    double valA = (Ablock.empty ()) ? 0.0 : stringBlocktoDouble (Ablock, ri);

    auto Bblock = trim (command.substr (rlc + 1, string_view::npos));
    double valB = stringBlocktoDouble (Bblock, ri);

    return (op == '+') ? valA + valB : valA - valB;
}

const double nan_val = std::nan ("0");

double multDivStringBlocks (string_view command, readerInfo &ri, size_t rlc)
{
    char op = command[rlc];

    auto Ablock = trim (command.substr (0, rlc));
    double valA = stringBlocktoDouble (Ablock, ri);

    // load the second half of the multiplication
    auto Bblock = trim (command.substr (rlc + 1, string_view::npos));
    double valB = stringBlocktoDouble (Bblock, ri);

    double val;
    switch (op)
    {
    case '*':
        val = valA * valB;
        break;
    case '%':
        val = (valB == 0.0) ? nan_val : fmod (valA, valB);
        break;
    case '/':
        val = (valB == 0.0) ? nan_val : valA / valB;
        break;
    case '^':
        val = pow (valA, valB);
        break;
    default:
        // this shouldn't happen
        val = nan_val;
    }
    return val;
}

size_t pChunckEnd (string_view command, size_t start)
{
    int open = 1;
    size_t rlc = start - 1;
    while (open > 0)
    {
        rlc = command.find_first_of ("()", rlc + 1);
        if (rlc == string_view::npos)
        {
            break;
        }
        open += (command[rlc] == '(') ? 1 : -1;
    }
    return rlc;
}

double InterpretFunction (const std::string &command, readerInfo &ri)
{
    auto fval = evalFunction (command);

    // if we still didn't find any function check if there is a redefinition
    if (std::isnan (fval))
    {
        auto rep = ri.checkDefines (command);
        if (rep != command)
        {
            fval = evalFunction (rep);  // don't let it iterate more than once
        }
    }
    return fval;
}

double InterpretFunction (const std::string &command, double val, readerInfo &ri)
{
    auto fval = evalFunction (command, val);

    // if we still didn't find any function check if there is a redefinition
    if (std::isnan (fval))
    {
        auto rep = ri.checkDefines (command);
        if (rep != command)
        {
            fval = evalFunction (rep, val);  // don't let it iterate more than once
        }
    }
    return fval;
}

double InterpretFunction (const std::string &command, double val1, double val2, readerInfo &ri)
{
    auto fval = evalFunction (command, val1, val2);

    // if we still didn't find any function check if there is a redefinition
    if (std::isnan (fval))
    {
        auto rep = ri.checkDefines (command);
        if (rep != command)
        {
            fval = evalFunction (rep, val1, val2);  // don't let it iterate more than once
        }
    }
    return fval;
}

double ObjectQuery (string_view command, coreObject *obj)
{
    if (obj == nullptr)
    {
        return nan_val;
    }
    objInfo query (command.to_string (), obj);
    if (!query.m_field.empty ())
    {
        double val = query.m_obj->get (query.m_field, query.m_unitType);
        return val;
    }
    return nan_val;
}

double stringBlocktoDouble (string_view block, readerInfo &ri)
{
    if (nonNumericFirstCharacter (
          block))  // if the first character is not a digit then go to the string interpreter
    {
        return interpretString_sv (block, ri);
    }
    try
    {
        size_t mpos;
        double valA = numConvComp<double> (block, mpos);
        if (mpos < block.length ())
        {
            valA = interpretString_sv (block, ri);
        }
        return valA;
    }
    catch (std::invalid_argument &)
    {
        return interpretString_sv (block, ri);
    }
}

}  // namespace griddyn