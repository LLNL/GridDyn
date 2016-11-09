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

#include "jsonReaderElement.h"
#include "json/json.h"
#include "stringOps.h"
#include <cassert>
#include <fstream>
#include <iostream>

static const std::string nullStr = std::string ("");

jsonReaderElement::jsonElement::jsonElement (Json::Value vElement, std::string newName) : name (newName), element (vElement)
{
  elementIndex = 0;

  if (element.isArray ())
    {
      arraytype = true;
      arrayIndex = 0;
      while ((arrayIndex < element.size ()) && (element[arrayIndex].empty ()))
        {
          ++arrayIndex;
        }
    }
}

void jsonReaderElement::jsonElement::clear ()
{
  element = Json::nullValue;
  elementIndex = 0;
  arrayIndex = 0;
  arraytype = false;
  name = nullStr;
}

jsonReaderElement::jsonReaderElement ()
{

}

jsonReaderElement::jsonReaderElement (const std::string &filename)
{
  loadFile (filename);
}



jsonReaderElement::~jsonReaderElement ()
{
}

void jsonReaderElement::clear ()
{
  current.clear ();
  parents.clear ();

}

bool jsonReaderElement::isValid () const
{
  return (!(current.isNull ()));
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
  ret->parents = parents;
  ret->current = current;
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
          current = jsonElement (*doc, filename);
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
      current = jsonElement (*doc, "string");
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
  return current.name;
}

double jsonReaderElement::getValue () const
{
  if (current.isNull ())
    {
      return readerNullVal;
    }

  if (current.getElement ().isConvertibleTo (Json::ValueType::realValue))
    {
      return current.getElement ().asDouble ();
    }
  else if (current.getElement ().isConvertibleTo (Json::ValueType::stringValue))
    {
      double val = doubleReadComplete (current.getElement ().asString (), readerNullVal);
      return val;
    }
  else
    {
      return readerNullVal;
    }
}

std::string jsonReaderElement::getText () const
{
  if (current.isNull ())
    {
      return nullStr;
    }

  if (current.getElement ().isConvertibleTo (Json::ValueType::stringValue))
    {
      return current.getElement ().asString ();
    }
  else
    {
      return nullStr;
    }


}

std::string jsonReaderElement::getMultiText (const std::string /*sep*/) const
{
  if (current.isNull ())
    {
      return nullStr;
    }

  if (current.getElement ().isConvertibleTo (Json::ValueType::stringValue))
    {
      return current.getElement ().asString ();
    }
  else
    {
      return nullStr;
    }
}
//no attributes in json
bool jsonReaderElement::hasAttribute (const std::string &attributeName) const
{
  if (current.isNull ())
    {
      return false;
    }

  if (current.getElement ().isMember (attributeName))
    {
      return (isAttribute (current.getElement ()[attributeName]));
    }
  else
    {
      return false;
    }
}

bool jsonReaderElement::isAttribute (const Json::Value &testValue) const
{
  if (testValue.empty ())
    {
      return false;
    }
  if (testValue.isObject ())
    {
      return false;
    }
  if (testValue.isArray ())
    {
      return false;
    }
  return true;
}


bool jsonReaderElement::isElement (const Json::Value &testValue) const
{
  if (testValue.empty ())
    {
      return false;
    }

  if (testValue.isObject ())
    {
      return true;
    }
  if (testValue.isArray ())
    {
      return true;
    }

  return false;
}

bool jsonReaderElement::hasElement (const std::string &elementName) const
{
  if (current.isNull ())
    {
      return false;
    }

  if (current.getElement ().isMember (elementName))
    {
      return (isElement (current.getElement ()[elementName]));
    }

  return false;
}

readerAttribute jsonReaderElement::getFirstAttribute ()
{
  if (current.isNull ())
    {
      return readerAttribute ();
    }

  attIterator = current.getElement ().begin ();
  auto elementEnd = current.getElement ().end ();


  while (attIterator != elementEnd)
    {
      if (isAttribute (*attIterator))
        {
          return readerAttribute (attIterator.name (), attIterator->asString ());
        }
      ++attIterator;
    }


  return readerAttribute ();
}

readerAttribute jsonReaderElement::getNextAttribute ()
{
  if (current.isNull ())
    {
      return readerAttribute ();
    }
  auto elementEnd = current.getElement ().end ();

  if (attIterator == elementEnd)
    {
      return readerAttribute ();
    }
  ++attIterator;
  while (attIterator != elementEnd)
    {
      if (isAttribute (*attIterator))
        {
          return readerAttribute (attIterator.name (), attIterator->asString ());
        }
      ++attIterator;
    }
  return readerAttribute ();
}

readerAttribute jsonReaderElement::getAttribute (const std::string & attributeName) const
{
  if (hasAttribute (attributeName))
    {
      return readerAttribute (attributeName, current.getElement ()[attributeName].asString ());
    }
  return readerAttribute ();
}

std::string jsonReaderElement::getAttributeText (const std::string &attributeName) const
{
  if (hasAttribute (attributeName))
    {
      return current.getElement ()[attributeName].asString ();
    }
  return nullStr;
}

double jsonReaderElement::getAttributeValue (const std::string &attributeName) const
{
  if (hasAttribute (attributeName))
    {

      if (current.getElement ()[attributeName].isConvertibleTo (Json::ValueType::realValue))
        {
          return current.getElement ()[attributeName].asDouble ();
        }
      else
        {
          double val = doubleReadComplete (current.getElement ()[attributeName].asString (), readerNullVal);
          return val;
        }


    }
  return readerNullVal;
}

std::shared_ptr<readerElement> jsonReaderElement::firstChild () const
{
  if (current.isNull ())
    {
      return nullptr;
    }

  auto newElement = clone ();
  newElement->moveToFirstChild ();
  return newElement;

}

std::shared_ptr<readerElement> jsonReaderElement::firstChild (const std::string &childName) const
{
  if (current.isNull ())
    {
      return nullptr;
    }
  auto newElement = clone ();
  newElement->moveToFirstChild (childName);
  return newElement;
}




void jsonReaderElement::moveToFirstChild ()
{
  if (current.isNull ())
    {
      return;
    }
  current.elementIndex = 0;
  auto elementIterator = current.getElement ().begin ();
  auto endIterator = current.getElement ().end ();

  while (elementIterator != endIterator)
    {

      if (isElement (*elementIterator))
        {
          parents.push_back (current);
          current = jsonElement (*elementIterator, elementIterator.name ());
          return;
        }
      ++elementIterator;
      ++current.elementIndex;
    }
  parents.push_back (current);
  current = jsonElement (Json::nullValue, "");

}

void jsonReaderElement::moveToFirstChild (const std::string &childName)
{
  if (current.isNull ())
    {
      return;
    }

  if (current.getElement ().isMember (childName))
    {
      if (isElement (current.getElement ()[childName]))
        {
          parents.push_back (current);
          current = jsonElement (current.getElement ()[childName], childName);
          return;
        }
    }

  parents.push_back (current);
  current = jsonElement (Json::nullValue, "");

}

void jsonReaderElement::moveToNextSibling ()
{
  if (current.isNull ())
    {
      return;
    }
  ++current.arrayIndex;
  while (current.arrayIndex < current.count ())
    {
      if (!current.getElement ().empty ())
        {
          return;
        }
      ++current.arrayIndex;
    }
  if (parents.empty ())
    {
      current = jsonElement (Json::nullValue, "");
      return;
    }
  //there are no more elements in a potential array
  auto elementIterator = parents.back ().getElement ().begin ();
  auto endIterator = parents.back ().getElement ().end ();
  ++parents.back ().elementIndex;
  //iterators don't survive copy so have to move the iterator to the next element index
  for (int ii = 0; ii < parents.back ().elementIndex; ++ii)
    {
      ++elementIterator;
      if (elementIterator == endIterator)
        {
          current = jsonElement (Json::nullValue, "");
        }
    }
  //Now find the next valid element
  while (elementIterator != endIterator)
    {

      if (isElement (*elementIterator))
        {
          current = jsonElement (*elementIterator, elementIterator.name ());
          return;
        }
      ++elementIterator;
      ++parents.back ().elementIndex;
    }
  current = jsonElement (Json::nullValue, "");

}

void jsonReaderElement::moveToNextSibling (const std::string &siblingName)
{
  if (current.isNull ())
    {
      return;
    }
  if (siblingName == current.name)
    {
      ++current.arrayIndex;
      while (current.arrayIndex < current.count ())
        {
          if (!current.getElement ().empty ())
            {
              return;
            }
          ++current.arrayIndex;
        }
      current = jsonElement (Json::nullValue, "");
    }
  else
    {
      if (parents.back ().getElement ().isMember (siblingName))
        {
          if (isElement (parents.back ().getElement ()[siblingName]))
            {
              current = jsonElement (parents.back ().getElement ()[siblingName], siblingName);
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
  if (current.isNull ())
    {
      return nullptr;
    }
  auto newElement = clone ();
  newElement->moveToNextSibling ();
  return newElement;
}

std::shared_ptr<readerElement> jsonReaderElement::nextSibling (const std::string &siblingName) const
{
  if (current.isNull ())
    {
      return nullptr;
    }
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
