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

#include "contingency.h"
#include "gridDyn.h"

int contingency::contCount = 0;

contingency::contingency()
{
  ++contCount;
  id = contCount;
  name = "contingency_" + std::to_string (id);
}

contingency::contingency(std::vector<char> &/*buffer*/)
{

}

void contingency::runContingency()
{

}

void contingency::serialize(std::vector<char> &/*buffer*/)
{

}

void contingency::setContingencyRoot(gridDynSimulation *gdSim)
{
	if (gds != gdSim)
	{
		gds = gdSim;
	}
	
}