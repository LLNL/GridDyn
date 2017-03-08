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
#include "utilities/stringOps.h"
#include "formatInterpreters/readerElement.h"

using namespace readerConfig;
std::string findElementName (std::shared_ptr<readerElement> &el, const std::string &ename, readerConfig::match_type matching)
{

  std::string fname = ename;
  if (el->hasElement (fname))
    {
      return fname;
    }
  switch (matching)
    {
    
    case match_type::capital_case_match:

      //check lower case
      fname = convertToLowerCase (fname);
      if (el->hasElement (fname))
        {
          return fname;
        }
      makeUpperCase (fname);
      if (el->hasElement (fname))
        {
          return fname;
        }
      break;
    case match_type::any_case_match:
      fname = convertToLowerCase (fname);

      el->moveToFirstChild ();
      while (el->isValid ())
        {
          std::string tempName = convertToLowerCase (el->getName ());
          if (fname == tempName)
            {
              fname = el->getName ();
              el->moveToParent ();
              return fname;
            }
          el->moveToNextSibling ();
        }
      el->moveToParent ();
      break;
	case match_type::strict_case_match:
	default:
		break;
    }
  return emptyString;
}

std::string getElementAttribute (std::shared_ptr<readerElement> &el, const std::string &ename, match_type matching)
{
  if (el->hasAttribute (ename))
    {
      return el->getAttributeText (ename);
    }
  switch (matching)
    {
    case match_type::capital_case_match:
	{
		auto tempName = convertToLowerCase(ename);
		if (el->hasAttribute(tempName))
		{
			return el->getAttributeText(tempName);
		}
		makeUpperCase(tempName);
		if (el->hasAttribute(tempName))
		{
			return el->getAttributeText(tempName);
		}
	}
      break;
    case match_type::any_case_match:
	{
		auto tempName = convertToLowerCase(ename);
		auto att = el->getFirstAttribute();
		while (att.isValid())
		{
			auto fname = convertToLowerCase(att.getName());
			if (tempName == fname)
			{
				return att.getText();
			}
			att = el->getNextAttribute();
		}
	}
      break;
	case match_type::strict_case_match:
	default:
		break;
    }
  return emptyString;
}

std::string getElementField (std::shared_ptr<readerElement> &el, const std::string &ename, match_type matching)
{

  auto ret = getElementAttribute (el, ename, matching);
  //an element can override an attribute
  auto name = findElementName (el, ename, matching);
  if (name.empty ())
    {
      return ret;
    }
  el->moveToFirstChild (name);
  ret = el->getText ();
  el->moveToParent ();
  return ret;
}


std::string getElementFieldOptions (std::shared_ptr<readerElement> &el, const stringVec &names, match_type matching)
{
  for (const auto &str : names)
    {
      auto ret = getElementField (el, str, matching);
      if (!ret.empty ())
        {
          return ret;
        }
    }
  return emptyString;
}


stringVec getElementFieldMultiple (std::shared_ptr<readerElement> &element, std::string ename, match_type matching)
{
  stringVec val;
  std::string elname_act;
  //get an attribute if there is one
  auto ret = getElementAttribute (element, ename, matching);

  if (!ret.empty ())
    {
      val.push_back (ret);
    }
  auto name = findElementName (element, ename, matching);
  if (name.empty ())
    {
      return val;
    }
  element->moveToFirstChild (name);
  while (element->isValid ())
    {
      val.push_back (element->getText ());

      element->moveToNextSibling (name);                                    // next object
    }
  element->moveToParent ();
  return val;
}

