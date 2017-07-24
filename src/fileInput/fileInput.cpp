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

#include "fileInput.h"

#include "readElement.h"

#include "core/coreExceptions.h"
#include "formatInterpreters/jsonReaderElement.h"
#include "formatInterpreters/tinyxml2ReaderElement.h"
#include "formatInterpreters/tinyxmlReaderElement.h"
#include "formatInterpreters/yamlReaderElement.h"
#include "readElementFile.h"
#include "utilities/stringOps.h"

#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/filesystem.hpp>

#include "griddyn.h"
namespace griddyn
{
namespace readerConfig
{
int printMode = READER_DEFAULT_PRINT;
int warnMode = READER_WARN_ALL;
int warnCount = 0;

match_type defMatchType = match_type::capital_case_match;

xmlreader default_xml_reader = xmlreader::tinyxml;

void setPrintMode (int level) { printMode = level; }
#define READER_VERBOSE_PRINT 3
#define READER_NORMAL_PRINT 2
#define READER_SUMMARY_PRINT 1
#define READER_NO_PRINT 0

#define READER_WARN_ALL 2
#define READER_WARN_IMPORTANT 1
#define READER_WARN_NONE 0

void setPrintMode (const std::string &level)
{
    if ((level == "0") || (level == "none"))
    {
        printMode = READER_NO_PRINT;
    }
    else if ((level == "1") || (level == "summary"))
    {
        printMode = READER_SUMMARY_PRINT;
    }
    else if ((level == "2") || (level == "normal"))
    {
        printMode = READER_SUMMARY_PRINT;
    }
    else if ((level == "3") || (level == "verbose"))
    {
        printMode = READER_SUMMARY_PRINT;
    }
    else
    {
        WARNPRINT (READER_WARN_IMPORTANT, "invalid printMode");
    }
}

void setWarnMode (int level) { warnMode = level; }
void setWarnMode (const std::string &level)
{
    if ((level == "0") || (level == "none"))
    {
        printMode = READER_WARN_NONE;
    }
    else if ((level == "1") || (level == "important"))
    {
        printMode = READER_WARN_IMPORTANT;
    }
    else if ((level == "2") || (level == "normal"))
    {
        printMode = READER_WARN_ALL;
    }
    else
    {
        WARNPRINT (READER_WARN_IMPORTANT, "invalid waring level specification");
    }
}

void setDefaultMatchType (const std::string &matchType)
{
    if ((matchType == "exact") || (matchType == "strict"))
    {
        defMatchType = match_type::strict_case_match;
    }
    else if ((matchType == "capital") || (matchType == "caps"))
    {
        defMatchType = match_type::capital_case_match;
    }
    else if ((matchType == "any") || (matchType == "all"))
    {
        defMatchType = match_type::any_case_match;
    }
}

void setDefaultXMLReader (const std::string &xmltype)
{
    if ((xmltype == "1") || (xmltype == "tinyxml1") || (xmltype == "ticpp"))
    {
        default_xml_reader = xmlreader::tinyxml;
    }
    else if ((xmltype == "2") || (xmltype == "tinyxml2"))
    {
        default_xml_reader = xmlreader::tinyxml2;
    }
}
}//namespace readerConfig

using namespace readerConfig;

int objectParameterSet (const std::string &label, coreObject *obj, gridParameter &param) noexcept
{
    try
    {
        if (param.stringType)
        {
            LEVELPRINT (READER_VERBOSE_PRINT, label << ":setting " << obj->getName () << ' ' << param.field
                                                    << " to " << param.strVal);
            obj->set (param.field, param.strVal);
        }
        else
        {
            LEVELPRINT (READER_VERBOSE_PRINT, label << ":setting " << obj->getName () << ' ' << param.field
                                                    << " to " << param.value);
            obj->set (param.field, param.value, param.paramUnits);
        }
        return 0;
    }
    catch (const unrecognizedParameter &)
    {
        WARNPRINT (READER_WARN_ALL, "unrecognized " << label << "  parameter " << param.field);
    }
    catch (const invalidParameterValue &)
    {
        if (param.stringType)
        {
            WARNPRINT (READER_WARN_ALL, "value for parameter " << param.field << " (" << param.strVal
                                                               << ") is invalid");
        }
        else
        {
            WARNPRINT (READER_WARN_ALL, "value for parameter " << param.field << " (" << param.value
                                                               << ") is invalid");
        }
    }
    catch (...)
    {
        WARNPRINT (READER_WARN_ALL, "unknown error when setting " << param.field);
    }
    return (-1);
}

uint32_t addflags (uint32_t iflags, const std::string &flags)
{
    using namespace stringOps;
    uint32_t oflags = iflags;
    auto flagsep = splitline (flags);
    trim (flagsep);
    for (auto &flag : flagsep)
    {
        if (flag == "ignore_step_up_transformers")
        {
            oflags |= (1 << ignore_step_up_transformer);
        }
    }
    // MORE will likely be added later
    return oflags;
}

void loadFile (std::unique_ptr<gridDynSimulation> &gds,
               const std::string &fileName,
               readerInfo *ri,
               const std::string &ext)
{
    loadFile (gds.get (), fileName, ri, ext);
}

void loadFile (coreObject *parentObject, const std::string &fileName, readerInfo *ri, std::string ext)
{
    

    if (ext.empty ())
    {
		boost::filesystem::path sourcePath(fileName);
        ext = convertToLowerCase (sourcePath.extension ().string ());
        if (ext[0] == '.')
        {
            ext.erase (0, 1);
        }
    }

    std::unique_ptr<readerInfo> uri = (ri != nullptr) ? nullptr : std::make_unique<readerInfo> ();
    if (ri == nullptr)
    {
        ri = uri.get ();
    }

    // get rid of the . on the extension if it has one

    if (ext == "xml")
    {
        switch (default_xml_reader)
        {
        case xmlreader::tinyxml:
        default:
            loadElementFile<tinyxmlReaderElement> (parentObject, fileName, ri);
            break;
        case xmlreader::tinyxml2:
            loadElementFile<tinyxml2ReaderElement> (parentObject, fileName, ri);
            break;
        }
    }
    else if (ext == "csv")
    {
        loadCSV (parentObject, fileName, *ri);
    }
    else if ((ext == "raw")||(ext=="psse")||(ext=="pss/e"))
    {
        loadRAW (parentObject, fileName, *ri);
    }
    else if (ext == "dyr")
    {
        loadDYR (parentObject, fileName, *ri);
    }
    else if ((ext == "cdf") || (ext == "txt"))
    {
        loadCDF (parentObject, fileName, *ri);
    }
    else if (ext == "uct")
    {
    }
    else if ((ext == "m")||(ext=="matlab"))
    {
        loadMFile (parentObject, fileName, *ri);
    }
    else if (ext == "psp")
    {
        loadPSP (parentObject, fileName, *ri);
    }
    else if (ext == "epc")
    {
        loadEPC (parentObject, fileName, *ri);
    }
    else if (ext == "pti")
    {
        loadRAW (parentObject, fileName, *ri);
    }
    else if (ext == "json")
    {
        loadElementFile<jsonReaderElement> (parentObject, fileName, ri);
    }
#ifdef YAML_FOUND
    else if ((ext == "yaml") || (ext == "yml"))
    {
        loadElementFile<yamlReaderElement> (parentObject, fileName, ri);
    }
#endif
    else if (ext == "gdz")  // gridDyn Zipped file
    {
        loadGDZ (parentObject, fileName, *ri);
    }
}

void addToParent (coreObject *objectToAdd, coreObject *parentObject)
{
    try
    {
        parentObject->add (objectToAdd);
    }
    catch (const unrecognizedObjectException &)
    {
        WARNPRINT (READER_WARN_IMPORTANT, "Object " << objectToAdd->getName () << " not recognized by "
                                                    << parentObject->getName ());
    }
    catch (const objectAddFailure &)
    {
        WARNPRINT (READER_WARN_IMPORTANT, "Failure to add " << objectToAdd->getName () << " to "
                                                            << parentObject->getName ());
    }
}

// if multiple object with the same name may have been added (parallel transmission lines, generators, etc)
// sequence through the count to find one that hasn't been used then rename the object and add it.
void addToParentRename (coreObject *objectToAdd, coreObject *parentObject)
{
    std::string bname = objectToAdd->getName ();
    int cnt = 2;
    auto fndObject = parentObject->find (bname + '-' + std::to_string (cnt));
    while (fndObject != nullptr)
    {
        ++cnt;
        fndObject = parentObject->find (bname + '-' + std::to_string (cnt));
    }
    objectToAdd->setName (bname + '-' + std::to_string (cnt));
    addToParent (objectToAdd, parentObject);
}

}//namespace griddyn