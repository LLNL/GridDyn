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


#include "gridDynFileInput.h"
#include "objectFactory.h"
#include "stringOps.h"
#include "objectInterpreter.h"
#include "core/gridDynExceptions.h"
#include "gridBus.h"
#include "linkModels/gridLink.h"
#include "relays/gridRelay.h"
#include "gridArea.h"
#include "readerHelper.h"

#include <fstream>
#include <cstdlib>
#include <iostream>

using namespace gridUnits;
using namespace readerConfig;

enum mode_state
{
  read_header, read_data
};

void loadCSV (coreObject *parentObject,const std::string &filename, readerInfo *ri, const std::string &oname)
{
  auto cof = coreObjectFactory::instance ();
  std::ifstream file (filename, std::ios::in);
  if (!(file.is_open ()))
    {
      std::cerr << "Unable to open file " << filename << '\n';
      return;
    }
  std::string line;        //line storage
  int lineNumber = 0;
  std::string temp1;        //temporary storage for substrings
  stringVec lineTokens;
  stringVec headers;
  std::vector<int> skipToken;
  std::vector<units_t> units;
  std::string ObjectMode;
  coreObject *obj = nullptr;
  int typekey = -1;
  int refkey = -1;
  std::string type;
  std::string ref;
  std::string field;
  double val;
  int ret;
  std::string str;
  gridBus *bus = nullptr;
  coreObject *obj2 = nullptr;
  mode_state mState = read_header;


  //loop over the sections
  while (std::getline ( file, line ))
    {
      ++lineNumber;
      if (line[0] == '#')
        {
          if (line[1] == '#')              //new section
            {
              mState = read_header;
              continue;
            }
          else                //general comment line
            {
              continue;
            }
        }
      if (mState == read_header)
        {
          headers = splitlineTrim (line);
          ObjectMode = headers[0];
          makeLowerCase (ObjectMode);
          //translate a few mode possibilities
          if ((ObjectMode == "branch") || (ObjectMode == "line"))
            {
              ObjectMode = "link";
            }
          if (!(cof->isValidObject (ObjectMode)))
            {
              if (!oname.empty ())
                {
                  if (!(cof->isValidObject (oname)))
                    {
                      ObjectMode = oname;
                    }
                  else
                    {
                      WARNPRINT (READER_WARN_IMPORTANT, "Unrecognized object " << ObjectMode << " Unable to process CSV");
                      return;
                    }
                }
              else
                {
                  WARNPRINT (READER_WARN_IMPORTANT, "Unrecognized object " << ObjectMode << " Unable to process CSV");
                  return;
                }
            }


          units = std::vector<units_t> (headers.size (), defUnit);
          skipToken.resize (headers.size (), 0);
          typekey = -1;
          int nn = 0;
          for (auto &tk : headers)
            {
              trimString (tk);

              if (tk.empty ())
                {
                  continue;
                }
              if (tk[0] == '#')
                {
                  tk.clear ();
                  continue;
                }
              makeLowerCase (tk);
              if (tk == "type")
                {
                  typekey = nn;
                }
              if ((tk == "ref") || (tk == "reference"))
                {
                  refkey = nn;
                }
              if (tk.back () == ')')
                {
                  auto p = tk.find_first_of ('(');
                  if (p != std::string::npos)
                    {
                      std::string uname = tk.substr (p + 1, tk.length () - 2 - p);
                      units[nn] = getUnits (uname);
                      tk = trim(tk.substr (0, p));
                    }
                }
              ++nn;
            }

          mState = read_data;
          //skip the reference
          if (refkey > 0)
            {
              skipToken[refkey] = 4;
            }
        }
      else
        {
          lineTokens = splitline (line);
          if (lineTokens.size () != headers.size ())
            {
              fprintf (stderr, "line %d length does not match section header\n", lineNumber);
              return;
            }
          //check the identifier
          ref = (refkey >= 0) ? trim ( lineTokens[refkey]) : "";
          type = (typekey >= 0) ? trim (lineTokens[typekey]) : "";
          //find or create the object
          int index = intRead (lineTokens[0], -2);
          if (index >= 0)
            {
              obj = parentObject->findByUserID (ObjectMode, index);
            }
          else if (index == -2)
            {
              obj = locateObject (str,parentObject);
            }
          if (refkey >= 0)
            {
              obj = ri->makeLibraryObject (ref, obj);
            }

          if (!(obj))
            {

              obj = cof->createObject (ObjectMode, type);
              if (obj)
                {
                  if (index > 0)
                    {
                      obj->setUserID (index);
                    }
                  else if (index == -2)
                    {
                      obj->setName ( str);
                    }
                }
            }


          if (!(obj))
            {
              std::cerr << "Line " << lineNumber << "::Unable to create object " << ObjectMode << " of Type " << type << '\n';
              return;
            }
          //

          for (size_t kk = 1; kk < lineTokens.size (); kk++)
            {
              //check if we just skip this one
              if (skipToken[kk] > 2)
                {
                  continue;
                }
              //make sure there is something in the field
              if (lineTokens[kk].empty ())
                {
                  continue;
                }

              field = headers[kk];
              if (field.empty ())
                {
                  skipToken[kk] = 4;
                }
              makeLowerCase (field);
              if (field == "type")
                {
                  if (ObjectMode == "bus")
                    {
                      str = lineTokens[kk];
                      trimString (str);
                      obj->set ("type", lineTokens[kk]);
                    }
                }
              else if ((field == "name") || (field == "description"))
                {
                  str = lineTokens[kk];
                  trimString (str);
                  str = ri->checkDefines (str);
                  obj->set (field, str);
                }
              else if ((field.substr (0,2) == "to") && (ObjectMode == "link"))
                {
                  str = lineTokens[kk];

                  str = ri->checkDefines (str);
                  paramRead (str, val, kBigNum);
                  if (val < kHalfBigNum)
                    {
                      bus = static_cast<gridBus *> (parentObject->findByUserID ("bus", static_cast<int> (val)));
                    }
                  else
                    {
                      trimString (str);
                      bus = static_cast<gridBus *> (locateObject (str, parentObject));
                    }
                  if (bus)
                    {
                      static_cast<gridLink *> (obj)->updateBus ( bus,2);
                    }

                }
              else if ((field.substr (0,4) == "from") && (ObjectMode == "link"))
                {
                  str = lineTokens[kk];
                  str = ri->checkDefines (str);
                  paramRead (str, val, kBigNum);
                  if (val < kHalfBigNum)
                    {
                      bus = static_cast<gridBus *> (parentObject->findByUserID ("bus", static_cast<int> (val)));
                    }
                  else
                    {
                      trimString (str);
                      bus = static_cast<gridBus *> (locateObject (str, parentObject));
                    }
                  if (bus)
                    {
                      static_cast<gridLink *> (obj)->updateBus (bus, 1);
                    }
                  else
                    {
                      WARNPRINT (READER_WARN_ALL, "line " << lineNumber << ":: unable to locate bus object  " << str);
                    }

                }
              else if ((field == "bus") && ((ObjectMode == "load")||(ObjectMode == "gen" )))
                {

                  str = lineTokens[kk];
                  str = ri->checkDefines (str);
                  paramRead (str, val, kBigNum);
                  if (val < kHalfBigNum)
                    {
                      bus = static_cast<gridBus *> (parentObject->findByUserID ("bus", static_cast<int> (val)));
                    }
                  else
                    {
                      bus = static_cast<gridBus *> (locateObject (str, parentObject));
                    }
                  if (bus)
                    {
                      bus->add (obj);
                    }
                  else
                    {
                      WARNPRINT (READER_WARN_ALL, "line " << lineNumber << ":: unable to locate bus object  " << str);
                    }
                }
              else if (((field == "target")||(field == "sink")||(field == "source")) && (ObjectMode == "relay"))
                {

                  str = lineTokens[kk];
                  str = ri->checkDefines (str);
                  obj2 = locateObject (str, parentObject);
                  if (obj2)
                    {
                      if (field != "sink")
                        {
                          (static_cast<gridRelay*> (obj))->setSource (obj2);
                        }
                      if (field != "source")
                        {
                          (static_cast<gridRelay*> (obj))->setSink (obj2);
                        }
                    }
                  else
                    {
                      WARNPRINT (READER_WARN_ALL, "line " << lineNumber << ":: unable to locate object  " << str);
                    }
                }
              else if (field == "file")
                {
                  str = lineTokens[kk];
                  ri->checkFileParam (str);
				  gridParameter po(field, str);

				  objectParameterSet(std::to_string(lineNumber), obj, po);
                }
              else if (field == "workdir")
                {
                  str = lineTokens[kk];
                  ri->checkDirectoryParam (str);
				  gridParameter po(field, str);
				  
				  objectParameterSet(std::to_string(lineNumber), obj, po);
                }
              else
                {
                  str = lineTokens[kk];
                  str = ri->checkDefines (str);
                  val = doubleRead (str, kBigNum);
				  
                  if (val < kHalfBigNum)
                    {
					  gridParameter po(field, val);
					  po.paramUnits = units[kk];
					  objectParameterSet(std::to_string(lineNumber), obj, po);

                    }
                  else
                    {
                      trimString (str);                          //check for empty string and if so ignore it
                      if (str.empty ())
                        {
                          continue;
                        }
                      gridParameter po (field, str);
                      paramStringProcess (&po, ri);
					  ret=objectParameterSet(std::to_string(lineNumber), obj, po);
                      
                      if (ret != 0)
                        {
                          skipToken[kk] += 1;
                        }
                    }


                }
            }
          if (obj->getParent () == nullptr)
            {
              if (!(ri->prefix.empty ()))
                {
                  obj->setName (ri->prefix + '_' + obj->getName ());
                }
			  try
			  {
				  parentObject->add(obj);
			  }
			  catch (const coreObjectException &uroe)
			  {
				  WARNPRINT(READER_WARN_ALL, "line " << lineNumber << ":: "<<uroe.what()<<" "<< uroe.who());
			  }
			
            }

        }
    }
}

