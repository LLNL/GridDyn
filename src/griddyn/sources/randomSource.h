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

#ifndef RANDOM_SOURCE_H_
#define RANDOM_SOURCE_H_

#include "sources/rampSource.h"
namespace utilities
{
	class gridRandom;
}

namespace griddyn
{
namespace sources
{
/** @brief a source generating a random output*/
class randomSource : public rampSource
{
public:
	/** random source flags*/
	enum random_source_flags
	{
		interpolate_flag = object_flag5, //!< indicator that the output should be interpolated
		proportional_flag = object_flag6,	//!< indicator that the random change is proportional to the current value
		repeated_flag = object_flag7,	//!< indicator that the random generation should be repeated
		triggered_flag = object_flag8,	//!< indicator that the random generation has been triggered

	};

protected:
	parameter_t param1_t = 0.0;           //!< parameter 1 for time distribution
	parameter_t param2_t = 100;           //!< parameter 2 for time distribution
	parameter_t param1_L = 0.0;           //!< parameter 1 for level distribution
	parameter_t param2_L = 0.0;           //!< parameter 2 for level distribution
	parameter_t zbias = 0.0;           //!< a factor describing the preference of changes to trend toward zero mean
	parameter_t offset = 0.0;          //!< the current bias in the value
	coreTime keyTime = 0.0;         //!< the next time change
	std::string timeDistribution = "constant";  //!< string representing the time Distribution random number generation type
	std::string valDistribution = "constant";	//!< string representing the value Distribution random number generation type
	std::unique_ptr<utilities::gridRandom> timeGenerator;            //!< random number generator for the time
	std::unique_ptr<utilities::gridRandom> valGenerator;                     //!< random number generator for the value

public:
	randomSource(const std::string &objName = "randomsource_#", double startVal = 0.0);
	~randomSource(); //included so the definition of gridRandom doesn't have to be
	virtual coreObject * clone(coreObject *obj = nullptr) const override;

	virtual void pFlowObjectInitializeA(coreTime time0, std::uint32_t flags) override;
	virtual void timestep(coreTime time, const IOdata &inputs, const solverMode &sMode) override;

	/** check if the random number generation has been triggered*/
	bool isTriggered()
	{
		return opFlags[triggered_flag];
	}
	virtual void reset(reset_levels level = reset_levels::minimal) override;

	virtual void set(const std::string &param, const std::string &val) override;
	virtual void set(const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;
	virtual void updateA(coreTime time) override;

	void setFlag(const std::string &flag, bool val = true) override;

	virtual void updateOutput(coreTime time) override;
private:
	/** generate the next step in the random process this source represents*/
	void nextStep(coreTime triggerTime);
	/** generate a random time for the next update*/
	coreTime ntime();
	/** generate a new random value*/
	double nval();
	void timeParamUpdate();
	void valParamUpdate();
	/** compute a bias shift in the random generation*/
	double computeBiasAdjust();
};
}//namespace sources
}//namespace griddyn

#endif

