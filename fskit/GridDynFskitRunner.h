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
#ifndef GRIDDYN_FSKIT_RUNNER_H
#define GRIDDYN_FSKIT_RUNNER_H
#include "gridDynRunner.h"

namespace fskit
{
	class GrantedTimeWindowScheduler;
}
class GriddynFskitRunner : public GriddynRunner
{
	GriddynFskitRunner();

	int Initialize(int argc, char *argv[], std::shared_ptr<fskit::GrantedTimeWindowScheduler> scheduler);
	virtual int Initialize(int argc, char *argv[]) override;

	virtual void Run() override;
	virtual void Finalize() override;
};
#endif
