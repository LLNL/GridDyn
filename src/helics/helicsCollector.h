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

#include "../griddyn/measurement/collector.h"
#include <utility>

namespace griddyn
{
namespace helicsLib
{
class helicsCollector :public collector
{
private:
	std::vector<std::pair<std::string, std::string>> complexPairs;
	std::vector<std::string> cnames;
public:
	helicsCollector(coreTime time0 = timeZero, coreTime period = timeOneSecond);
	explicit helicsCollector(const std::string &name);

	virtual std::unique_ptr<collector> clone() const override;
	virtual void cloneTo(collector *col) const override;
	virtual change_code trigger(coreTime time) override;


	void set(const std::string &param, double val) override;
	void set(const std::string &param, const std::string &val) override;

	virtual const std::string &getSinkName() const override;
protected:
	void dataPointAdded(const collectorPoint &cp) override;
};

}// namespace helicsLib
} // namespace griddyn