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

#include "readerInfo.h"
#include "core/coreObject.h"
#include "fileInput.h"
#include "formatInterpreters/readerElement.h"
#include "griddyn/measurement/collector.h"
#include "readerHelper.h"

#include <boost/date_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/filesystem.hpp>

#include <cmath>
namespace griddyn
{
using namespace readerConfig;

void basicReaderInfo::setFlag (int flagID)
{
    if (flagID < 32 && flagID >= 0)
    {
        flags |= (1 << flagID);
    }
}

readerInfo::readerInfo () { loadDefaultDefines (); }
readerInfo::readerInfo (basicReaderInfo const& bri) : basicReaderInfo (bri) {}
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
                defines[std::get<1> (scopedDefinitions.back ())] = std::get<3> (scopedDefinitions.back ());
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
    // now deal with scoped directory definitions
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

void readerInfo::addDefinition (const std::string &def, const std::string &replacement)
{
    auto search = lockDefines.find (def);
    if (search == lockDefines.end ())
    {
        if (currentScope > 0)
        {
            auto prevdef = defines.find (def);
            if (prevdef != defines.end ())
            {
                scopedDefinitions.emplace_back (currentScope, def, true, prevdef->second);
            }
            else
            {
                scopedDefinitions.emplace_back (currentScope, def, false, "");
            }
        }
        defines[def] = replacement;
    }
}

void readerInfo::addTranslate (const std::string &def, const std::string &component)
{
    objectTranslations[def] = component;
    parameterIgnoreStrings.insert (def);
}

void readerInfo::addTranslate (const std::string &def, const std::string &component, const std::string &type)
{
    objectTranslations[def] = component;
    objectTranslationsType[def] = type;
    parameterIgnoreStrings.insert (def);
}

void readerInfo::addTranslateType (const std::string &def, const std::string &type)
{
    objectTranslationsType[def] = type;
}

void readerInfo::addLockedDefinition (const std::string &def, const std::string &replacement)
{
    auto search = lockDefines.find (def);
    if (search == lockDefines.end ())
    {
        defines[def] = replacement;
        lockDefines[def] = replacement;
    }
}

void readerInfo::replaceLockedDefinition (const std::string &def, const std::string &replacement)
{
    defines[def] = replacement;
    lockDefines[def] = replacement;
}

std::string readerInfo::checkDefines (const std::string &input)
{
    std::string out = input;
    int repcnt = 0;
    bool rep (true);
    while (rep)
    {
        rep = false;
        ++repcnt;
        if (repcnt > 15)
        {
            WARNPRINT (READER_WARN_IMPORTANT, "probable definition recursion in " << input << "currently " << out);
            continue;
        }
        auto search = defines.find (out);
        if (search != defines.end ())
        {
            out = search->second;
            rep = true;
            continue;  // continue outer loop
        }
        auto pos1 = out.find_first_of ('$');
        while (pos1 != std::string::npos)
        {
            auto pos2 = out.find_first_of ('$', pos1 + 1);
            if (pos2 != std::string::npos)
            {
                auto temp = out.substr (pos1 + 1, pos2 - pos1 - 1);
                search = defines.find (temp);
                if (search != defines.end ())
                {
                    // out = out.substr (0, pos1) + search->second + out.substr (pos2 + 1);
                    out.replace (pos1, pos2 - pos1 + 1, search->second);
                    rep = true;
                    break;  // break out of inner loop
                }
                // try a recursive interpretation of the string block to convert a numeric result back into a
                // string
                double val = interpretString (temp, *this);
                if (!std::isnan (val))
                {
                    if (std::abs (trunc (val) - val) < 1e-9)
                    {
                        // out = out.substr (0, pos1) + std::to_string (static_cast<int> (val)) + out.substr
                        // (pos2 + 1);
                        out.replace (pos1, pos2 - pos1 + 1, std::to_string (static_cast<int> (val)));
                    }
                    else
                    {
                        // out = out.substr (0, pos1) + std::to_string (val) + out.substr (pos2 + 1);
                        auto str = std::to_string (val);
                        while (str.back () == '0')  // remove trailing zeros
                        {
                            str.pop_back ();
                        }
                        out.replace (pos1, pos2 - pos1 + 1, str);
                    }
                    rep = true;
                    break;  // break out of inner loop
                }
                pos1 = pos2;
            }
            else
            {
                break;
            }
        }
    }
    return out;
}

std::string readerInfo::objectNameTranslate (const std::string &input)
{
    std::string out = input;
    int repcnt = 0;
    bool rep (true);
    while (rep)
    {
        ++repcnt;
        if (repcnt > 15)
        {
            WARNPRINT (READER_WARN_IMPORTANT,
                       "probable Translation recursion in " << input << "currently " << out);
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

bool readerInfo::addLibraryObject (coreObject *obj, std::vector<gridParameter> &pobjs)
{
    auto retval = library.find (obj->getName ());
    if (retval == library.end ())
    {
        library[obj->getName ()] = std::make_pair (obj, pobjs);
        obj->setName (obj->getName () + "_$");  // make sure all cloned object have a unique name
        return true;
    }
    return false;
}

coreObject *readerInfo::findLibraryObject (const std::string &objName) const
{
    auto retval = library.find (objName);
    if (retval != library.end ())
    {
        return retval->second.first;
    }
    return nullptr;
}

const std::string libraryLabel = "library";
coreObject *readerInfo::makeLibraryObject (const std::string &objName, coreObject *mobj)
{
    auto objloc = library.find (objName);
    if (objloc == library.end ())
    {
        WARNPRINT (READER_WARN_ALL, "unknown reference object " << objName);
        return mobj;
    }

    coreObject *obj = objloc->second.first->clone (mobj);
    for (auto &po : objloc->second.second)
    {
        paramStringProcess (po, *this);
        objectParameterSet (libraryLabel, obj, po);
    }
    obj->updateName ();
    return obj;
}

void readerInfo::loadDefaultDefines ()
{
    namespace bg = boost::gregorian;

    std::ostringstream ss1, ss2, ss3;
    // assumes std::cout's locale has been set appropriately for the entire app
    ss1.imbue (std::locale (std::cout.getloc (), new bg::date_facet ("%Y%m%d")));
    ss1 << bg::day_clock::universal_day ();

    addDefinition ("%date", ss1.str ());

    ss2.imbue (std::locale (std::cout.getloc (), new boost::posix_time::time_facet ("%Y%m%dT%H%M%S%F%q")));
    ss2 << boost::posix_time::second_clock::universal_time ();

    addDefinition ("%datetime", ss2.str ());

    ss3.imbue (std::locale (std::cout.getloc (), new boost::posix_time::time_facet ("%H%M%S%F")));
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

std::shared_ptr<collector> readerInfo::findCollector (const std::string &name, const std::string &fileName)
{
    for (auto &col : collectors)
    {
        if ((name.empty ()) || (col->getName () == name))
        {
            if ((fileName.empty ()) || (col->getSinkName ().empty ()) || (col->getSinkName () == fileName))
            {
                return col;
            }
        }
    }
    return nullptr;
}

bool readerInfo::checkFileParam (std::string &strVal, bool extra_find)
{
    if (strVal.back () == '_')  // escape hatch to skip the file checking
    {
        strVal.pop_back ();
        return false;
    }
    strVal = checkDefines (strVal);
    boost::filesystem::path sourcePath (strVal);
    bool ret = false;
    if (sourcePath.has_relative_path ())
    {
        // check the most recently added directories first
        for (auto checkDirectory = directories.rbegin (); checkDirectory != directories.rend (); ++checkDirectory)
        {
            auto qpath = boost::filesystem::path (*checkDirectory);
            auto tempPath = (qpath.has_root_path ()) ? qpath / sourcePath :
                                                       boost::filesystem::current_path () / qpath / sourcePath;

            if (boost::filesystem::exists (tempPath))
            {
                sourcePath = tempPath;
                ret = true;
                break;
            }
        }
        if ((!ret) && (extra_find))
        {
            // check the most recently added directories first
            for (auto checkDirectory = directories.rbegin (); checkDirectory != directories.rend ();
                 ++checkDirectory)
            {
                auto qpath = boost::filesystem::path (*checkDirectory);
                auto tempPath = (qpath.has_root_path ()) ?
                                  qpath / sourcePath.filename () :
                                  boost::filesystem::current_path () / qpath / sourcePath.filename ();

                if (boost::filesystem::exists (tempPath))
                {
                    sourcePath = tempPath;
                    WARNPRINT (READER_WARN_ALL,
                               "file location path change " << strVal << " mapped to " << sourcePath.string ());
                    ret = true;
                    break;
                }
            }
        }
        if (!ret)
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
    // if for some reason we need to capture the files
    if (captureFiles)
    {
        if (ret)
        {
            capturedFiles.push_back (strVal);
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
            auto tempPath = (qpath.has_root_path ()) ? qpath / sourcePath :
                                                       boost::filesystem::current_path () / qpath / sourcePath;

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

// a reader info thing that requires element class information
void readerInfo::addCustomElement (const std::string &name,
                                   const std::shared_ptr<readerElement> &element,
                                   int nargs)
{
    customElements[name] = std::make_pair (element->clone (), nargs);
    parameterIgnoreStrings.insert (name);
}

bool readerInfo::isCustomElement (const std::string &name) const
{
    auto retval = customElements.find (name);
    return (retval != customElements.end ());
}

const std::pair<std::shared_ptr<readerElement>, int> readerInfo::getCustomElement (const std::string &name) const
{
    auto retval = customElements.find (name);
    return retval->second;
}

const ignoreListType &readerInfo::getIgnoreList () const { return parameterIgnoreStrings; }
}  // namespace griddyn
