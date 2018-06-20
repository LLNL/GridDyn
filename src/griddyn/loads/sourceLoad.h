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
#pragma once

#include "zipLoad.h"
#include <array>
namespace griddyn
{
class Source;
namespace loads
{
/** @brief a load that uses sources to calculate the values for the each of the load parameters
eventually will replace most of the shaped loads*/
class sourceLoad : public zipLoad
{
public:
	enum sourceLoc
	{
		p_source = 0,
		q_source = 1,
		yp_source = 2,
		yq_source = 3,
		ip_source = 4,
		iq_source = 5,
		r_source = 6,
		x_source = 7,
	};
	enum class sourceType
	{
		other,
		pulse,
		sine,
		random,
	};
private:
	
	std::vector<Source *> sources;
	std::array<int, 8> sourceLink; //source lookups for the values
	sourceType sType = sourceType::other;
public:
	
	explicit sourceLoad(const std::string &objName = "sourceLoad_$");
	sourceLoad(sourceType type, const std::string &objName = "sourceLoad_$");
	virtual coreObject * clone(coreObject *obj = nullptr) const override;

	virtual void add(coreObject *obj) override;
	virtual void add(Source *src);
	virtual void remove(coreObject *obj) override;
	virtual void remove(Source *src);
	
	virtual void set(const std::string &param, const std::string &val) override;
	virtual void set(const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;
	virtual void setFlag(const std::string &flag, bool val=true) override;

	virtual void pFlowObjectInitializeA(coreTime time0, std::uint32_t flags) override;
	virtual void dynObjectInitializeA(coreTime time0, std::uint32_t flags) override;

	virtual void updateLocalCache(const IOdata &inputs, const stateData &sD, const solverMode &sMode) override;
	
	virtual void setState(coreTime time, const double state[], const double dstate_dt[], const solverMode &sMode) override;
	virtual void timestep(coreTime time, const IOdata &inputs, const solverMode &sMode) override;

	coreObject *find(const std::string &obj) const override;
private:
	void getSourceLoads();
	Source *makeSource(sourceLoc loc);
	Source *findSource(const std::string &srcname);
	Source *findSource(const std::string &srcname) const;
};
}//namespace loads
}//namespace griddyn
