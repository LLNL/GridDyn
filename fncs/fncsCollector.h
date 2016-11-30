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

#ifndef DIME_COLLECTOR_HEADER_
#define DIME_COLLECTOR_HEADER_

#include "recorder_events/collector.h"
#include <utility>

class fncsCollector :public collector
{
private:
	std::vector<std::pair<std::string,std::string>> complexPairs;
	std::vector<std::string> cnames;
public:
	fncsCollector(gridDyn_time time0 = timeZero, gridDyn_time period = timeOne);
	explicit fncsCollector(const std::string &name);
	~fncsCollector();

	virtual std::shared_ptr<collector> clone(std::shared_ptr<collector> gr = nullptr) const override;

	virtual change_code trigger(gridDyn_time time) override;


	void set(const std::string &param, double val) override;
	void set(const std::string &param, const std::string &val) override;

	virtual const std::string &getSinkName() const override;
protected:
	void dataPointAdded(const collectorPoint &cp) override;
};
#endif