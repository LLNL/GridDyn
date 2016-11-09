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

#include "tinyxmlReaderElement.h"
#include "ticpp/ticpp.h"
#include "stringOps.h"


tinyxmlReaderElement::tinyxmlReaderElement ()
{

}

tinyxmlReaderElement::tinyxmlReaderElement (const std::string &filename)
{
  loadFile (filename);
}


tinyxmlReaderElement::tinyxmlReaderElement (const ticpp::Element *ticppElement, const ticpp::Element *ticppParent) : element (ticppElement), parent (ticppParent)
{

}

tinyxmlReaderElement::~tinyxmlReaderElement ()
{
}

void tinyxmlReaderElement::clear ()
{
  element = nullptr;
  parent = nullptr;
  att = nullptr;
  bookmarks.clear ();
}

bool tinyxmlReaderElement::isValid () const
{
  return ((element) || ((parent == nullptr)&&(doc)));
}

bool tinyxmlReaderElement::isDocument () const
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


std::shared_ptr<readerElement> tinyxmlReaderElement::clone () const
{
  auto ret = std::make_shared<tinyxmlReaderElement> (element, parent);
  ret->doc = doc;
  return ret;
}

bool tinyxmlReaderElement::loadFile (const std::string &filename)
{
  try
    {
      doc = std::make_shared<ticpp::Document> (filename);
      doc->LoadFile ();
      clear ();
      element = doc->FirstChildElement (false);
      return true;
    }
  catch (ticpp::Exception& ex)
    {
      std::cerr << ex.m_details << std::endl;
      doc = nullptr;
      clear ();
      return false;
    }
}


bool tinyxmlReaderElement::parse (const std::string &string)
{
  doc = std::make_shared<ticpp::Document> ();
  try
    {
      doc->Parse (string.data (), string.length ());
      clear ();
      element = doc->FirstChildElement (false);
      return true;
    }
  catch (ticpp::Exception& ex)
    {
      std::cerr << ex.m_details << std::endl;
      doc = nullptr;
      clear ();
      return false;
    }
}

std::string tinyxmlReaderElement::getName () const
{
  std::string name;
  if (element)
    {
      element->GetValue (&name);

    }
  else if (doc)
    {
      doc->GetValue (&name);
    }
  return name;
}

double tinyxmlReaderElement::getValue () const
{
  if (element)
    {
      double ret = doubleReadComplete (element->GetText (false), readerNullVal);
      return ret;
    }
  return readerNullVal;
}

std::string tinyxmlReaderElement::getText () const
{
  if (element)
    {
      return element->GetText (false);
    }
  return "";
}

std::string tinyxmlReaderElement::getMultiText (const std::string sep) const
{
  if (element)
    {
      ticpp::Iterator< ticpp::Text > child;
      std::string text;
      std::string ret = "";
      for (child = child.begin (element); child != child.end (); child++)
        {
          child->GetValue (&text);
          if (!text.empty ())
            {
              if (ret.empty ())
                {
                  ret = text;
                }
              else
                {
                  ret += sep + text;
                }

            }
        }
      return ret;
    }
  return "";
}

bool tinyxmlReaderElement::hasAttribute (const std::string &attributeName) const
{
  if (element)
    {
      return element->HasAttribute (attributeName);
    }
  return false;
}

bool tinyxmlReaderElement::hasElement (const std::string &elementName) const
{
  if (element)
    {
      auto testElement = element->FirstChildElement (elementName,false);
      if (testElement)
        {
          return true;
        }
    }
  return false;
}

readerAttribute tinyxmlReaderElement::getFirstAttribute ()
{
  if (element)
    {
      att = element->FirstAttribute (false);
      if (att)
        {
          std::string val;
          att->GetValue (&val);
          return readerAttribute (att->Name (), val);
        }
    }
  return readerAttribute ();
}

readerAttribute tinyxmlReaderElement::getNextAttribute ()
{
  if (att)
    {
      att = att->Next (false);
      if (att)
        {
          std::string val;
          att->GetValue (&val);
          return readerAttribute (att->Name (), val);
        }
    }
  return readerAttribute ();
}

readerAttribute tinyxmlReaderElement::getAttribute (const std::string &attributeName) const
{
  if (element)
    {
      std::string val = element->GetAttribute (attributeName);
      if (!val.empty ())
        {
          return readerAttribute (attributeName, val);
        }
    }
  return readerAttribute ();
}

std::string tinyxmlReaderElement::getAttributeText (const std::string &attributeName) const
{
  if (element)
    {
      return element->GetAttribute (attributeName);
    }
  return "";
}

double tinyxmlReaderElement::getAttributeValue (const std::string &attributeName) const
{
  if (element)
    {
      return doubleReadComplete (element->GetAttribute (attributeName), readerNullVal);
    }
  return readerNullVal;
}

std::shared_ptr<readerElement> tinyxmlReaderElement::firstChild () const
{
  ticpp::Element *child = nullptr;
  if (element)
    {
      child = element->FirstChildElement (false);
    }
  else if (isDocument ())
    {
      child = doc->FirstChildElement (false);
    }
  if (child)
    {
      auto a = std::make_shared<tinyxmlReaderElement> (child,element);
      return a;
    }
  else
    {
      return nullptr;
    }

}

std::shared_ptr<readerElement> tinyxmlReaderElement::firstChild (const std::string &childName) const
{
  ticpp::Element *child = nullptr;
  if (element)
    {
      child = element->FirstChildElement (childName,false);
    }
  else if (isDocument ())
    {
      child = doc->FirstChildElement (childName,false);
    }
  if (child)
    {
      return std::make_shared<tinyxmlReaderElement> (child,element);
    }
  else
    {
      return nullptr;
    }
}

void tinyxmlReaderElement::moveToNextSibling ()
{
  if (element)
    {
      element = element->NextSiblingElement (false);
      att = nullptr;
    }
}

void tinyxmlReaderElement::moveToNextSibling (const std::string &siblingName)
{
  if (element)
    {
      element = element->NextSiblingElement (siblingName,false);
      att = nullptr;
    }

}


void tinyxmlReaderElement::moveToFirstChild ()
{
  if (element)
    {
      parent = element;
      att = nullptr;
      element = element->FirstChildElement (false);
    }
  else if (isDocument ())
    {
      element = doc->FirstChildElement (false);
    }

}

void tinyxmlReaderElement::moveToFirstChild (const std::string &childName)
{
  if (element)
    {
      parent = element;
      att = nullptr;
      element = element->FirstChildElement (childName,false);
    }
  else if (isDocument ())
    {
      element = doc->FirstChildElement (childName,false);
    }
}

void tinyxmlReaderElement::moveToParent ()
{
  if (parent)
    {
      element = parent;
      att = nullptr;
      auto temp = element->Parent (false);
      if (temp)
        {
          if (temp->Type () == TiXmlNode::ELEMENT)
            {
              parent = temp->ToElement ();
            }
          else
            {
              parent = nullptr;
            }
        }
      else
        {
          parent = nullptr;
        }

    }
}

std::shared_ptr<readerElement> tinyxmlReaderElement::nextSibling () const
{
  if (element)
    {
      auto sibling = element->NextSiblingElement (false);
      if (sibling)
        {
          return std::make_shared<tinyxmlReaderElement> (sibling,parent);
        }
    }
  return nullptr;
}

std::shared_ptr<readerElement> tinyxmlReaderElement::nextSibling (const std::string &siblingName) const
{
  if (element)
    {
      auto sibling = element->NextSiblingElement (siblingName,false);
      if (sibling)
        {
          return std::make_shared<tinyxmlReaderElement> (sibling,parent);
        }
    }
  return nullptr;
}



void tinyxmlReaderElement::bookmark ()
{
  bookmarks.emplace_back (element, parent);
}

void tinyxmlReaderElement::restore ()
{
  if (bookmarks.empty ())
    {
      return;
    }
  element = bookmarks.back ().first;
  parent = bookmarks.back ().second;
  bookmarks.pop_back ();
}
