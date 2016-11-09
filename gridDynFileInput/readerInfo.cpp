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

#include "readerInfo.h"
#include "gridDynFileInput.h"
#include "readerHelper.h"
#include "recorder_events/collector.h"
#include "readerElement.h"
#include "gridCore.h"


#include <boost/filesystem.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time.hpp>

#include <cmath>

using namespace readerConfig;

basicReaderInfo::basicReaderInfo ()
{
}


basicReaderInfo::basicReaderInfo (const basicReaderInfo *bri) : base (bri->base),
                                                                basefreq (bri->basefreq), flags (bri->flags),
                                                                version (bri->version), prefix (bri->prefix)
{

}

basicReaderInfo::~basicReaderInfo ()
{
}

readerInfo::readerInfo ()
{
  loadDefaultDefines ();
}

readerInfo::readerInfo (const basicReaderInfo *bri) : basicReaderInfo (*bri)
{

}


readerInfo::~readerInfo ()
{
  for (auto &libObj : library)
    {
      delete libObj.second.first;
    }
}

readerInfo::scopeID readerInfo::newScope ()
{
  if (keepdefines)
    {
      return 0;
    }
  ++currentScope;
  return currentScope;
}


void readerInfo::closeScope (scopeID scopeToClose)
{
  if (scopeToClose == 0)
    {
      if (currentScope > 0)
        {
          --currentScope;
        }
    }
  else
    {
      currentScope = scopeToClose - 1;
    }
  if (!scopedDefinitions.empty ())
    {
      while (std::get<0> (scopedDefinitions.back ()) > currentScope)
        {
          if (std::get<2> (scopedDefinitions.back ()))
            {
              defines[std::get < 1 > (scopedDefinitions.back ())] = std::get<3> (scopedDefinitions.back ());
            }
          else
            {
              defines.erase (std::get<1> (scopedDefinitions.back ()));
            }
          scopedDefinitions.pop_back ();
          if (scopedDefinitions.empty ())
            {
              break;
            }
        }
    }
  //now deal with scoped directory definitions
  if (!directoryScope.empty ())
    {
      while (directoryScope.back () > currentScope)
        {
          directories.pop_back ();
          directoryScope.pop_back ();
          if (directoryScope.empty ())
            {
              break;
            }
        }
    }
}


void readerInfo::addDefinition (std::string def, std::string replacement)
{
  auto search = lockDefines.find (def);
  if (search == lockDefines.end ())
    {
      if (currentScope > 0)
        {
          auto prevdef = defines.find (def);
          if (prevdef != defines.end ())
            {
              scopedDefinitions.emplace_back (currentScope, def,true,prevdef->second);
            }
          else
            {
              scopedDefinitions.emplace_back (currentScope, def,false,"");
            }

        }
      defines[def] = replacement;
    }
}


void readerInfo::addTranslate (std::string def, std::string component)
{
  objectTranslations[def] = component;
  parameterIgnoreStrings.insert (def);
}

void readerInfo::addTranslate (std::string def, std::string component, std::string type)
{
  objectTranslations[def] = component;
  objectTranslationsType[def] = type;
  parameterIgnoreStrings.insert (def);
}

void readerInfo::addTranslateType (std::string def, std::string type)
{
  objectTranslationsType[def] = type;
}


void readerInfo::addLockedDefinition (std::string def, std::string replacement)
{

  auto search = lockDefines.find (def);
  if (search == lockDefines.end ())
    {
      defines[def] = replacement;
      lockDefines[def] = replacement;
    }

}

void readerInfo::replaceLockedDefinition (std::string def, std::string replacement)
{
  defines[def] = replacement;
  lockDefines[def] = replacement;
}

std::string readerInfo::checkDefines (const std::string input)
{
  std::string out = input;
  int repcnt = 0;
  bool rep (true);
  while (rep)
    {
      ++repcnt;
      if (repcnt > 15)
        {
          WARNPRINT (READER_WARN_IMPORTANT, "probable definition recursion in " << input << "currently " << out);
          rep = false;
        }
      auto search = defines.find (out);
      if (search != defines.end ())
        {
          out = search->second;
          continue;
        }
      auto pos1 = out.find_first_of ('$');
      if (pos1 != std::string::npos)
        {
          auto pos2 = out.find_first_of ('$', pos1 + 1);
          if (pos2 != std::string::npos)
            {
              auto temp = out.substr (pos1 + 1, pos2 - pos1 - 1);
              search = defines.find (temp);
              if (search != defines.end ())
                {
                  out = out.substr (0, pos1) + search->second + out.substr (pos2 + 1);
                  continue;
                }
              else
                {
                  //try a recursive interpretation of the string block to convert a numeric result back into a string
                  double val = interpretString (temp, this);
                  if (!std::isnan (val))
                    {
                      if (std::abs (trunc (val) - val) < 1e-9)
                        {
                          out = out.substr (0, pos1) + std::to_string (static_cast<int> (val)) + out.substr (pos2 + 1);
                        }
                      else
                        {
                          out = out.substr (0, pos1) + std::to_string (val) + out.substr (pos2 + 1);
                        }
                      continue;
                    }
                }
            }
        }
      rep = false;
    }
  return out;
}


std::string readerInfo::objectNameTranslate (const std::string input)
{
  std::string out = input;
  int repcnt = 0;
  bool rep (true);
  while (rep)
    {
      ++repcnt;
      if (repcnt > 15)
        {
          WARNPRINT (READER_WARN_IMPORTANT, "probable Translation recursion in " << input << "currently " << out);
          rep = false;
        }
      auto search = objectTranslations.find (out);
      if (search != objectTranslations.end ())
        {
          out = search->second;
          continue;
        }
      rep = false;
    }
  return out;
}

bool readerInfo::addLibraryObject (gridCoreObject *obj, std::vector<gridParameter> &pobjs)
{
  auto retval = library.find (obj->getName ());
  if (retval == library.end ())
    {
      library[obj->getName ()] = std::make_pair (obj, pobjs);
	  obj->setName(obj->getName() + "_$"); //make sure all cloned object have a unique name
      return true;
    }
  else
    {
      return false;
    }

}

gridCoreObject *readerInfo::findLibraryObject (const std::string &ename) const
{
  auto retval = library.find (ename);
  if (retval == library.end ())
    {
      return nullptr;
    }
  else
    {
      return retval->second.first;
    }
}

const std::string libraryLabel = "library";
gridCoreObject *readerInfo::makeLibraryObject (const std::string &ename, gridCoreObject *mobj)
{
  auto retval = library.find (ename);
  if (retval == library.end ())
    {
      WARNPRINT (READER_WARN_ALL, "unknown reference object " << ename);
      return mobj;
    }
  else
    {
      gridCoreObject *obj = retval->second.first->clone (mobj);
      for (auto &po : retval->second.second)
        {
          paramStringProcess (&po, this);
          objectParameterSet (libraryLabel, obj, po);

        }
      return obj;
    }
}


void readerInfo::loadDefaultDefines ()
{
  namespace bg = boost::gregorian;

  std::ostringstream ss1, ss2, ss3;
  // assumes std::cout's locale has been set appropriately for the entire app
  ss1.imbue (std::locale (std::cout.getloc (), new bg::date_facet ("%Y%m%d")));
  ss1 << bg::day_clock::universal_day ();

  addDefinition ("%date", ss1.str ());


  ss2.imbue (std::locale (std::cout.getloc (), new  boost::posix_time::time_facet ("%Y%m%dT%H%M%S%F%q")));
  ss2 << boost::posix_time::second_clock::universal_time ();

  addDefinition ("%datetime", ss2.str ());


  ss3.imbue (std::locale (std::cout.getloc (), new  boost::posix_time::time_facet ("%H%M%S%F")));
  ss3 << boost::posix_time::microsec_clock::local_time ();

  addDefinition ("%time", ss3.str ());
  addDefinition ("inf", std::to_string (kBigNum));
}

void readerInfo::addDirectory (const std::string &directory)
{
  directories.push_back (directory);
  if (currentScope > 0)
    {
      directoryScope.push_back (currentScope);
    }
}


std::shared_ptr<collector> readerInfo::findCollector (const std::string &name, const std::string &fname)
{
  for (auto &col : collectors)
    {
      if ((name.empty ()) || (col->getName() == name))
        {
          if ((fname.empty ())||(col->getSinkName ().empty ())||(col->getSinkName () == fname))
            {
              return col;
            }
        }
    }
  return nullptr;
}

bool readerInfo::checkFileParam (std::string &strVal,  bool extra_find)
{
  if (strVal.back () == '_')                 //escape hatch to skip the file checking
    {
      strVal.pop_back ();
      return false;
    }
  strVal = checkDefines (strVal);
  boost::filesystem::path sourcePath (strVal);
  bool ret = false;
  if (sourcePath.has_relative_path ())
    {
      //check the most recently added directories first
      for (auto checkDirectory = directories.rbegin (); checkDirectory != directories.rend (); ++checkDirectory)
        {
          auto qpath = boost::filesystem::path (*checkDirectory);
          auto tempPath = (qpath.has_root_path ()) ? qpath / sourcePath : boost::filesystem::current_path () / qpath / sourcePath;

          if (boost::filesystem::exists (tempPath))
            {
              sourcePath = tempPath;
              ret = true;
              break;
            }
        }
      if ((ret == false) && (extra_find))
        {
          //check the most recently added directories first
          for (auto checkDirectory = directories.rbegin (); checkDirectory != directories.rend (); ++checkDirectory)
            {
              auto qpath = boost::filesystem::path (*checkDirectory);
              auto tempPath = (qpath.has_root_path ()) ? qpath / sourcePath.filename () : boost::filesystem::current_path () / qpath / sourcePath.filename ();

              if (boost::filesystem::exists (tempPath))
                {
                  sourcePath = tempPath;
                  WARNPRINT (READER_WARN_ALL, "file location path change " << strVal << " mapped to " << sourcePath.string ());
                  ret = true;
                  break;
                }
            }
        }
      if (ret == false)
        {
          if (boost::filesystem::exists (sourcePath))
            {
              ret = true;
            }
        }
      strVal = sourcePath.string ();
    }
  else
    {
      if (boost::filesystem::exists (sourcePath))
        {
          ret = true;
        }
    }

  return ret;


}

bool readerInfo::checkDirectoryParam (std::string &strVal)
{
  strVal = checkDefines (strVal);
  boost::filesystem::path sourcePath (strVal);
  bool ret = false;
  if (sourcePath.has_relative_path ())
    {
      for (auto checkDirectory = directories.rbegin (); checkDirectory != directories.rend (); ++checkDirectory)
        {
          auto qpath = boost::filesystem::path (*checkDirectory);
          auto tempPath = (qpath.has_root_path ()) ? qpath / sourcePath : boost::filesystem::current_path () / qpath / sourcePath;

          if (boost::filesystem::exists (tempPath))
            {
              sourcePath = tempPath;
              ret = true;
              break;
            }
        }

      strVal = sourcePath.string ();
    }

  return ret;
}


//a reader info thing that requires element class information
void readerInfo::addCustomElement(std::string name, const std::shared_ptr<readerElement> &element, int nargs)
{
	customElements[name] = std::make_pair(element->clone(), nargs);
	parameterIgnoreStrings.insert(name);
}


bool readerInfo::isCustomElement(const std::string &name) const
{
	auto retval = customElements.find(name);
	if (retval != customElements.end())
	{
		return true;
	}
	return false;
}

const std::pair<std::shared_ptr<readerElement>, int > readerInfo::getCustomElement(const std::string &name) const
{
	auto retval = customElements.find(name);
	return retval->second;
}

const std::unordered_set<std::string> &readerInfo::getIgnoreList() const
{
	return parameterIgnoreStrings;
}