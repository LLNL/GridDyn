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

#include "readerElement.h"
#include "gridDynTypes.h"
#include "stringOps.h"


readerAttribute::readerAttribute ()
{

}

readerAttribute::readerAttribute (std::string attName, std::string attText) : name (attName), text (attText)
{

}

void readerAttribute::set (std::string attName, std::string attText)
{
  name = attName;
  text = attText;
}

double readerAttribute::getValue () const
{
  return doubleReadComplete (text, kNullVal);
}

readerElement::~readerElement ()
{
}

