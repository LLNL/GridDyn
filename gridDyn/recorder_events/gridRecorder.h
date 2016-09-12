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

#ifndef GRIDDYN_RECORDER_H_
#define GRIDDYN_RECORDER_H_

#include "fileReaders.h"
#include "gridGrabbers.h"
#include "eventInterface.h"

class gridGrabberInfo
{
public:
  std::string  m_target;        //!< name of the object to target
  std::string field;        //!<the field to record
  std::string rString;     //!<a string defining the recorder
  int column = -1;      //!< (suggested) which column to stick the data in
  index_t offset = kNullLocation;      //!<the offset to use to numerically pick off the state
  double gain = 1.0;      //!<a multiplier factor for the results
  double bias = 0.0;       //!< a shift factor of the results
  gridUnits::units_t outputUnits = gridUnits::defUnit;       //!<which units to output the data
public:
  gridGrabberInfo ()
  {
  }
};


class gridRecorder : public eventInterface
{
public:
  std::string  name;
  std::string description;
  double timePeriod;
  double reqPeriod;
  double startTime = -kBigNum;
  double stopTime = kBigNum;
protected:
  timeSeries2 dataset;
  double triggerTime = -kBigNum;
  std::string filename;
  std::string directory;
  count_t columns = 0;
  std::vector<std::shared_ptr<gridGrabber> > dataGrabbers;        //the data grabbers
  std::vector<int> dataColumns;         //the column where each grabber is stored
  double lastSaveTime = -kBigNum;
  bool recheck = false;
  bool binaryFile = true;
  bool armed = true;
  bool delayProcess = true;          //!< wait to process recorders until other events have executed
  int precision = -1;                //!< precision for writing text files.
  count_t autosave = 0;
public:
  gridRecorder (double time0 = 0,double period = 1.0);
  ~gridRecorder ();

  std::shared_ptr<gridRecorder> clone (gridCoreObject *nobj) const;
  std::shared_ptr<gridRecorder> cloneTo (gridCoreObject *src, gridCoreObject *dest) const;

  change_code trigger (double time) override;
  void recheckColumns ();
  double nextTriggerTime () const override
  {
    return triggerTime;
  }
  event_execution_mode executionMode () const override
  {
    return (delayProcess) ? event_execution_mode::delayed : event_execution_mode::normal;
  }
  int saveFile (const std::string &fileName = "");

  int add (std::shared_ptr<gridGrabber> ggb,int column = -1);
  int add (gridGrabberInfo *gdRI, gridCoreObject *obj);
  int add (const std::string &field, gridCoreObject *obj);
  void setSpace (double span);
  void addSpace (double span);
  bool isArmed () const override
  {
    return armed;
  }

  int set (const std::string &param, double val);
  int set (const std::string &param, const std::string &val);

  const std::string &getFileName () const
  {
    return filename;
  }
  const std::string &getDirectory () const
  {
    return directory;
  }
  void setTime (double time);
  void reset ();

  const timeSeries2 * getData () const
  {
    return &(dataset);
  }
  const std::vector<double> &getTime () const
  {
    return dataset.time;
  }
  const std::vector<double> &getData (size_t col) const
  {
  
       return dataset.data[(col < columns)?col:0];
  }
};

#endif