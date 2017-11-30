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

#ifndef GRABBER_SOURCE_H_
#define GRABBER_SOURCE_H_

#include "sources/rampSource.h"


namespace griddyn
{
class grabberSet;
namespace sources
{
/** source to grab data from another location and use it in another context*/
class grabberSource : public rampSource
{
private:
	std::unique_ptr<grabberSet> gset; //!< the grabberSet to get the data
	std::string field;  //!< the field to grab
	std::string target; //!< the name of the target
	parameter_t multiplier; //!< a multiplier on the grabber value
public:
	grabberSource(const std::string &objName = "grabbersource_#");
	~grabberSource();
protected:
	virtual void pFlowObjectInitializeA(coreTime time0, std::uint32_t flags) override;

	virtual void dynObjectInitializeB(const IOdata &inputs, const IOdata &desiredOutput, IOdata &fieldSet) override;

	virtual coreObject * clone(coreObject *obj = nullptr) const override;
	/** update the target field of the grabber*/
	void updateField(const std::string &newfield);

	/** update the target object of the grabber*/
	void updateTarget(const std::string &newTarget);

	/** update the target object of the grabber directly*/
	void updateTarget(coreObject *obj);

	virtual void setFlag(const std::string &flag, bool val) override;
	virtual void set(const std::string &param, const std::string &val) override;
	virtual void set(const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;
	virtual double get(const std::string &param, gridUnits::units_t unitType = gridUnits::defUnit) const override;

	virtual IOdata getOutputs(const IOdata &inputs, const stateData &sD, const solverMode &sMode) const override;
	virtual double getOutput(const IOdata &inputs, const stateData &sD, const solverMode &sMode, index_t num = 0) const override;

	virtual double getOutput (index_t outputNum = 0) const override;

	virtual double getDoutdt(const IOdata &inputs, const stateData &sD, const solverMode &sMode, index_t num = 0) const override;
};
}//namespace sources
}//namespace griddyn

#endif

