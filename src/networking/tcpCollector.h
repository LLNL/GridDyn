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

#include "griddyn/measurement/collector.h"

namespace griddyn
{
namespace tcpLib
{
class TcpConnection;

class tcpCollector :public collector
{
private:
	std::string server;
	std::string port;
    std::shared_ptr<TcpConnection> connection;
public:
	tcpCollector(coreTime time0 = timeZero, coreTime period = timeOneSecond);
	explicit tcpCollector(const std::string &name);
	~tcpCollector();

	virtual std::unique_ptr<collector> clone() const override;

	virtual void cloneTo(collector *col) const override;
	virtual change_code trigger(coreTime time) override;


	void set(const std::string &param, double val) override;
	void set(const std::string &param, const std::string &val) override;

	virtual const std::string &getSinkName() const override;

};

}//namespace tcpLib
}//namespace griddyn