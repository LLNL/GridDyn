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
#pragma once
#ifndef _FMI_COLLECTOR_H_
#define _FMI_COLLECTOR_H_

#include "measurement/collector.h"
#include <memory>

class fmiCoordinator;

class fmiCollector : public collector
{
protected:
	std::vector<index_t> vrs;
	fmiCoordinator *coord=nullptr;
public:

	fmiCollector();
	explicit fmiCollector(const std::string &name);
	//~fmiCollector();

	virtual std::shared_ptr<collector> clone(std::shared_ptr<collector> gr = nullptr) const override;

	virtual change_code trigger(coreTime time) override;


	void set(const std::string &param, double val) override;
	void set(const std::string &param, const std::string &val) override;

	virtual const std::string &getSinkName() const override;
	friend class fmiCoordinator;
protected:
	virtual void dataPointAdded(const collectorPoint& cp) override;
};
#endif
