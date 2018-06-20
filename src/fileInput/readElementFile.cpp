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

#include "readElement.h"

#include "core/helperObject.h"
#include "elementReaderTemplates.hpp"
#include "griddyn/gridDynSimulation.h"
#include "readerHelper.h"
#include <sstream>
#include <boost/filesystem.hpp>
#include "utilities/gridRandom.h"
#include "utilities/stringConversion.h"

namespace griddyn
{
using namespace readerConfig;

void loadElementInformation (coreObject *obj,
                             std::shared_ptr<readerElement> &element,
                             const std::string &objectName,
                             readerInfo &ri,
                             const IgnoreListType &ignoreList)
{
    objSetAttributes (obj, element, objectName, ri, ignoreList);
    readImports (element, ri, obj, false);
    // check for child objects
    loadSubObjects (element, ri, obj);

    // get all element fields
    paramLoopElement (obj, element, objectName, ri, ignoreList);
    readImports (element, ri, obj, true);
}

void checkForEndUnits(gridParameter &param, const std::string &paramStr);

static const std::string importString ("import");
void readImports (std::shared_ptr<readerElement> &element,
                  readerInfo &ri,
                  coreObject *parentObject,
                  bool finalFlag)
{
    if (!element->hasElement (importString))
    {
        return;
    }

    // run any source files
    auto bflags = ri.getFlags();
    element->bookmark ();
    element->moveToFirstChild (importString);
    while (element->isValid ())
    {
        bool finalMode = false;
        std::string fstring = getElementField (element, "final", defMatchType);

        if ((fstring == "true") || (fstring == "1"))
        {
            finalMode = true;
        }

        if (finalFlag != finalMode)
        {
            element->moveToNextSibling (importString);
            continue;
        }

        std::string flags = getElementField (element, "flags", defMatchType);
        if (!flags.empty ())
        {
            addflags (ri, flags);
        }
        std::string sourceFile = getElementField (element, "file", defMatchType);
        if (sourceFile.empty ())
        {
            // if we don't find a field named file, just use the text in the source element
            sourceFile = element->getText ();
        }

        // check through the files to find the right location
        ri.checkFileParam (sourceFile, true);

        boost::filesystem::path sourcePath (sourceFile);
        std::string prefix = getElementField (element, "prefix", match_type::capital_case_match);
        // get the prefix if any
        if (prefix.empty ())
        {
            prefix = ri.prefix;
        }
        else if (!(ri.prefix.empty ()))
        {
            prefix = ri.prefix + '_' + prefix;
        }

        // check for type override
        std::string ext = convertToLowerCase (getElementField (element, "filetype", defMatchType));

        std::swap (prefix, ri.prefix);
        if (ext.empty ())
        {
            loadFile (parentObject, sourceFile, &ri);
        }
        else
        {
            loadFile (parentObject, sourceFile, &ri, ext);
        }
        std::swap (prefix, ri.prefix);

        ri.setAllFlags(bflags);
        element->moveToNextSibling (importString);  // next import file
    }
    element->restore ();
}

static const std::string unitString1 ("units");
static const std::string unitString2 ("unit");

gridUnits::units_t readUnits (const std::shared_ptr<readerElement> &element, const std::string &field)
{
    std::string uname = element->getAttributeText (unitString1);
    // actually specifying a "unit" attribute takes precedence
    if (uname.empty ())
    {
        uname = element->getAttributeText (unitString2);
    }
    if (!uname.empty ())
    {
        auto retUnits = gridUnits::getUnits (uname);
        if (retUnits == gridUnits::defUnit)
        {
            WARNPRINT (READER_WARN_ALL, "unknown unit " << uname);
        }
        return retUnits;
    }
    if (field.back () == ')')
    {
        auto p = field.find_last_of ('(');

        if (p != std::string::npos)
        {
            uname = field.substr (p + 1, field.length () - 2 - p);
            auto retUnits = gridUnits::getUnits (uname);
            if (retUnits == gridUnits::defUnit)
            {
                WARNPRINT (READER_WARN_ALL, "unknown unit " << uname);
            }
            return retUnits;
        }
    }
    return gridUnits::defUnit;
}

static const std::string valueString ("value");

gridParameter getElementParam(const std::shared_ptr<readerElement> &element)
{
	gridParameter P;
	getElementParam(element, P);
	return P;
}

void getElementParam (const std::shared_ptr<readerElement> &element, gridParameter &param)
{

		param.paramUnits = gridUnits::defUnit;
		param.valid = false;
    std::string fieldName = convertToLowerCase (element->getName ());

    if (fieldName == "param")
    {
        std::string pname = element->getAttributeText ("name");
        if (pname.empty ())
        {
            pname = element->getAttributeText ("field");
        }
        if (pname.empty ())
        {
            // no name or field attribute so just read the string and see if we can process it
			param.fromString (element->getText ());
			return;
        }
		param.paramUnits = readUnits (element, pname);
        if (pname.back () == ')')
        {
            auto p = pname.find_last_of ('(');
            if (p != std::string::npos)
            {
                pname.erase (p);
            }
        }
		param.field = convertToLowerCase (pname);
        if (element->hasAttribute (valueString))
        {
			param.value = element->getAttributeValue (valueString);
            if (param.value == readerNullVal)
            {
				checkForEndUnits(param, element->getAttributeText(valueString));
            }
            else
            {
				param.stringType = false;
            }
        }
        else
        {
			param.value = element->getValue ();
            if (param.value == readerNullVal)
            {
				checkForEndUnits(param, element->getText());
            }
            else
            {
				param.stringType = false;
            }
        }
    }
    // all other properties
    else
    {
		param.paramUnits = readUnits (element, fieldName);
        if (fieldName.back () == ')')
        {
            auto p = fieldName.find_last_of ('(');
            if (p != std::string::npos)
            {
                fieldName.erase (p);
            }
        }
		param.field = fieldName;
		param.value = element->getValue ();
        if (param.value == readerNullVal)
        {
			checkForEndUnits(param, element->getText());
        }
        else
        {
			param.stringType = false;
        }
    }
	param.valid = true;
}

void checkForEndUnits(gridParameter &param, const std::string &paramStr)
{
	double val = numeric_conversion(paramStr, readerNullVal);
	if (val != readerNullVal)
	{
		auto N = paramStr.find_last_of("012345689. )]");
		auto Unit = gridUnits::getUnits(paramStr.substr(N + 1));
		if (Unit != gridUnits::defUnit)
		{
			param.value = val;
			param.paramUnits = Unit;
			param.stringType = false;
			return;
		}

	}
	param.strVal = paramStr;
	param.stringType = true;
}

static const IgnoreListType keywords{"type",      "ref",      "number",        "index",   "retype",
                                     "name",      "define",   "library",       "import",  "area",
                                     "bus",       "link",     "load",          "exciter", "if",
                                     "source",    "governor", "block",         "pss",     "simulation",
                                     "generator", "array",    "relay",         "parent",  "genmodel",
                                     "line",      "solver",   "agc",           "reserve", "reservedispatch",
                                     "dispatch",  "econ",     "configuration", "custom",  "purpose",
                                     "event",     "collector"};

void objSetAttributes (coreObject *obj,
                       std::shared_ptr<readerElement> &element,
                       const std::string &component,
                       readerInfo &ri,
                       const IgnoreListType &ignoreList)
{
    auto att = element->getFirstAttribute ();
    while (att.isValid ())
    {
        gridUnits::units_t unitType = gridUnits::defUnit;
        std::string fieldName = convertToLowerCase (att.getName ());

        if (fieldName.back () == ')')
        {
            auto p = fieldName.find_last_of ('(');
            if (p != std::string::npos)
            {
                std::string ustring = fieldName.substr (p + 1, fieldName.length () - 2 - p);
                unitType = gridUnits::getUnits (ustring);
				fieldName = fieldName.substr (0, p - 1);
            }
        }
        auto ifind = keywords.find (fieldName);
        if (ifind != keywords.end ())
        {
            att = element->getNextAttribute ();
            continue;
        }
        ifind = ignoreList.find (fieldName);
        if (ifind != ignoreList.end ())
        {
            att = element->getNextAttribute ();
            continue;
        }

        if ((fieldName.find ("file") != std::string::npos)||(fieldName=="fmu"))
        {
            std::string strVal = att.getText ();
            ri.checkFileParam (strVal);
            gridParameter po (fieldName, strVal);
            objectParameterSet (component, obj, po);
        }
        else if ((fieldName.find ("workdir") != std::string::npos) || (fieldName.find ("directory") != std::string::npos))
        {
            std::string strVal = att.getText ();
            ri.checkDirectoryParam (strVal);
            gridParameter po (fieldName, strVal);
            objectParameterSet (component, obj, po);
        }
        else if ((fieldName == "flag") || (fieldName == "flags"))  // read the flags parameter
        {
                try
                {
					setMultipleFlags(obj, att.getText());
                }
                catch (const unrecognizedParameter &)
                {
                    WARNPRINT (READER_WARN_ALL, "unrecognized flag " << att.getText() << "\n");
                }
        }
        else
        {
            double val = att.getValue ();
            if (val != readerNullVal)
            {
                gridParameter po (fieldName, val);
                po.paramUnits = unitType;
                objectParameterSet (component, obj, po);
            }
            else
            {
                gridParameter po (fieldName, att.getText ());
                paramStringProcess (po, ri);
                objectParameterSet (component, obj, po);
            }
        }
        att = element->getNextAttribute ();
    }
}

void paramLoopElement (coreObject *obj,
                       std::shared_ptr<readerElement> &element,
                       const std::string &component,
                       readerInfo &ri,
                       const IgnoreListType &ignoreList)
{

    element->moveToFirstChild ();
    while (element->isValid ())
    {
        std::string fieldName = convertToLowerCase (element->getName ());
        auto ifind = keywords.find (fieldName);
        if (ifind != keywords.end ())
        {
            element->moveToNextSibling ();
            continue;
        }
        ifind = ri.getIgnoreList ().find (fieldName);
        if (ifind != ri.getIgnoreList ().end ())
        {
            element->moveToNextSibling ();
            continue;
        }
        ifind = ignoreList.find (fieldName);
        if (ifind != ignoreList.end ())
        {
            element->moveToNextSibling ();
            continue;
        }
        // get all the parameter fields
        auto param=getElementParam (element);
        if (param.valid)
        {
            if (param.stringType)
            {
                if ((param.field.find ("file") != std::string::npos)||(param.field=="fmu"))
                {
                    ri.checkFileParam (param.strVal);
                    objectParameterSet (component, obj, param);
                }
                else if ((param.field.find ("workdir") != std::string::npos) ||
                         (param.field.find ("directory") != std::string::npos))
                {
                    ri.checkDirectoryParam (param.strVal);
                    objectParameterSet (component, obj, param);
                }
                else if ((fieldName == "flag") || (fieldName == "flags"))  // read the flags parameter
                {
                    paramStringProcess (param, ri);
                    try
                    {
                        setMultipleFlags(obj, param.strVal);
                    }
                    catch (const unrecognizedParameter &)
                    {
                        WARNPRINT(READER_WARN_ALL, "unrecognized flag in " <<param.strVal << "\n");
                    }
                }
                else
                {
                    paramStringProcess (param, ri);
                    objectParameterSet (component, obj, param);
                }
            }
            else
            {
                objectParameterSet (component, obj, param);
            }
        }
        element->moveToNextSibling ();
    }
    element->moveToParent ();
}

void readConfigurationFields (std::shared_ptr<readerElement> &sim, readerInfo & /*ri*/)
{
    if (sim->hasElement ("configuration"))
    {
        sim->bookmark ();
        sim->moveToFirstChild ("configuration");
        auto cfgAtt = sim->getFirstAttribute ();

        while (cfgAtt.isValid ())
        {
            auto cfgname = convertToLowerCase (cfgAtt.getName ());
            if ((cfgname == "matching") || (cfgname == "match_type"))
            {
                readerConfig::setDefaultMatchType (cfgAtt.getText ());
            }
            else if (cfgname == "printlevel")
            {
                readerConfig::setPrintMode (cfgAtt.getText ());
            }
            else if ((cfgname == "xmlreader") || (cfgname == "xml"))
            {
                readerConfig::setDefaultXMLReader (cfgAtt.getText ());
            }
            else if ((cfgname == "seed"))
            {
                try
                {
                    auto seed = std::stoll(cfgAtt.getText());
                    utilities::gridRandom::setSeed(seed);
                }
                catch (const std::invalid_argument &)
                {
                    WARNPRINT(READER_WARN_IMPORTANT, "invalid seed value, must be an integer");
                }

            }
            cfgAtt = sim->getNextAttribute ();
        }

        sim->moveToFirstChild ();
        while (sim->isValid ())
        {
            auto fieldName = convertToLowerCase (sim->getName ());
            if ((fieldName == "matching") || (fieldName == "match_type"))
            {
                readerConfig::setDefaultMatchType (sim->getText ());
            }
            else if (fieldName == "printlevel")
            {
                readerConfig::setPrintMode (sim->getText ());
            }
            else if ((fieldName == "xmlreader") || (fieldName == "xml"))
            {
                readerConfig::setDefaultXMLReader (sim->getText ());
            }
			else if ((fieldName == "seed"))
			{
				try
				{
					auto seed = std::stoll(cfgAtt.getText());
					utilities::gridRandom::setSeed(seed);
				}
				catch (const std::invalid_argument &)
				{
					WARNPRINT(READER_WARN_IMPORTANT, "invalid seed value, must be an integer");
				}

			}
            sim->moveToNextSibling ();
        }

        sim->restore ();
    }
}

void setAttributes (helperObject *obj,
                    std::shared_ptr<readerElement> &element,
                    const std::string &component,
                    readerInfo &ri,
                    const IgnoreListType &ignoreList)
{
    using namespace readerConfig;
    auto att = element->getFirstAttribute ();

    while (att.isValid ())
    {
        std::string fieldName = convertToLowerCase (att.getName ());

        auto ifind = ignoreList.find (fieldName);
        if (ifind != ignoreList.end ())
        {
            att = element->getNextAttribute ();
            continue;
        }
        try
        {
            if ((fieldName.find ("file") != std::string::npos)||(fieldName=="fmu"))
            {
                std::string strVal = att.getText ();
                ri.checkFileParam (strVal);
                LEVELPRINT (READER_VERBOSE_PRINT, component << ": setting " << fieldName << " to " << strVal);
                obj->set (fieldName, strVal);
            }
            else
            {
                double val = att.getValue ();
                if ((val != readerNullVal)&&(val!=kNullVal))
                {
                    LEVELPRINT (READER_VERBOSE_PRINT, component << ": setting " << fieldName << " to " << val);
                    obj->set (fieldName, val);
                }
                else
                {
                    gridParameter po (fieldName, att.getText ());
                    paramStringProcess (po, ri);
                    if (po.stringType)
                    {
                        obj->set (po.field, po.strVal);
                        LEVELPRINT (READER_VERBOSE_PRINT,
                                    component << ": setting " << fieldName << " to " << po.strVal);
                    }
                    else
                    {
                        obj->set (po.field, po.value);
                        LEVELPRINT (READER_VERBOSE_PRINT,
                                    component << ": setting " << fieldName << " to " << po.value);
                    }
                }
            }
        }
        catch (const unrecognizedParameter &)
        {
            WARNPRINT (READER_WARN_ALL, "unknown " << component << " parameter " << fieldName);
        }
        catch (const invalidParameterValue &)
        {
            WARNPRINT (READER_WARN_ALL, "value for " << component << " parameter " << fieldName << " ("
                                                     << att.getText () << ") is invalid");
        }

        att = element->getNextAttribute ();
    }
}

void setParams (helperObject *obj,
                std::shared_ptr<readerElement> &element,
                const std::string &component,
                readerInfo &ri,
                const IgnoreListType &ignoreList)
{
    using namespace readerConfig;

    element->moveToFirstChild ();
    while (element->isValid ())
    {
        std::string fieldName = convertToLowerCase (element->getName ());
        auto ifind = ignoreList.find (fieldName);
        if (ifind != ignoreList.end ())
        {
            element->moveToNextSibling ();
            continue;
        }

        auto param=getElementParam (element);
        if (param.valid)
        {
            try
            {
                if (param.stringType)
                {
                    if ((param.field.find ("file") != std::string::npos)||(param.field=="fmu"))
                    {
                        ri.checkFileParam (param.strVal);
                        LEVELPRINT (READER_VERBOSE_PRINT,
                                    component << ":setting " << obj->getName () << " file to " << param.strVal);
                        obj->set (param.field, param.strVal);
                    }
                    else
                    {
                        paramStringProcess (param, ri);
                        if (param.stringType)
                        {
                            LEVELPRINT (READER_VERBOSE_PRINT, component << ":setting " << obj->getName () << " "
                                                                        << param.field << " to " << param.strVal);
                            obj->set (param.field, param.strVal);
                        }
                        else
                        {
                            LEVELPRINT (READER_VERBOSE_PRINT, component << ":setting " << obj->getName () << " "
                                                                        << param.field << " to " << param.value);
                            obj->set (param.field, param.value);
                        }
                    }
                }
                else
                {
                    LEVELPRINT (READER_VERBOSE_PRINT, component << ":setting " << obj->getName () << " "
                                                                << param.field << " to " << param.value);
                    obj->set (param.field, param.value);
                }
            }
            catch (const unrecognizedParameter &)
            {
                WARNPRINT (READER_WARN_ALL, "unknown " << component << " parameter " << param.field);
            }
            catch (const invalidParameterValue &)
            {
                if (param.stringType)
                {
                    WARNPRINT (READER_WARN_ALL, "value for " << component << " parameter " << param.field << " ("
                                                             << param.strVal << ") is invalid");
                }
                else
                {
                    WARNPRINT (READER_WARN_ALL, "value for " << component << " parameter " << param.field << " ("
                                                             << param.value << ") is invalid");
                }
            }
        }
        element->moveToNextSibling ();
    }
    element->moveToParent ();
}

}//namespace griddyn
