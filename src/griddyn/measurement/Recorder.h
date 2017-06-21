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

#ifndef GRIDDYN_RECORDER_H_
#define GRIDDYN_RECORDER_H_

#include "utilities/timeSeriesMulti.hpp"
#include "measurement/collector.h"

namespace griddyn
{

/** class to store and save data from the grid, based on a collector */
class Recorder : public collector
{
protected:
  
  coreTime lastSaveTime = negTime; //!< the last time the recorder saved to file
  timeSeriesMulti<double,coreTime> dataset;  //!< the actual time series data
  std::string fileName_; //!< the fileName to store the data
  std::string directory_; //!< the directory to generate the specified file 
  
  bool binaryFile = true;	//!< flag indicating if the file is binary
  bool firstTrigger = true; //!< flag indicating that the recorder has not been triggered yet
  int precision = -1;                //!< precision for writing text files.
  count_t autosave = 0;			//!< flag indicating the recorder should autosave after the given number of points
public:
  Recorder (coreTime time0 = timeZero,coreTime period = 1.0);
  explicit Recorder(const std::string &name);
  ~Recorder ();

  virtual std::shared_ptr<collector> clone (std::shared_ptr<collector> gr=nullptr) const override;

  virtual change_code trigger (coreTime time) override;

  
  void saveFile (const std::string &fileName = "");

  void setSpace (double span);
  void addSpace (double span);
 
  void set (const std::string &param, double val) override;
  void set (const std::string &param, const std::string &val) override;

  virtual void flush() override;
  virtual const std::string &getSinkName() const override;

  const std::string &getFileName () const
  {
    return fileName_;
  }
  const std::string &getDirectory () const
  {
    return directory_;
  }
  
  void reset ();

  //const timeSeriesMulti<double,coreTime> * getData () const
   const auto &getTimeSeries() const
  {
    return dataset;
  }
  const std::vector<coreTime> &getTime () const
  {
    return dataset.time();
  }
  const std::vector<double> &getData (count_t col) const
  {
       return dataset.data((col < columns)?col:0);
  }
private:
	void fillDatasetFields();
};


}//namespace griddyn
#endif