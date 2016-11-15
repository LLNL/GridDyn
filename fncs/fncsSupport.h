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

#ifndef FNCS_SUPPORT_HEADER_
#define FNCS_SUPPORT_HEADER_

#include "fncs.hpp"
#include "gridDynTypes.h"
#include <vector>
#include <set>
#include <memory>

#ifdef FULLCPP14
template<typename T>
constexpr T fncsTickPerSecond = T(1'000'000'000.00);
#else
const double fncsTickPerSecond_f(1'000'000'000.00);
const unsigned long long fncsTickPerSecond_i(1'000'000'000);
#endif

fncs::time gd2fncsTime(gridDyn_time evntTime);

gridDyn_time fncs2gdTime(fncs::time ftime);

class zplInfo
{
public:
	std::string name;
	std::string brokerAddress;
	int minTimeStep;
	std::string minTimeStepUnits;
};

class fncsRegister
{
private:
	static std::shared_ptr<fncsRegister> p_instance;

	std::set<std::string> subscriptions;
	std::set<std::string> publications;

	fncsRegister() {};

public:
	void registerSubscription(const std::string &sub);
	void registerPublication(const std::string &pub);
	void makeZPLfile(const std::string &fname, const zplInfo &info);

	static std::shared_ptr<fncsRegister> instance();
};
#endif