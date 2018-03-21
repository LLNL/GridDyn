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

#include "griddyn/measurement/collector.h"
#include <memory>

namespace griddyn
{
namespace fmi
{
class fmiCoordinator;

/** collector object to interface with an fmi output*/
class fmiCollector : public collector
{
protected:
	std::vector<index_t> vrs;  //!< vector of fmi value references that match the data
	fmiCoordinator *coord=nullptr;	//!< pointer the fmi coordination object
public:

	fmiCollector();
	explicit fmiCollector(const std::string &name);
	//~fmiCollector();

	virtual std::unique_ptr<collector> clone() const override;

	virtual void cloneTo(collector *gr = nullptr) const override;
	virtual change_code trigger(coreTime time) override;


	void set(const std::string &param, double val) override;
	void set(const std::string &param, const std::string &val) override;

	virtual const std::string &getSinkName() const override;

	virtual coreObject *getOwner() const override;
	friend class fmiCoordinator;
protected:
	virtual void dataPointAdded(const collectorPoint& cp) override;
};

}//namespace fmi
}//namespace griddyn

