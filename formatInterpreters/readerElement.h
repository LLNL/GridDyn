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

#ifndef READERELEMENT_H_
#define READERELEMENT_H_

#include <string>
#include <memory>

const double readerNullVal(-1.345e48);
/** @brief simple class for containing an attribute */
class readerAttribute
{
private:
  std::string name;
  std::string text;
public:
  readerAttribute ();
  readerAttribute (std::string attName, std::string attText);
  void set (std::string attName, std::string attText);
  const std::string &getName () const
  {
    return name;
  }
  const std::string &getText () const
  {
    return text;
  }
  double getValue () const;
  long long getInt() const;
  bool isValid () const
  {
    return (!name.empty ());
  }
};

/** @brief class for wrapping various document readers for use in the reader function for Griddyn
 readerElement is a abstract virtual class requiring an instantiation for implementation
*/
class readerElement
{
public:
  virtual ~readerElement () = 0;
  /** @brief load a file into the element
  @param[in] filename
  @return true if the file was loaded successfully
  */
  virtual bool loadFile (const std::string &filename) = 0;

  /** @brief parse a string into the element
  @param[in] inputString the string to parse
  @return true if the file was loaded successfully
  */
  virtual bool parse (const std::string &inputString) = 0;
  /** @brief copy to the element to a new shared ptr object*/
  virtual std::shared_ptr<readerElement> clone () const = 0;

  virtual std::string getName () const = 0;
  virtual double getValue () const = 0;
  virtual std::string getText () const = 0;
  /** @brief get all the text in an element even if they are in multiple sections
  @param[in] sep the separator to place between multiple sections of text
  @return all the text in  string*/
  virtual std::string getMultiText (const std::string sep = " ") const = 0;

  virtual bool hasAttribute (const std::string &attributeName) const = 0;
  virtual bool hasElement (const std::string &elementName) const = 0;
  virtual readerAttribute getFirstAttribute () = 0;
  virtual readerAttribute getNextAttribute () = 0;
  virtual readerAttribute getAttribute (const std::string &attributeName) const = 0;
  virtual std::string getAttributeText (const std::string &attributeName) const = 0;
  virtual double getAttributeValue (const std::string &attributeName) const = 0;

  virtual std::shared_ptr<readerElement> firstChild () const = 0;
  virtual std::shared_ptr<readerElement> firstChild (const std::string &childName) const = 0;

  virtual void moveToNextSibling () = 0;
  virtual void moveToNextSibling (const std::string &siblingName) = 0;

  virtual void moveToFirstChild () = 0;
  virtual void moveToFirstChild (const std::string &childName) = 0;

  virtual void moveToParent () = 0;

  virtual std::shared_ptr<readerElement> nextSibling () const = 0;
  virtual std::shared_ptr<readerElement> nextSibling (const std::string &siblingName) const = 0;
  /** check if the current element is valid
   *@return true if valid false otherwise
   */
  virtual bool isValid () const = 0;
  /** @brief check if the current element is a root document */
  virtual bool isDocument () const = 0;
  /** @brief save the current location */
  virtual void bookmark () = 0;
  /** @brief restore the last bookmarked location */
  virtual void restore () = 0;
};


#endif
