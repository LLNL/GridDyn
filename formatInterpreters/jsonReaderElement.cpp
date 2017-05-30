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

#include "jsonReaderElement.h"
#include "jsonElement.h"
#include "utilities/stringConversion.h"
#include <cassert>
#include <fstream>
#include <iostream>

static const std::string nullStr = std::string ("");

bool isElement(const Json::Value &testValue);
bool isAttribute(const Json::Value &testValue);

jsonReaderElement::jsonReaderElement ()
{

}

jsonReaderElement::jsonReaderElement (const std::string &filename)
{
  loadFile (filename);
}

void jsonReaderElement::clear ()
{
	parents.clear();
	if (current)
	{
		current->clear();
	}
}

bool jsonReaderElement::isValid () const
{
  return ((current)&&(!current->isNull()));
}

bool jsonReaderElement::isDocument () const
{
  if (parents.empty ())
    {
      if (doc)
        {
          return true;
        }
    }
  return false;
}


std::shared_ptr<readerElement> jsonReaderElement::clone () const
{
  auto ret = std::make_shared<jsonReaderElement> ();
  ret->parents.reserve(parents.size());
  for (auto &parent : parents)
  {
	  ret->parents.push_back(std::make_shared<jsonElement>(*parent));
  }
  ret->current = std::make_shared<jsonElement>(*current);
  ret->doc = doc;
  return ret;
}

bool jsonReaderElement::loadFile (const std::string &filename)
{
  std::ifstream file (filename);
  if (file.is_open ())
    {
      doc = std::make_shared<Json::Value> ();


      Json::CharReaderBuilder rbuilder;
      std::string errs;
      bool ok = Json::parseFromStream (rbuilder, file, doc.get (), &errs);
      if (ok)
        {
          current = std::make_shared<jsonElement> (*doc, filename);
          return true;
        }
      else
        {
          std::cerr << "file read error in " << filename << "::" << errs << '\n';
          doc = nullptr;
          clear ();
          return false;
        }
    }
  else
    {
      std::cerr << "unable to open file " << filename << '\n';
      doc = nullptr;
      clear ();
      return false;
    }

}


bool jsonReaderElement::parse (const std::string &inputString)
{

  doc = std::make_shared<Json::Value> ();

  Json::Reader stringReader;
  bool ok = stringReader.parse (inputString, *doc.get (), false);
  if (ok)
    {
      current = std::make_shared<jsonElement> (*doc, "string");
      return true;
    }
  else
    {
      std::cerr << "file read error in stream::" << stringReader.getFormattedErrorMessages () << '\n';
      doc = nullptr;
      clear ();
      return false;
    }


}

std::string jsonReaderElement::getName () const
{
  return current->name;
}

double jsonReaderElement::getValue () const
{
  if (!isValid())
    {
      return readerNullVal;
    }

  if (current->getElement ().isConvertibleTo (Json::ValueType::realValue))
    {
      return current->getElement ().asDouble ();
    }
  else if (current->getElement ().isConvertibleTo (Json::ValueType::stringValue))
    {
      return numeric_conversionComplete (current->getElement ().asString (), readerNullVal);
    }
  else
    {
      return readerNullVal;
    }
}

std::string jsonReaderElement::getText () const
{
  if (!isValid())
    {
      return nullStr;
    }

  if (current->getElement ().isConvertibleTo (Json::ValueType::stringValue))
    {
      return current->getElement ().asString ();
    }
  else
    {
      return nullStr;
    }


}

std::string jsonReaderElement::getMultiText (const std::string &/*sep*/) const
{
  if (!isValid())
    {
      return nullStr;
    }

  if (current->getElement ().isConvertibleTo (Json::ValueType::stringValue))
    {
      return current->getElement ().asString ();
    }
  else
    {
      return nullStr;
    }
}
//no attributes in json
bool jsonReaderElement::hasAttribute (const std::string &attributeName) const
{
  if (!isValid())
    {
      return false;
    }

  if (current->getElement ().isMember (attributeName))
    {
      return (isAttribute (current->getElement ()[attributeName]));
    }
  else
    {
      return false;
    }
}



bool jsonReaderElement::hasElement (const std::string &elementName) const
{
  if (!isValid())
    {
      return false;
    }

  if (current->getElement ().isMember (elementName))
    {
      return (isElement (current->getElement ()[elementName]));
    }

  return false;
}

readerAttribute jsonReaderElement::getFirstAttribute ()
{
  if (!isValid())
    {
      return readerAttribute ();
    }

  auto attIterator = current->getElement ().begin ();
  auto elementEnd = current->getElement ().end ();
  iteratorCount = 0;

  while (attIterator != elementEnd)
    {
      if (isAttribute (*attIterator))
        {
          return readerAttribute (attIterator.name (), attIterator->asString ());
        }
      ++attIterator;
	  ++iteratorCount;
    }


  return readerAttribute ();
}

readerAttribute jsonReaderElement::getNextAttribute ()
{
  if (!isValid())
    {
      return readerAttribute ();
    }
  auto elementEnd = current->getElement ().end ();
  auto attIterator = current->getElement().begin();
  for (int ii = 0; ii < iteratorCount; ++ii)
  {
	  ++attIterator;
	  if (attIterator == elementEnd)
	  {
		  return readerAttribute();
	  }
  }
  if (attIterator == elementEnd)
    {
      return readerAttribute ();
    }
  ++attIterator;
  ++iteratorCount;
  while (attIterator != elementEnd)
    {
      if (isAttribute (*attIterator))
        {
          return readerAttribute (attIterator.name (), attIterator->asString ());
        }
      ++attIterator;
	  ++iteratorCount;
    }
  return readerAttribute ();
}

readerAttribute jsonReaderElement::getAttribute (const std::string & attributeName) const
{
  if (hasAttribute (attributeName))
    {
      return readerAttribute (attributeName, current->getElement ()[attributeName].asString ());
    }
  return readerAttribute ();
}

std::string jsonReaderElement::getAttributeText (const std::string &attributeName) const
{
  if (hasAttribute (attributeName))
    {
      return current->getElement ()[attributeName].asString ();
    }
  return nullStr;
}

double jsonReaderElement::getAttributeValue (const std::string &attributeName) const
{
  if (hasAttribute (attributeName))
    {

      if (current->getElement ()[attributeName].isConvertibleTo (Json::ValueType::realValue))
        {
          return current->getElement ()[attributeName].asDouble ();
        }
      else
        {
          return numeric_conversionComplete (current->getElement ()[attributeName].asString (), readerNullVal);
        }


    }
  return readerNullVal;
}

std::shared_ptr<readerElement> jsonReaderElement::firstChild () const
{
  auto newElement = clone ();
  newElement->moveToFirstChild ();
  return newElement;

}

std::shared_ptr<readerElement> jsonReaderElement::firstChild (const std::string &childName) const
{
  auto newElement = clone ();
  newElement->moveToFirstChild (childName);
  return newElement;
}




void jsonReaderElement::moveToFirstChild ()
{
  if (!isValid())
    {
      return;
    }
  current->elementIndex = 0;
  auto elementIterator = current->getElement ().begin ();
  auto endIterator = current->getElement ().end ();

  while (elementIterator != endIterator)
    {

      if (isElement (*elementIterator))
        {
          parents.push_back (current);
          current = std::make_shared<jsonElement>(*elementIterator, elementIterator.name ());
          return;
        }
      ++elementIterator;
      ++current->elementIndex;
    }
  parents.push_back (current);
  current->clear();

}

void jsonReaderElement::moveToFirstChild (const std::string &childName)
{
  if (!isValid())
    {
      return;
    }

  if (current->getElement ().isMember (childName))
    {
      if (isElement (current->getElement ()[childName]))
        {
          parents.push_back (current);
          current = std::make_shared<jsonElement> (current->getElement ()[childName], childName);
          return;
        }
    }

  parents.push_back (current);
  current->clear();

}

void jsonReaderElement::moveToNextSibling ()
{
  if (!isValid())
    {
      return;
    }
  ++current->arrayIndex;
  while (current->arrayIndex < current->count ())
    {
      if (!current->getElement ().empty ())
        {
          return;
        }
      ++current->arrayIndex;
    }
  if (parents.empty ())
    {
      current->clear();
      return;
    }
  //there are no more elements in a potential array
  auto elementIterator = parents.back ()->getElement ().begin ();
  auto endIterator = parents.back ()->getElement ().end ();
  ++parents.back ()->elementIndex;
  //iterators don't survive copy so have to move the iterator to the next element index
  for (int ii = 0; ii < parents.back ()->elementIndex; ++ii)
    {
      ++elementIterator;
      if (elementIterator == endIterator)
        {
          current->clear();
        }
    }
  //Now find the next valid element
  while (elementIterator != endIterator)
    {

      if (isElement (*elementIterator))
        {
          current = std::make_shared<jsonElement> (*elementIterator, elementIterator.name ());
          return;
        }
      ++elementIterator;
      ++parents.back ()->elementIndex;
    }
  current->clear();

}

void jsonReaderElement::moveToNextSibling (const std::string &siblingName)
{
  if (!isValid())
    {
      return;
    }
  if (siblingName == current->name)
    {
      ++current->arrayIndex;
      while (current->arrayIndex < current->count ())
        {
          if (!current->getElement ().empty ())
            {
              return;
            }
          ++current->arrayIndex;
        }
      current->clear();
    }
  else
    {
      if (parents.back ()->getElement ().isMember (siblingName))
        {
          if (isElement (parents.back ()->getElement ()[siblingName]))
            {
              current = std::make_shared<jsonElement>(parents.back ()->getElement ()[siblingName], siblingName);
              return;
            }
        }
    }


}

void jsonReaderElement::moveToParent ()
{
  if (parents.empty ())
    {
      return;
    }
  current = parents.back ();
  parents.pop_back ();

}

std::shared_ptr<readerElement> jsonReaderElement::nextSibling () const
{
  auto newElement = clone ();
  newElement->moveToNextSibling ();
  return newElement;
}

std::shared_ptr<readerElement> jsonReaderElement::nextSibling (const std::string &siblingName) const
{
  auto newElement = clone ();
  newElement->moveToNextSibling (siblingName);
  return newElement;
}


void jsonReaderElement::bookmark ()
{
  bookmarks.push_back (std::static_pointer_cast<jsonReaderElement> (clone ()));
}

void jsonReaderElement::restore ()
{
  if (bookmarks.empty ())
    {
      return;
    }
  parents = bookmarks.back ()->parents;
  current = bookmarks.back ()->current;
  bookmarks.pop_back ();
}


bool isAttribute(const Json::Value &testValue)
{
	if (testValue.empty())
	{
		return false;
	}
	if (testValue.isObject())
	{
		return false;
	}
	if (testValue.isArray())
	{
		return false;
	}
	return true;
}


bool isElement(const Json::Value &testValue)
{
	if (testValue.empty())
	{
		return false;
	}

	if (testValue.isObject())
	{
		return true;
	}
	if (testValue.isArray())
	{
		return true;
	}

	return false;
}