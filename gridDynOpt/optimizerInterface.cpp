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

#include "optimizerInterface.h"

#include "gridDynOpt.h"

#include <cstdio>
#include <algorithm>
#include <string>



optimizerInterface::optimizerInterface (gridDynOptimization *gdo, const optimMode &oMode) : mode (oMode),m_gdo (gdo)
{

}


void optimizerInterface::setOptimizationData (gridDynOptimization *gdo, const optimMode &oMode)
{

  mode = oMode;
  if (gdo)
    {
      m_gdo = gdo;
    }
}

void optimizerInterface::initializeJacArray (count_t size)
{
  a1.reserve (size);
  a1.clear ();
}

double optimizerInterface::get (const std::string & /*param*/) const
{
  return kNullVal;
}


int optimizerInterface::check_flag (void *flagvalue, const std::string &funcname, int opt, bool printError)
{
  int *errflag;
  // Check if SUNDIALS function returned nullptr pointer - no memory allocated
  if (opt == 0 && flagvalue == nullptr)
    {
      if (printError)
        {
          m_gdo->log (m_gdo,GD_ERROR_PRINT, funcname + " failed - returned nullptr pointer");
        }
      return(1);
    }
  else if (opt == 1)
    {
      // Check if flag < 0
      errflag = (int *)flagvalue;
      if (*errflag < 0)
        {
          if (printError)
            {
              m_gdo->log (m_gdo, GD_ERROR_PRINT, funcname + " failed with flag = " + std::to_string (*errflag));
            }
          return(1);
        }
    }
  else if (opt == 2 && flagvalue == nullptr)
    {
      // Check if function returned nullptr pointer - no memory allocated
      if (printError)
        {
          m_gdo->log (m_gdo, GD_ERROR_PRINT, funcname + " failed MEMORY_ERROR- returned nullptr pointer");
        }
      return(1);
    }

  return 0;
}


int basicOptimizer::allocate (count_t size)
{
  // load the vectors
  if (size == svsize)
    {
      return FUNCTION_EXECUTION_SUCCESS;
    }

  svsize = size;
  initialized = false;
  allocated = true;
  return FUNCTION_EXECUTION_SUCCESS;
}

void basicOptimizer::objectInitializeA (double /*t0*/)
{
  if (!allocated)
    {
      //  return (-2);
    }
  initialized = true;
  //return FUNCTION_EXECUTION_SUCCESS;
}

std::shared_ptr<optimizerInterface> makeOptimizer (gridDynOptimization *gdo, const optimMode &oMode)
{
  std::shared_ptr<optimizerInterface> od;
  switch (oMode.flowMode)
    {
    case flowModel_t::none:
      od = std::make_shared<basicOptimizer> (gdo, oMode);
      break;
    case flowModel_t::transport:
    case flowModel_t::dc:
    case flowModel_t::ac:
      break;
    }
  return od;
}

std::shared_ptr<optimizerInterface> makeOptimizer (const std::string &type)
{
  std::shared_ptr<optimizerInterface> od;
  if (type == "basic")
    {
      od = std::make_shared<basicOptimizer> ();
    }
  else if (type == "kinsol")
    {
      od = nullptr;
    }
  else if (type == "ida")
    {
      od = nullptr;
    }
  else if (type == "cvode")
    {
      od = nullptr;
    }
  else if (type == "arkode")
    {
      od = nullptr;
    }
  else
    {
      od = nullptr;
    }
  return od;
}
