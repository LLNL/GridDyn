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

#include "tinyxml2ReaderElement.h"
#include "tinyxml2/tinyxml2.h"
#include "utilities/stringConversion.h"


using namespace tinyxml2;

tinyxml2ReaderElement::tinyxml2ReaderElement ()
{

}

tinyxml2ReaderElement::tinyxml2ReaderElement (const std::string &filename)
{
  loadFile (filename);
}


tinyxml2ReaderElement::tinyxml2ReaderElement (const XMLElement *xmlElement, const XMLElement *xmlParent) : element (xmlElement), parent (xmlParent)
{

}

tinyxml2ReaderElement::~tinyxml2ReaderElement ()
{
}

void tinyxml2ReaderElement::clear ()
{
  element = nullptr;
  parent = nullptr;
  att = nullptr;
  bookmarks.clear ();
}

bool tinyxml2ReaderElement::isValid () const
{
  return ((element) || ((parent == nullptr)&&(doc)));
}

bool tinyxml2ReaderElement::isDocument () const
{
  if (parent == nullptr)
    {
      if (doc)
        {
          return true;
        }
    }
  return false;
}


std::shared_ptr<readerElement> tinyxml2ReaderElement::clone () const
{
  auto ret = std::make_shared<tinyxml2ReaderElement> (element, parent);
  ret->doc = doc;
  return ret;
}

bool tinyxml2ReaderElement::loadFile (const std::string &filename)
{
  doc = std::make_shared<XMLDocument> (true, COLLAPSE_WHITESPACE);
  XMLError res = doc->LoadFile (filename.c_str ());
  clear ();
  if (res == XML_NO_ERROR)
    {
      element = doc->FirstChildElement ();
      return true;
    }
  else
    {
      doc = nullptr;
      return false;
    }
}


bool tinyxml2ReaderElement::parse (const std::string &string)
{
  doc = std::make_shared<XMLDocument> (true, COLLAPSE_WHITESPACE);
  XMLError res = doc->Parse (string.data (), string.length ());
  clear ();
  if (res == XML_NO_ERROR)
    {
      element = doc->FirstChildElement ();
      return true;
    }
  else
    {
      doc = nullptr;
      return false;
    }
}

std::string tinyxml2ReaderElement::getName () const
{
  if (element)
    {
      auto cname = element->Name ();
      if (cname)
        {
          return std::string (cname);
        }

    }
  return "";
}

double tinyxml2ReaderElement::getValue () const
{
  if (element)
    {
      auto cText = element->GetText ();
      if (cText)
        {
          double val = numeric_conversionComplete (std::string (cText), readerNullVal);
          return val;
        }
      //double ret = numeric_conversionComplete(element->GetText(false), kNullVal);
      //return ret;
    }
  return readerNullVal;
}

std::string tinyxml2ReaderElement::getText () const
{
  if (element)
    {
      auto cname = element->GetText ();
      if (cname)
        {
          return std::string (cname);
        }
    }
  return "";
}

std::string tinyxml2ReaderElement::getMultiText (const std::string &sep) const
{
  std::string ret = "";
  if (element)
    {
      auto childNode = element->FirstChild ();
      while (childNode)
        {
          auto textChildNode = childNode->ToText ();
          if (textChildNode)
            {
              auto c = textChildNode->Value ();
              if (ret.empty ())
                {
                  ret = std::string (c);
                }
              else
                {
                  ret += sep + std::string (c);
                }
            }
          childNode = childNode->NextSibling ();
        }

    }
  return ret;
}

bool tinyxml2ReaderElement::hasAttribute (const std::string &attributeName) const
{
  if (element)
    {
      auto A = element->Attribute (attributeName.c_str ());
      return (A == nullptr);
    }
  return false;
}

bool tinyxml2ReaderElement::hasElement (const std::string &elementName) const
{
  if (element)
    {
      auto testElement = element->FirstChildElement (elementName.c_str ());
      if (testElement)
        {
          return true;
        }
    }
  return false;
}

readerAttribute tinyxml2ReaderElement::getFirstAttribute ()
{
  if (element)
    {
      att = element->FirstAttribute ();
      if (att)
        {
          return readerAttribute (std::string (att->Name ()),std::string (att->Value ()));
        }
    }
  return readerAttribute ();
}

readerAttribute tinyxml2ReaderElement::getNextAttribute ()
{
  if (att)
    {
      att = att->Next ();
      if (att)
        {
          return readerAttribute (std::string (att->Name ()), std::string (att->Value ()));
        }
    }
  return readerAttribute ();
}

readerAttribute tinyxml2ReaderElement::getAttribute (const std::string &attributeName) const
{
  if (element)
    {
      auto c = element->Attribute (attributeName.c_str ());
      if (c)
        {
          return readerAttribute (attributeName, std::string (c));
        }
    }
  return readerAttribute ();
}

std::string tinyxml2ReaderElement::getAttributeText (const std::string &attributeName) const
{
  if (element)
    {
      auto c = element->Attribute (attributeName.c_str ());
      if (c)
        {
          return std::string (c);
        }
    }
  return "";
}

double tinyxml2ReaderElement::getAttributeValue (const std::string &attributeName) const
{
  if (element)
    {
      auto c = element->Attribute (attributeName.c_str ());
      if (c)
        {
          double val = numeric_conversionComplete (std::string (c), readerNullVal);
          return val;
        }
    }
  return readerNullVal;
}

std::shared_ptr<readerElement> tinyxml2ReaderElement::firstChild () const
{
  const XMLElement *child = nullptr;
  if (element)
    {
      child = element->FirstChildElement ();
    }
  else if (isDocument ())
    {
      child = doc->FirstChildElement ();
    }
  if (child)
    {
      auto a = std::make_shared<tinyxml2ReaderElement> (child,element);
      return a;
    }
  else
    {
      return nullptr;
    }

}

std::shared_ptr<readerElement> tinyxml2ReaderElement::firstChild (const std::string &childName) const
{
  const XMLElement *child = nullptr;
  if (element)
    {
      child = element->FirstChildElement (childName.c_str ());
    }
  else if (isDocument ())
    {
      child = doc->FirstChildElement (childName.c_str ());
    }
  if (child)
    {
      auto a = std::make_shared<tinyxml2ReaderElement> (child,element);
      return a;
    }
  else
    {
      return nullptr;
    }
}

void tinyxml2ReaderElement::moveToNextSibling ()
{
  if (element)
    {
      element = element->NextSiblingElement ();
      att = nullptr;
    }
}

void tinyxml2ReaderElement::moveToNextSibling (const std::string &siblingName)
{
  if (element)
    {
      element = element->NextSiblingElement (siblingName.c_str ());
      att = nullptr;
    }

}


void tinyxml2ReaderElement::moveToFirstChild ()
{
  if (element)
    {
      parent = element;
      att = nullptr;
      element = element->FirstChildElement ();
    }
  else if (isDocument ())
    {
      element = doc->FirstChildElement ();
    }

}

void tinyxml2ReaderElement::moveToFirstChild (const std::string &childName)
{
  if (element)
    {
      parent = element;
      att = nullptr;
      element = element->FirstChildElement (childName.c_str ());
    }
  else if (isDocument ())
    {
      element = doc->FirstChildElement (childName.c_str ());
    }
}

void tinyxml2ReaderElement::moveToParent ()
{
  if (parent)
    {
      element = parent;
      att = nullptr;
      auto a = element->Parent ();
      if (a)
        {
          parent = a->ToElement ();
        }
      else
        {
          parent = nullptr;
        }
    }
}

std::shared_ptr<readerElement> tinyxml2ReaderElement::nextSibling () const
{
  if (element)
    {
      auto sibling = element->NextSiblingElement ();
      if (sibling)
        {
          return std::make_shared<tinyxml2ReaderElement> (sibling,parent);
        }
    }
  return nullptr;
}

std::shared_ptr<readerElement> tinyxml2ReaderElement::nextSibling (const std::string &siblingName) const
{
  if (element)
    {
      auto sibling = element->NextSiblingElement (siblingName.c_str ());
      if (sibling)
        {
          return std::make_shared<tinyxml2ReaderElement> (sibling,parent);
        }
    }
  return nullptr;
}



void tinyxml2ReaderElement::bookmark ()
{
  bookmarks.emplace_back (element, parent);
}

void tinyxml2ReaderElement::restore ()
{
  if (bookmarks.empty ())
    {
      return;
    }
  element = bookmarks.back ().first;
  parent = bookmarks.back ().second;
  bookmarks.pop_back ();
}
