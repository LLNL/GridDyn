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

#include "Recorder.h"

#include "core/coreExceptions.h"
#include "core/helperTemplates.hpp"
#include "core/objectInterpreter.h"
#include "utilities/stringOps.h"
#include <boost/filesystem.hpp>

namespace griddyn
{
Recorder::Recorder (coreTime time0, coreTime period) : collector (time0, period) {}
Recorder::Recorder (const std::string &name) : collector (name) {}
Recorder::~Recorder ()
{
    // check to make sure there is no unrecorded data
    if (!fileName_.empty ())
    {
        try
        {
            saveFile ();
        }
        catch (const std::exception &e)  // no exceptions in a destructor
        {
            collector::getObject ()->log (collector::getObject (), print_level::error, e.what ());
        }
    }
}

std::shared_ptr<collector> Recorder::clone (std::shared_ptr<collector> gr) const
{
    auto nrec = cloneBase<Recorder, collector> (this, gr);
    if (!nrec)
    {
        return gr;
    }

    nrec->fileName_ = fileName_;
    nrec->directory_ = directory_;
    nrec->binaryFile = binaryFile;

    return nrec;
}

void Recorder::set (const std::string &param, double val)
{
    if (param == "precision")
    {
        precision = static_cast<int> (val);
    }
    else if ((param == "reserve") || (param == "reservecount"))
    {
        dataset.reserve (static_cast<fsize_t> (val));
    }
    else if (param == "autosave")
    {
        autosave = static_cast<count_t> (val);
    }
    else
    {
        collector::set (param, val);
    }
}

void Recorder::set (const std::string &param, const std::string &val)
{
    if ((param == "file") || (param == "fileName"))
    {
        fileName_ = val;
        boost::filesystem::path filePath (fileName_);
        std::string ext = convertToLowerCase (filePath.extension ().string ());
		binaryFile = ((ext != ".csv") && (ext != ".txt"));
    }
    else if (param == "directory")
    {
        directory_ = val;
    }
    else
    {
        collector::set (param, val);
    }
}

void Recorder::saveFile (const std::string &fileName)
{
    bool bFile = binaryFile;
    boost::filesystem::path savefileName (fileName_);
    if (!fileName.empty ())
    {
        savefileName = boost::filesystem::path (fileName);

        std::string ext = convertToLowerCase (savefileName.extension ().string ());
        bFile = ((ext != ".csv") && (ext != ".txt"));
    }
    else
    {
        if (triggerTime == lastSaveTime)
        {
            return;  // no work todo
        }
    }
    if (!savefileName.empty ())
    {
        if (!directory_.empty ())
        {
            if (!savefileName.has_root_directory ())
            {
                savefileName = boost::filesystem::path (directory_) / savefileName;
            }
        }
        // check to make sure the directories exist if not create them
        auto tmp = savefileName.parent_path ();
        if (!tmp.empty ())
        {
            if (!boost::filesystem::exists (tmp))
            {
                boost::filesystem::create_directories (tmp);
            }
        }
        // recheck the columns if necessary
        if (recheck)
        {
            recheckColumns ();
        }
        dataset.description = getName () + ": " + getDescription ();
        bool append = (lastSaveTime > negTime);

        // create the file based on extension
        if (bFile)
        {
            dataset.writeBinaryFile (savefileName.string (), append);
        }
        else
        {
            dataset.writeTextFile (savefileName.string (), precision, append);
        }
        lastSaveTime = triggerTime;
    }
    else
    {
        throw (invalidFileName ());
    }
}

void Recorder::reset () { dataset.clear (); }
void Recorder::setSpace (double span)
{
    auto pts = static_cast<count_t> (span / timePeriod);
    dataset.reserve (pts + 1);
}

void Recorder::addSpace (double span)
{
    auto pts = static_cast<count_t> (span / timePeriod);
    dataset.reserve (static_cast<fsize_t> (dataset.time ().capacity ()) + pts + 1);
}

void Recorder::fillDatasetFields () { dataset.setFields (collector::getColumnDescriptions ()); }
change_code Recorder::trigger (coreTime time)
{
    collector::trigger (time);
    if (firstTrigger)
    {
        dataset.setCols (static_cast<fsize_t> (data.size ()));
        fillDatasetFields ();
        firstTrigger = false;
    }
    dataset.addData (time, data);
    if ((autosave > 0) && (static_cast<count_t> (dataset.size ()) >= autosave))
    {
        saveFile ();
        dataset.clear ();
    }
    return change_code::no_change;
}

void Recorder::flush () { saveFile (); }
const std::string &Recorder::getSinkName () const { return getFileName (); }
}  // namespace griddyn