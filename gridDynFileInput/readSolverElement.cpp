/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil;  eval: (c-set-offset 'innamespace 0); -*- */
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

#include "readElement.h"
#include "elementReaderTemplates.hpp"
#include "gridDynFileInput.h"
#include "solvers/solverInterface.h"
#include "gridDyn.h"
#include "utilities/stringConversion.h"


using namespace readerConfig;
// aP is the link element

static const IgnoreListType solverIgnoreFields {
  "flags", "name", "type", "index"
};

void loadSolverElement (std::shared_ptr<readerElement> &element, readerInfo &ri, gridDynSimulation *gdo)
{

  std::shared_ptr<solverInterface> sd;
  std::string type = getElementField (element, "type", defMatchType);
  if (type.empty ())
    {
    }
  std::string name = getElementField (element, "index", defMatchType);
  //check for the field attributes
  if (!name.empty ())
    {
      int index= numeric_conversion(name, -1);

      if (index >= 0)
        {
          sd = gdo->getSolverInterface (index);
        }
      if (!(sd))
        {
          if (!type.empty ())
            {
              sd = makeSolver (type);
              if (sd)
                {
                  sd->set ("index", index);
                }
            }
        }
    }
  name = getElementField (element, "name", defMatchType);
  if (!name.empty ())
    {
      if (sd)
        {
          if (sd->getSolverMode ().offsetIndex > 1)                               //don't allow overriding the names on solvermode index 0 and 1
            {
              sd->setName(name);
            }
        }
      else
        {
          sd = gdo->getSolverInterface (name);
          if (!(sd))
            {
              if (!type.empty ())
                {
                  sd = makeSolver (type);
                  if (sd)
                    {
                      sd->setName( name);
                    }
                }
            }

        }
    }
  if (!(sd))
    {
      if (type.empty ())
        {
          if (!name.empty ())
            {
              sd = makeSolver (name);
            }
        }
      else
        {
          sd = makeSolver (type);

        }
      if (!(sd))
        {
          return;
        }
      if (!name.empty ())
        {
          sd->setName( name);
        }
    }
  std::string field = getElementField (element, "index", defMatchType);
  if (!field.empty ())
    {
      sd->set ("flags", field);
    }

  setAttributes (sd.get(), element, "solver", ri, solverIgnoreFields);
  setParams (sd.get(), element, "solver", ri, solverIgnoreFields);
  //add the solver
  gdo->add (sd);


}
