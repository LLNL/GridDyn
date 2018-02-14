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

#ifndef FILE_SOURCE_H_
#define FILE_SOURCE_H_

#include "sources/rampSource.h"
#include "utilities/timeSeries.hpp"

namespace griddyn
{
namespace sources
{
/** Source getting its data from a file*/
class fileSource : public rampSource
{
public:
	/** enumerations of flags used in the file source*/
	enum file_load_flags
	{
		use_absolute_time_flag = object_flag7,  //!< flag indicating use of an absolute time reference in the file
		use_step_change_flag = object_flag8,	//!< flag indicating a step function change on the output
	};
private:
	std::string fileName_;  //!< name of the file
	timeSeries<double, coreTime> schedLoad;  //!< time series containing the output schedule
	index_t currIndex = 0;                //!< the current location in the file
	count_t count = 0;            //!< the total number of elements in the file
	index_t m_column = 0;         //!< the column of the file to use
	// 4 byte structure hole here
public:
	fileSource(const std::string &fileName = "", int column = 0);

	virtual coreObject * clone(coreObject *obj = nullptr) const override;

	virtual void setFlag(const std::string &flag, bool val) override;
	virtual void set(const std::string &param, const std::string &val) override;
	virtual void set(const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;

	int setFile(const std::string &fileName, index_t column);
	virtual void pFlowObjectInitializeA(coreTime time0, std::uint32_t flags) override;

	virtual void updateA(coreTime time) override;
	virtual void timestep(coreTime time, const IOdata &inputs, const solverMode &sMode) override;
	//let predict fall through to ramp function

private:
	/** @brief load the file*/
	int loadFile();
};
}//namespace sources
}//namespace griddyn

#endif

