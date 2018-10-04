/*
 * LLNS Copyright Start
 * Copyright (c) 2014-2018, Lawrence Livermore National Security
 * This work was performed under the auspices of the U.S. Department
 * of Energy by Lawrence Livermore National Laboratory in part under
 * Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
 * Produced at the Lawrence Livermore National Laboratory.
 * All rights reserved.
 * For details, see the LICENSE file.
 * LLNS Copyright End
 */
#pragma once

#include "readerElement.h"
#include <memory>
#include <vector>

namespace toml
{
class Value;
struct ParseResult;
}  // namespace toml

class tomlElement;

/** @brief class defines a reader element around the toml reader*/
class tomlReaderElement : public readerElement
{
  public:
    tomlReaderElement ();
    explicit tomlReaderElement (const std::string &fileName);

    std::shared_ptr<readerElement> clone () const override;

    virtual bool isValid () const override;
    virtual bool isDocument () const override;

    virtual bool loadFile (const std::string &fileName) override;
    virtual bool parse (const std::string &inputString) override;
    virtual std::string getName () const override;
    virtual double getValue () const override;
    virtual std::string getText () const override;
    virtual std::string getMultiText (const std::string &sep = " ") const override;

    virtual bool hasAttribute (const std::string &attributeName) const override;
    virtual bool hasElement (const std::string &elementName) const override;

    virtual readerAttribute getFirstAttribute () override;
    virtual readerAttribute getNextAttribute () override;
    virtual readerAttribute getAttribute (const std::string &attributeName) const override;
    virtual std::string getAttributeText (const std::string &attributeName) const override;
    virtual double getAttributeValue (const std::string &attributeName) const override;

    virtual std::shared_ptr<readerElement> firstChild () const override;
    virtual std::shared_ptr<readerElement> firstChild (const std::string &childName) const override;

    virtual void moveToNextSibling () override;
    virtual void moveToNextSibling (const std::string &siblingName) override;

    virtual void moveToFirstChild () override;
    virtual void moveToFirstChild (const std::string &childName) override;

    virtual void moveToParent () override;

    virtual std::shared_ptr<readerElement> nextSibling () const override;
    virtual std::shared_ptr<readerElement> nextSibling (const std::string &siblingName) const override;

    virtual void bookmark () override;
    virtual void restore () override;

  private:
    void clear ();

  private:
    std::shared_ptr<toml::ParseResult> doc;  //!< document root
    std::vector<std::shared_ptr<tomlElement>> parents;
    std::shared_ptr<tomlElement> current;
    int iteratorCount = 0;

    std::vector<std::shared_ptr<tomlReaderElement>> bookmarks;
};
