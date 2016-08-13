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

#include "gridRecorder.h"
#include "generators/gridDynGenerator.h"
#include "loadModels/gridLoad.h"
#include "linkModels/gridLink.h"
#include "gridBus.h"
#include "gridArea.h"
#include "gridDyn.h"
#include "fileReaders.h"
#include "gridEvent.h"
#include "stringOps.h"
#include <cmath>

#include <boost/filesystem.hpp>


gridRecorder::gridRecorder (double time0, double period) :  timePeriod (period), reqPeriod (period), triggerTime (time0)
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


std::shared_ptr<gridRecorder> gridRecorder::clone (gridCoreObject *nobj) const
{
  std::shared_ptr<gridRecorder> nrec = std::make_shared<gridRecorder> (triggerTime, timePeriod);
  nrec->reqPeriod = reqPeriod;
  nrec->timePeriod = timePeriod;
  nrec->name =  name;
  nrec->description = description;
  nrec->filename = filename;
  nrec->directory = directory;
  nrec->binaryFile = binaryFile;
  nrec->startTime = startTime;
  nrec->stopTime = stopTime;
  std::shared_ptr<gridGrabber> ggn;
  int cnt = 0;
  for (auto gg : dataGrabbers)
    {
      ggn = gg->clone (nobj);
      if (ggn)
        {
          nrec->add (ggn,dataColumns[cnt]);
        }
      ++cnt;
    }
  return nrec;
}

std::shared_ptr<gridRecorder> gridRecorder::cloneTo (gridCoreObject *src, gridCoreObject *dest) const
{
  std::shared_ptr<gridRecorder> nrec = std::make_shared<gridRecorder> (triggerTime, timePeriod);
  nrec->reqPeriod = reqPeriod;
  nrec->timePeriod = timePeriod;
  nrec->name =  name;
  nrec->description = description;
  nrec->filename = filename;
  nrec->directory = directory;
  nrec->binaryFile = binaryFile;
  nrec->startTime = startTime;
  nrec->stopTime = stopTime;
  std::shared_ptr<gridGrabber> ggn;
  int cnt = 0;
  for (auto gg : dataGrabbers)
    {
      gridCoreObject *nobj = findMatchingObject (gg->getObject (), dynamic_cast<gridPrimary *> (src), dynamic_cast<gridPrimary *> (dest));
      ggn = gg->clone (nobj);
      if (ggn)
        {
          nrec->add (ggn, dataColumns[cnt]);
        }
      ++cnt;
    }
  return nrec;
}


int gridRecorder::set (const std::string &param, double val)
{
  int out = PARAMETER_FOUND;
  if (param == "precision")
    {
      precision = static_cast<int> (val);
    }
  else if (param == "period")
    {
      reqPeriod = val;
      timePeriod = val;
    }
  else if ((param == "reserve")||(param == "reservecount"))
    {
      dataset.reserve (static_cast<fsize_t> (val));
    }
  else if (param == "frequency")
    {
      reqPeriod = 1.0 / val;
      timePeriod = val;
    }
  else if (param == "triggertime")
    {
      triggerTime = val;
    }
  else if (param == "starttime")
    {
      startTime = val;
      triggerTime = startTime;
    }
  else if (param == "stoptime")
    {
      stopTime = val;
    }
  else if (param == "autosave")
    {
      autosave = static_cast<count_t> (val);
    }
  else if (param == "period_resolution")
    {
      if (val > 0)
        {
          auto per = static_cast<int> (std::round (reqPeriod / val));
          timePeriod = (per == 0) ? val : val * per;
        }
    }
  else
    {
      out = PARAMETER_NOT_FOUND;
    }
  return out;
}

int gridRecorder::set (const std::string &param, const std::string &val)
{
  int out = PARAMETER_FOUND;
  if ((param == "file") || (param == "filename"))
    {
      filename = val;
      boost::filesystem::path filePath (filename);
      std::string ext = filePath.extension ().string ();
      makeLowerCase (ext);
      if ((ext == ".csv") || (ext == ".txt"))
        {
          binaryFile = false;
        }
      else
        {
          binaryFile = true;
        }
    }
  else if (param == "name")
    {
      name = val;
    }
  else if (param == "directory")
    {
      directory = val;
    }
  else if (param == "description")
    {
      description = val;
    }
  else
    {
      out = PARAMETER_NOT_FOUND;
    }
  return out;
}

int gridRecorder::saveFile (const std::string &fname)
{
  int ret = FUNCTION_EXECUTION_SUCCESS;

  bool bFile = binaryFile;
  boost::filesystem::path savefileName (filename);
  if (!fname.empty ())
    {
      savefileName = boost::filesystem::path (fname);

      std::string ext = savefileName.extension ().string ();
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
          return (-2); //TODO:: file error code
        }

    }
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
          ret = dataset.writeBinaryFile (savefileName.string (),append);
        }
      else
        {
          ret = dataset.writeTextFile (savefileName.string (),precision,append);
        }
      lastSaveTime = triggerTime;
    }
  else
    {
      ret = FUNCTION_EXECUTION_FAILURE;
    }
  return ret;
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

void gridRecorder::setTime (double time)
{
  if (time > triggerTime)
    {
      triggerTime = time;
    }
}

void gridRecorder::recheckColumns ()
{
  fsize_t ct = 0;
  size_t kk;
  std::vector<double> vals;
  for (kk = 0; kk < dataGrabbers.size (); ++kk)
    {
      if (dataColumns[kk] == -1)
        {
          dataColumns[kk] = ct;
        }
      if (dataGrabbers[kk]->vectorGrab)
        {
          dataGrabbers[kk]->grabData (vals);
          ct += static_cast<fsize_t> (vals.size ());
        }
      else
        {
          ct++;
        }

    }
  dataset.setCols (ct);
  stringVec dstring;
  for (kk = 0; kk < dataGrabbers.size (); ++kk)
    {
      if (dataGrabbers[kk]->vectorGrab)
        {
          dataGrabbers[kk]->getDesc (dstring);
          for (size_t nn = 0; nn < dstring.size (); ++nn)
            {
              dataset.fields[nn + dataColumns[kk]] = dstring[nn];
            }

        }
      else
        {
          dataset.fields[dataColumns[kk]] = dataGrabbers[kk]->desc;
        }

    }
  recheck = false;
}

change_code gridRecorder::trigger (double time)
{
  double val;
  size_t kk;
  std::vector<double> vals;

  if (recheck)
    {
      recheckColumns ();
    }
  if (time >= triggerTime)
    {
      for (kk = 0; kk < dataGrabbers.size (); ++kk)
        {
          if (dataGrabbers[kk]->vectorGrab)
            {
              dataGrabbers[kk]->grabData (vals);
              dataset.addData (time, vals, dataColumns[kk]);
            }
          else
            {
              val = dataGrabbers[kk]->grabData ();
              dataset.addData (time, val, dataColumns[kk]);
            }

        }
      triggerTime += timePeriod;
      if (triggerTime < time)
        {
          triggerTime = time + (timePeriod - std::fmod (time - triggerTime, timePeriod));
        }
      if (triggerTime > stopTime)
        {
          triggerTime = kBigNum;
        }

    }
  if ((autosave > 0)&&(dataset.count >= autosave))
    {
      saveFile ();
      dataset.clear ();
    }
  return change_code::no_change;
}

int gridRecorder::add (std::shared_ptr<gridGrabber> ggb,int column)
{

  dataGrabbers.push_back (ggb);
  if (column < 0)
    {
      if (recheck)
        {
          column = -1;
        }
      else
        {
          column = static_cast<int> (columns);
        }

    }
  if (column >= static_cast<int> (columns))
    {
      columns = column + 1;
      dataset.setCols (static_cast<fsize_t> (columns));

    }
  if (column >= 0)
    {
      dataset.fields[column] = ggb->desc;
    }

  dataColumns.push_back (column);
  if (ggb->vectorGrab)
    {
      recheck = true;
    }
  if (ggb->loaded)
    {
      return OBJECT_ADD_SUCCESS;
    }
  else
    {
      return OBJECT_ADD_FAILURE;
    }
}

int gridRecorder::add (gridGrabberInfo *gdRI,gridCoreObject *obj)
{
  if (gdRI->field.empty ())              //any field specification overrides the offset
    {
      if (gdRI->offset > 0)
        {
          auto ggb = createGrabber (gdRI->offset, obj);
          if (ggb)
            {
              ggb->bias = gdRI->bias;
              ggb->gain = gdRI->gain;
              return add (ggb, gdRI->column);
            }
          else
            {
              return OBJECT_ADD_FAILURE;
            }
        }
      else
        {
          obj->log (obj, GD_WARNING_PRINT, "unable to create recorder no field or offset specified");
        }
    }
  else
    {
      if (gdRI->field.find_first_of (",;") != std::string::npos)
        {                 //now go into a loop of the comma variables
          //if multiple fields were specified by comma seperation
          stringVec v = splitlineBracket (gdRI->field,",;");
          int ccol = gdRI->column;
          for (auto fld : v)
            {
              gdRI->field = fld;
              if (ccol >= 0)
                {
                  gdRI->column = ccol++;                               //post increment intended
                }
              auto ret = add (gdRI, obj);
              if (ret == OBJECT_ADD_FAILURE)
                {
                  obj->log (obj, GD_WARNING_PRINT, "unable to add recorder from " + fld);
                }
            }
        }
      else                       //now we get to the interesting bit
        {
          auto fldGrabbers = makeGrabbers (gdRI->field, obj);
          if (fldGrabbers.size () == 1)
            {
              //merge the gain and bias
              fldGrabbers[0]->gain *= gdRI->gain;
              fldGrabbers[0]->bias *= gdRI->gain;
              fldGrabbers[0]->bias += gdRI->bias;
              if (gdRI->outputUnits != gridUnits::defUnit)
                {
                  fldGrabbers[0]->outputUnits = gdRI->outputUnits;
                }

              add (fldGrabbers[0], gdRI->column);
            }
          else
            {
              int ccol = gdRI->column;
              for (auto &ggb : fldGrabbers)
                {
                  if (ccol > 0)
                    {
                      add (ggb, ccol++);
                    }
                  else
                    {
                      add (ggb);
                    }
                }
            }
          if (fldGrabbers.empty ())
            {
              obj->log (obj, GD_WARNING_PRINT, "no grabbers created from " + gdRI->field);
              return OBJECT_ADD_FAILURE;
            }
        }
    }


  return OBJECT_ADD_SUCCESS;
}

int gridRecorder::add (const std::string &field, gridCoreObject *obj)
{
  if (field.find_first_of (",;") != std::string::npos)
    {                     //now go into a loop of the comma variables
      //if multiple fields were specified by comma seperation
      stringVec grabberStrings = splitlineBracket (field, ",;");
      for (const auto &fld : grabberStrings)
        {
          auto ret = add (fld, obj);
          if (ret == OBJECT_ADD_FAILURE)
            {
              obj->log (obj, GD_WARNING_PRINT, "unable to add recorder from " + fld);
            }
        }
    }
  else                         //now we get to the interesting bit
    {
      auto fldGrabbers = makeGrabbers (field, obj);
      for (auto &ggb : fldGrabbers)
        {
          add (ggb);
        }
      if (fldGrabbers.empty ())
        {
          obj->log (obj, GD_WARNING_PRINT, "no grabbers created from " + field);
          return OBJECT_ADD_FAILURE;
        }
    }
  return OBJECT_ADD_SUCCESS;
}



