/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil;  eval: (c-set-offset 'innamespace 0); -*- */
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

#ifndef DIME_COLLECTOR_HEADER_
#define DIME_COLLECTOR_HEADER_

#include "measurement/collector.h"
class dimeClientInterface;

class dimeCollector :public collector
{
private:
	std::string server;
	std::string processName;
	std::unique_ptr<dimeClientInterface> dime;
public:
	dimeCollector(coreTime time0 = timeZero, coreTime period = timeOneSecond);
	explicit dimeCollector(const std::string &name);
	~dimeCollector();

	virtual std::shared_ptr<collector> clone(std::shared_ptr<collector> gr = nullptr) const override;

	virtual change_code trigger(coreTime time) override;


	void set(const std::string &param, double val) override;
	void set(const std::string &param, const std::string &val) override;

	virtual const std::string &getSinkName() const override;

};
#endif