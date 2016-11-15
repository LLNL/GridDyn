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

#include "collector.h"
#include "timeSeries.h"
#include "stringOps.h"
#include "objectInterpreter.h"
#include "core/helperTemplates.h"
#include "core/gridDynExceptions.h"
#include <boost/filesystem.hpp>


gridRecorder::gridRecorder (double time0, double period) :  collector (time0,period)
{

}

gridRecorder::gridRecorder(const std::string &objName):collector(objName)
{
	
}

gridRecorder::~gridRecorder ()
{
  //check to make sure there is no unrecorded data
  if (!filename.empty ())
    {
      saveFile ();
    }
}


std::shared_ptr<collector> gridRecorder::clone (std::shared_ptr<collector> gr) const
{
	auto nrec = cloneBase<gridRecorder, collector>(this, gr);
	if (!nrec)
	{
		return gr;
	}


  nrec->filename = filename;
  nrec->directory = directory;
  nrec->binaryFile = binaryFile;
  
  return nrec;
}


void gridRecorder::set (const std::string &param, double val)
{

  if (param == "precision")
    {
      precision = static_cast<int> (val);
    }
  else if ((param == "reserve")||(param == "reservecount"))
    {
      dataset.reserve (static_cast<fsize_t> (val));
    }
  else if (param == "autosave")
    {
      autosave = static_cast<count_t> (val);
    }
  else
    {
      collector::set(param,val);
    }

}

void gridRecorder::set (const std::string &param, const std::string &val)
{
 
  if ((param == "file") || (param == "filename"))
    {
      filename = val;
      boost::filesystem::path filePath (filename);
      std::string ext =convertToLowerCase(filePath.extension ().string ());
      if ((ext == ".csv") || (ext == ".txt"))
        {
          binaryFile = false;
        }
      else
        {
          binaryFile = true;
        }
    }
  else if (param == "directory")
    {
      directory = val;
    }
  else
    {
      collector::set(param,val);
    }

}

void gridRecorder::saveFile (const std::string &fname)
{
  

  bool bFile = binaryFile;
  boost::filesystem::path savefileName (filename);
  if (!fname.empty ())
    {
      savefileName = boost::filesystem::path (fname);

      std::string ext = convertToLowerCase(savefileName.extension ().string ());
      if ((ext == ".csv") || (ext == ".txt"))
        {
          bFile = false;
        }
      else
        {
          bFile = true;
        }
    }
  else
    {
      if (triggerTime == lastSaveTime)
        {
		  return;  //no work todo
        }

    }
  int ret;
  if (!savefileName.empty ())
    {
      if (!directory.empty ())
        {
          if (!savefileName.has_root_directory ())
            {
              savefileName = boost::filesystem::path (directory) / savefileName;
            }
        }
      //check to make sure the directories exist if not create them
      auto tmp = savefileName.parent_path ();
      if (!tmp.empty ())
        {
          auto res = boost::filesystem::exists (tmp);
          if (!res)
            {
              boost::filesystem::create_directories (tmp);
            }
        }
      //recheck the columns if necessary
      if (recheck)
        {
          recheckColumns ();
        }
      dataset.description = name + ": " + description;
      bool append = (lastSaveTime > -kHalfBigNum);

      //create the file based on extension
      if (bFile)
        {
          dataset.writeBinaryFile (savefileName.string (),append);
        }
      else
        {
          dataset.writeTextFile (savefileName.string (),precision,append);
        }
      lastSaveTime = triggerTime;
    }
  else
    {
	  throw(invalidFileName());
    }

}

void gridRecorder::reset ()
{
  dataset.clear ();
}

void gridRecorder::setSpace (double span)
{
  count_t pts = static_cast<count_t> (span / timePeriod);
  dataset.reserve (pts + 1);
}

void gridRecorder::addSpace (double span)
{
  count_t pts = static_cast<count_t> (span / timePeriod);
  dataset.reserve (static_cast<fsize_t> (dataset.time.capacity ()) + pts + 1);
}

void gridRecorder::fillDatasetFields()
{
	dataset.fields = collector::getColumnDescriptions();
}

change_code gridRecorder::trigger (double time)
{
	collector::trigger(time);
	if (firstTrigger)
	{
		dataset.setCols(static_cast<fsize_t>(data.size()));
		fillDatasetFields();
		firstTrigger = false;
	}
	dataset.addData(time, data);
  if ((autosave > 0)&&(dataset.count >= autosave))
    {
      saveFile ();
      dataset.clear ();
    }
  return change_code::no_change;
}


void gridRecorder::flush()
{
	saveFile();
}

const std::string &gridRecorder::getSinkName() const
{
	return getFileName();
}