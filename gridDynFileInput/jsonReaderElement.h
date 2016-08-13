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

#ifndef JSONREADERELEMENT_H_
#define JSONREADERELEMENT_H_

#include "readerElement.h"
#include "json/json-forwards.h"
#include "json/json.h"
#include <memory>


/** @brief class defines a reader element around the ticpp element XML reader*/
class jsonReaderElement : public readerElement
{
public:
  jsonReaderElement ();
  jsonReaderElement (const std::string &filename);

  virtual ~jsonReaderElement () override;

  std::shared_ptr<readerElement> clone () const override;

  virtual bool isValid () const override;
  virtual bool isDocument () const override;

  virtual bool loadFile (const std::string &filename) override;
  virtual bool parse (const std::string &inputString) override;
  virtual std::string getName () const override;
  virtual double getValue () const override;
  virtual std::string getText () const override;
  virtual std::string getMultiText (const std::string sep = " ") const override;

  virtual bool hasAttribute (const std::string &attributeName) const override;
  virtual bool hasElement (const std::string &elementName) const override;

  virtual readerAttribute getFirstAttribute () override;
  virtual readerAttribute getNextAttribute () override;
  virtual readerAttribute getAttribute (const std::string &attributeName) const override;
  virtual std::string getAttributeText (const std::string &attributeName) const override;
  virtual double getAttributeValue (const std::string &attributeName) const override;

  virtual std::shared_ptr<readerElement> firstChild () const override;
  virtual std::shared_ptr<readerElement> firstChild (const std::string &childName) const override;

  virtual void moveToNextSibling ()  override;
  virtual void moveToNextSibling (const std::string &siblingName)  override;

  virtual void moveToFirstChild ()  override;
  virtual void moveToFirstChild (const std::string &childName) override;

  virtual void moveToParent ()  override;

  virtual std::shared_ptr<readerElement> nextSibling () const  override;
  virtual std::shared_ptr<readerElement> nextSibling (const std::string &siblingName) const  override;

  virtual void bookmark () override;
  virtual void restore () override;
private:
  void clear ();
  bool isAttribute (const Json::Value &testValue) const;
  bool isElement (const Json::Value &testValue) const;
private:
  class jsonElement
  {
public:
    int elementIndex = 0;
    std::string name;
    Json::ArrayIndex arrayIndex = 0;
    jsonElement ()
    {
    }
    jsonElement (Json::Value vElement, std::string newName);
    void clear ();
    const Json::Value &getElement () const
    {
      return (arraytype) ? element[arrayIndex] : element;
    }
    Json::ArrayIndex count () const
    {
      return (arraytype) ? element.size () : Json::ArrayIndex (1);
    }
    bool isNull () const
    {
      return (arraytype) ? element[arrayIndex].isNull () : element.isNull ();
    }
private:
    Json::Value element = Json::nullValue;
    bool arraytype = false;

  };

  std::shared_ptr<Json::Value> doc;             //!<document root
  std::vector<jsonElement> parents;
  jsonElement current;
  Json::ValueConstIterator attIterator;

  std::vector<std::shared_ptr<jsonReaderElement> > bookmarks;
};

#endif

