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

#include "readElement.h"
#include "readerElement.h"
#include "readerHelper.h"
#include "elementReaderTemplates.hpp"

#include <cmath>

using namespace readerConfig;

static const IgnoreListType ignoreConditionVariables {
  "condition"
};
bool checkCondition (const std::string &cond, readerInfo *ri, coreObject *parentObject);
// "aP" is the XML element passed from the reader
void loadConditionElement (std::shared_ptr<readerElement> &element, readerInfo *ri, coreObject *parentObject)
{

  auto riScope = ri->newScope ();

  loadDefines (element, ri);
  loadDirectories (element, ri);

  bool eval = false;
  std::string condString = getElementField (element, "condition", defMatchType);

  if (!condString.empty ())
    {
      //deal with &gt for > and &lt for < if necessary
      condString = xmlCharacterCodeReplace (condString);
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
      //load the subobjects of gen
      loadSubObjects (element, ri, parentObject);
      //get all element fields
      paramLoopElement (parentObject, element, "local", ri, ignoreConditionVariables);
      readImports (element, ri, parentObject, true);

    }

  ri->closeScope (riScope);

}


bool checkCondition (const std::string &cond, readerInfo *ri, coreObject * /*parentObject*/)
{
  size_t pos = cond.find_first_of ("><=!");
  bool eval = false;
  char A, B;
  std::string BlockA, BlockB;

  if (pos == std::string::npos)
    {
      A = '!';
      B = '=';
      BlockA = cond;
      BlockB = "0";
    }
  else
    {
      A = cond[pos];
      B = cond[pos + 1];
      BlockA = trim (cond.substr (0, pos - 1));
      BlockB = (B == '=') ? trim (cond.substr (pos + 2)) : trim (cond.substr (pos + 1));
    }

  double aval = interpretString (BlockA, ri);
  double bval = interpretString (BlockB, ri);

  if (std::isnan (aval) && (std::isnan (bval)))
    {    //do a string comparison
      std::string astr = ri->checkDefines (BlockA);
      std::string bstr = ri->checkDefines (BlockB);
      if (A == '>')
        {
          if (B == '=')
            {
              eval = (astr >= bstr);
            }
          else
            {
              eval = (astr > bstr);
            }

        }
      else if (A == '<')
        {
          if (B == '=')
            {
              eval = (astr <= bstr);
            }
          else
            {
              eval = (astr < bstr);
            }
        }
      else if (A == '=')
        {
          eval = (astr == bstr);
        }
      else
        {
          eval = (astr != bstr);
        }
    }
  else if (std::isnan (aval) || (std::isnan (bval)))
    {     //mixed string and number comparison
      WARNPRINT (READER_WARN_IMPORTANT, "invalid comparison terms");
    }
  else
    {
      if (A == '>')
        {
          if (B == '=')
            {
              eval = (aval >= bval);
            }
          else
            {
              eval = (aval > bval);
            }

        }
      else if (A == '<')
        {
          if (B == '=')
            {
              eval = (aval <= bval);
            }
          else
            {
              eval = (aval < bval);
            }
        }
      else if (A == '=')
        {
          eval = (aval == bval);
        }
      else
        {
          eval = (aval != bval);
        }
    }


  return eval;
}
