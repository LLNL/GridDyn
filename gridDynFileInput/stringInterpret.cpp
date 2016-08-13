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

#include "functionInterpreter.h"
#include "stringOps.h"
#include "gridDynFileInput.h"

#include <cmath>
#include <functional>
#include <utility>
#include <cctype>

double interpretStringBlock (const std::string &command, readerInfo *ri);

void interpretStringBlock (const std::string &command, readerInfo *ri, std::vector<double> &outputs);

double addSubStringBlocks (const std::string &command, readerInfo *ri, size_t rlc);
double multDivStringBlocks (const std::string &command, readerInfo *ri, size_t rlc);
size_t pChunckEnd (const std::string &cmd, size_t start);

double InterpretFunction (const std::string &cmdString, readerInfo *ri);
double InterpretFunction (const std::string &cmdString, double val, readerInfo *ri);
double InterpretFunction (const std::string &cmdString, double val1,double val2, readerInfo *ri);

double stringBlocktoDouble (const std::string &block, readerInfo *ri);

double interpretString (std::string command, readerInfo *ri)
{
  size_t rlc, rlcp = 0;
  //check for functions
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
    {             //just remove outer perenthesis and call again
      val = interpretString (command.substr (1, rlcp - 1), ri);
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
      val = addSubStringBlocks (command,ri, rlc);
    }
  else if ((rlc = command.find_first_of ("*/^%", rlcp + 1)) != std::string::npos)
    {
      val = multDivStringBlocks (command, ri, rlc);
    }
  else
    {
      if (rlcps != std::string::npos)
        {
          std::string cmdBlock = command.substr (0, rlcps);

          std::string fcallstr = trim (command.substr (rlcps + 1, rlcp - rlcps - 1));
          if (fcallstr.empty ())
            {
              val = InterpretFunction (cmdBlock, ri);
            }
          else
            {
              auto cloc = fcallstr.find_first_of (',');
              if (cloc != std::string::npos)
                {
                  auto args = splitlineBracketTrim (fcallstr, ",");
                  if (args.size () == 2)
                    {
                      double v1 = stringBlocktoDouble (args[0], ri);
                      double v2 = stringBlocktoDouble (args[1], ri);
                      val = InterpretFunction (cmdBlock, v1,v2, ri);
                    }
                  else if (args.size () == 1)                //if the single argument is a function of multiple arguments
                    {
                      double v1 = stringBlocktoDouble (args[0], ri);
                      val = InterpretFunction (cmdBlock, v1, ri);
                    }
                  else
                    {
                      printf ("invalid arguments to function %s\n", cmdBlock.c_str ());
                    }
                }
              else
                {
                  val = stringBlocktoDouble (fcallstr, ri);

                  if (!std::isnan (val))
                    {
                      val = InterpretFunction (cmdBlock, val, ri);
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

double interpretStringBlock (const std::string &command, readerInfo *ri)
{
  double val = doubleReadComplete (command, std::nan ("0"));
  if (std::isnan (val))
    {
      std::string ncommand = ri->checkDefines (command);
      //iterate the process until the variable is no longer modified and still fails conversion to numerical
      if (ncommand != command)
        {
          val = doubleReadComplete (command, std::nan ("0"));
          if (std::isnan (val))
            {
              val = interpretString (ncommand, ri);
            }
        }
    }
  return val;
}

void interpretStringBlock (const std::string &command, readerInfo *ri, std::vector<double> &outputs)
{
  auto strSplit = splitlineTrim (command);
  outputs.resize (strSplit.size ());
  for (size_t kk = 0; kk < strSplit.size (); ++kk)
    {
      outputs[kk] = interpretStringBlock (strSplit[kk], ri);
    }
}

double addSubStringBlocks (const std::string &command, readerInfo *ri, size_t rlc)
{


  char op = command[rlc];

  //check if either Ablock or Bclock is a constant
  std::string Ablock = trim (command.substr (0, rlc));
  double valA = (Ablock.empty ()) ? 0.0 : stringBlocktoDouble (Ablock, ri);

  std::string Bblock = trim (command.substr (rlc + 1, std::string::npos));
  double valB = stringBlocktoDouble (Bblock, ri);

  return (op == '+') ? valA + valB : valA - valB;

}

double multDivStringBlocks (const std::string &command, readerInfo *ri, size_t rlc)
{


  char op = command[rlc];


  std::string Ablock = trim (command.substr (0, rlc));
  double valA = stringBlocktoDouble (Ablock, ri);

  // load the second half of the multiplication
  std::string Bblock = trim (command.substr (rlc + 1, std::string::npos));
  double valB = stringBlocktoDouble (Bblock, ri);

  double val = 0.0;
  switch (op)
    {
    case '*':
      val = valA * valB;
      break;
    case '%':
      val = (valB == 0.0) ? std::nan ("0") : fmod (valA, valB);
      break;
    case '/':
      val = (valB == 0.0) ? std::nan ("0") : valA / valB;
      break;
    case '^':
      val = pow (valA, valB);
      break;
    default:
      //this shouldn't happen
      val = std::nan ("0");
    }
  return val;
}


size_t pChunckEnd (const std::string &cmd, size_t start)
{
  int open = 1;
  size_t rlc = start - 1;
  while (open > 0)
    {
      rlc = cmd.find_first_of ("()", rlc + 1);
      if (rlc == std::string::npos)
        {
          break;
        }
      open += (cmd[rlc] == '(') ? 1 : -1;
    }
  return rlc;

}


double InterpretFunction (const std::string &cmdString, readerInfo *ri)
{
  double fval = evalFunction (cmdString);

  //if we still didn't find any function check if there is a redefinition
  if ((std::isnan (fval)) && (ri))
    {
      auto rep = ri->checkDefines (cmdString);
      if (rep != cmdString)
        {
          fval = evalFunction (rep);                      //don't let it iterate more than once
        }
    }
  return fval;
}


double InterpretFunction (const std::string &cmdString, double val, readerInfo *ri)
{
  double fval = evalFunction (cmdString, val);

  //if we still didn't find any function check if there is a redefinition
  if ((std::isnan (fval)) && (ri))
    {
      auto rep = ri->checkDefines (cmdString);
      if (rep != cmdString)
        {
          fval = evalFunction (rep, val);                      //don't let it iterate more than once
        }
    }
  return fval;
}

double InterpretFunction (const std::string &cmdString, double val1, double val2, readerInfo *ri)
{
  double fval = evalFunction (cmdString, val1, val2);

  //if we still didn't find any function check if there is a redefinition
  if ((std::isnan (fval)) && (ri))
    {
      auto rep = ri->checkDefines (cmdString);
      if (rep != cmdString)
        {
          fval = evalFunction (rep, val1,val2);                      //don't let it iterate more than once
        }
    }
  return fval;
}

double stringBlocktoDouble (const std::string &block, readerInfo *ri)
{
  if (!isdigit (block[0]))       //if the first character is not a digit then go to the string interpreter
    {
      return interpretString (block, ri);
    }
  try
    {
      size_t mpos;
      double valA = std::stod (block, &mpos);
      if (mpos < block.length ())
        {
          valA = interpretString (block, ri);
        }
      return valA;
    }
  catch (std::invalid_argument)
    {
      return interpretString (block, ri);
    }
}
