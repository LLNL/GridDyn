/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil;  eval: (c-set-offset 'innamespace 0); -*- */
/*
 * LLNS Copyright Start
 * Copyright (c) 2016, Lawrence Livermore National Security
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

#include "stringOps.h"

#include "objectInterpreter.h"
#include "functionInterpreter.h"
#include <memory>
#include <string>
#include <functional>
#include <type_traits>
#include <cmath>



template <class baseX, class opX, class funcX>
class grabberInterpreter
{
  static_assert (std::is_base_of<baseX, opX>::value, "Operations class and base class must have a parent child relationship");
  static_assert (std::is_base_of<baseX, funcX>::value, "functions class and base class must have a parent child relationship");
private:
  std::function < std::shared_ptr<baseX> (const std::string &, gridCoreObject *)> createX;
public:
  grabberInterpreter (std::function < std::shared_ptr<baseX> (const std::string &, gridCoreObject *)> fc) : createX (fc)
  {
  }
  std::shared_ptr<baseX> interpretGrabberBlock (const std::string &command, gridCoreObject *obj)
  {
    size_t rlc, rlcp;
    std::shared_ptr<baseX> ggb = nullptr;
    std::shared_ptr<baseX> ggbA = nullptr;

    //check for functions
    size_t rlcps = command.find_first_of ('(', 0);
    if (rlcps != std::string::npos)
      {
        rlcp = pChunckEnd (command, rlcps + 1);
        if (rlcp == std::string::npos)
          {
            rlcp = command.length ();
          }
      }
    else             //no parenthesis
      {
        rlcp = 0;
      }
    if ((rlcps == 0) && (rlcp == command.length () - 1))
      {
        ggb = interpretGrabberBlock (command.substr (1, rlcp - 1), obj);
      }
    else if (((rlc = command.find_first_of ("+-", 1)) != std::string::npos) && (rlc < rlcps))
      {
        ggb = addSubGrabberBlocks (command, obj, rlc);
      }
    else if (((rlc = command.find_first_of ("*/^", 1)) != std::string::npos) && (rlc < rlcps))
      {
        ggb = multDivGrabberBlocks (command, obj, rlc);
      }
    else if ((rlc = command.find_first_of ("+-", rlcp + 1)) != std::string::npos)
      {
        ggb = addSubGrabberBlocks (command, obj, rlc);
      }
    else if ((rlc = command.find_first_of ("*/^", rlcp + 1)) != std::string::npos)
      {
        ggb = multDivGrabberBlocks (command, obj, rlc);
      }
    else
      {
        if (rlcps != std::string::npos)
          {
            std::string cmdBlock = command.substr (0, rlcps);
            if (isFunctionName (cmdBlock))
              {
                std::string fcallstr = command.substr (rlcps + 1, rlcp - rlcps - 1);
                trimString (fcallstr);
                ggbA = interpretGrabberBlock (fcallstr, obj);
                if (ggbA)
                  {
                    ggb = std::make_shared<funcX> (ggbA, cmdBlock);
                  }
              }
            else                     // not a function call so must be units description
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
  std::shared_ptr<baseX> singleBlockInterpreter (const std::string &command, gridCoreObject *obj)
  {
    std::shared_ptr<baseX> ggb;
    gridCoreObject *mobj;
    std::string field;
    gridUnits::units_t outUnit = gridUnits::defUnit;
    //get the object which to grab from
    size_t rlc = command.find_last_of (':');
    if (rlc != std::string::npos)
      {
        mobj = locateObject (command, obj);
        if (!(mobj))
          {
            return nullptr;
          }
        field = command.substr (rlc + 1, std::string::npos);
      }
    else
      {
        mobj = obj;
        field = command;
      }

    rlc = field.find_first_of ('(');
    if (rlc != std::string::npos)
      {
        size_t rlc2 = field.find_first_of (')',rlc);
        std::string unitName = field.substr (rlc + 1, rlc2 - rlc - 1);
        outUnit = gridUnits::getUnits (unitName);
        field = field.substr (0, rlc);
      }
    ggb = createX (field, mobj);
    ggb->outputUnits = outUnit;
    return ggb;
  }


  std::shared_ptr<baseX> addSubGrabberBlocks (const std::string &command, gridCoreObject *obj, size_t rlc)
  {
    std::shared_ptr<baseX> ggb = nullptr;
    std::string Ablock = command.substr (0, rlc);
    trimString (Ablock);
    char op = command[rlc];
    std::string Bblock = command.substr (rlc + 1, std::string::npos);
    trimString (Bblock);
    //check if either Ablock or Bclock is a constant
    double valA = doubleReadComplete (Ablock,kNullVal);
    std::shared_ptr<baseX> ggbA = (valA == kNullVal) ? interpretGrabberBlock (Ablock, obj) : nullptr;

    double valB = doubleReadComplete (Bblock, kNullVal);
    std::shared_ptr<baseX> ggbB = (valB == kNullVal) ? interpretGrabberBlock (Bblock, obj) : nullptr;

    if (ggbA)             //we know Ablock isstd::make_shared<grabber
      {
        if (ggbB)                  //both are grabber blocks
          {
            ggb = std::make_shared<opX> (ggbA, ggbB, command.substr (rlc, 1));
          }
        else if (valB != kNullVal)
          {
            if (op == '+')
              {
                ggb = ggbA;
                ggb->bias = valB;
              }
            else
              {
                ggb = ggbA;
                ggb->bias = -valB;
              }
          }
        else
          {
            return nullptr;                      //we can't interpret this
          }
      }
    else if (valA != kNullVal)
      {
        if (ggbB)                  //both are grabber blocks
          {
            ggb = ggbB;
            ggb->bias = valA;
            if (op == '-')
              {
                ggb->gain = -1.0;
              }
          }
        else if (valB != kNullVal)                //both are numeric
          {
            ggb = std::make_shared<baseX> ();
            ggb->setInfo ("constant", obj);
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
            return nullptr;                      //we can't interpret this
          }
      }
    else
      {
        return nullptr;
      }
    return ggb;
  }

  std::shared_ptr<baseX> multDivGrabberBlocks (const std::string &command, gridCoreObject *obj, size_t rlc)
  {

    std::shared_ptr<baseX> ggb = nullptr;

    std::string Ablock = command.substr (0, rlc);
    trimString (Ablock);
    char op = command[rlc];
    std::string Bblock = command.substr (rlc + 1, std::string::npos);
    trimString (Bblock);
    //check if either Ablock or Bclock is a constant
    double valA = doubleReadComplete (Ablock, kNullVal);
    std::shared_ptr<baseX> ggbA = (valA == kNullVal) ? interpretGrabberBlock (Ablock, obj) : nullptr;

    double valB = doubleReadComplete (Bblock, kNullVal);
    std::shared_ptr<baseX> ggbB = (valB == kNullVal) ? interpretGrabberBlock (Bblock, obj) : nullptr;

    if (ggbA)             //we know Ablock isstd::make_shared<grabber
      {
        if (ggbB)                  //both are grabber blocks
          {
            ggb = std::make_shared<opX> (ggbA, ggbB, command.substr (rlc, 1));
          }
        else if (valB != kNullVal)
          {
            if (op == '*')
              {
                ggb = ggbA;
                ggb->gain = valB;
              }
            else if (op == '/')
              {
                ggb = ggbA;
                ggb->gain = 1 / valB;
              }
            else
              {
                //set up power function
                ggbB = std::make_shared<baseX> ();
                ggbB->setInfo ("constant", obj);
                ggbB->bias = valB;
                ggb = std::make_shared<opX> (ggbA, ggbB, command.substr (rlc, 1));
              }
          }
        else
          {
            return nullptr;                      //we can't interpret this
          }
      }
    else if (valA != kNullVal)
      {
        if (ggbB)                  //B is a block
          {
            ggb = ggbB;
            if (op == '*')
              {
                ggb->gain = valA;
              }
            else
              {
                //set up division by the grabber not a constant
                ggbA = std::make_shared<baseX> ();
                ggbA->setInfo ("constant", obj);
                ggbA->bias = valA;
                ggb = std::make_shared<opX> (ggbA, ggbB, command.substr (rlc, 1));
              }
          }
        else if (valB != kNullVal)                //both are numeric
          {
            ggb = std::make_shared<baseX> ();
            ggb->setInfo ("constant", obj);
            if (op == '*')
              {
                ggb->bias = valA * valB;
              }
            else if (op == '/')
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
            return nullptr;                      //we can't interpret this
          }
      }
    else
      {
        return nullptr;
      }
    return ggb;
  }


  size_t pChunckEnd (const std::string &cmd, size_t start)
  {
    int open = 1;
    size_t rlc = start;
    while (open)
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



#endif
