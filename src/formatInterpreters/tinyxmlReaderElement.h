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

#ifndef TINYXMLREADERELEMENT_H_
#define TINYXMLREADERELEMENT_H_

#include "readerElement.h"

#include <memory>
#include <utility>
#include <vector>

namespace ticpp
{
class Document;
class Element;
class Attribute;
}  // namespace ticpp

/** @brief class defines a reader element around the ticpp element XML reader*/
class tinyxmlReaderElement : public readerElement
{
  private:
    std::shared_ptr<ticpp::Document> doc;  //!< document root
    const ticpp::Element *element = nullptr;  //!< pointer to current element
    const ticpp::Attribute *att = nullptr;  //!< pointer to current attribute
    const ticpp::Element *parent = nullptr;  //!< pointer to parent element
    std::vector<std::pair<const ticpp::Element *, const ticpp::Element *>>
      bookmarks;  //!< storage for recorded locations
  public:
    tinyxmlReaderElement () noexcept;
    tinyxmlReaderElement (const std::string &fileName);
    tinyxmlReaderElement (const ticpp::Element *ticppElement, const ticpp::Element *ticppParent);

    virtual ~tinyxmlReaderElement () override;

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
};

#endif
