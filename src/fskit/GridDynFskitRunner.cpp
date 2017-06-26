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

#include "GridDynFskitRunner.h"
#include "gridDynFederatedScheduler.h"
#include "griddyn-tracer.h"

GriddynFskitRunner::GriddynFskitRunner()
{
}

int GriddynFskitRunner::Initialize(int argc, char *argv[], std::shared_ptr<fskit::GrantedTimeWindowScheduler> scheduler)
{
	if (scheduler)
	{
		GriddynFederatedScheduler::Initialize(scheduler);
	}
	GRIDDYN_TRACER("GridDyn::GriddynRunner::Initialize");
	return GriddynRunner::Initialize(argc, argv);
}

int GriddynFskitRunner::Initialize(int argc, char *argv[])
{
	GRIDDYN_TRACER("GridDyn::GriddynRunner::Initialize");
	return GriddynRunner::Initialize(argc, argv);
}

void GriddynFskitRunner::Run()
{
	GRIDDYN_TRACER("GridDyn::GriddynRunner::Run");
	GriddynRunner::Run();
}

void GriddynFskitRunner::Finalize()
{
	GRIDDYN_TRACER("GridDyn::GriddynRunner::Finalize");
	GriddynRunner::Finalize();
}
