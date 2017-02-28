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
#ifndef _FMI_RUNNER_H_
#define _FMI_RUNNER_H_

#include "gridDynRunner.h"
#include "fmi/FMI2/fmi2FunctionTypes.h"

class fmiCoordinator;
class readerInfo;

class fmiRunner : public GriddynRunner
{
	private:
		std::shared_ptr<fmiCoordinator> coord;
public:
	fmiRunner(const std::string &name, const std::string &resourceLocations, const fmi2CallbackFunctions* functions);

	//most of the stuff that would be in here is dealt with in the constructor and has to be handled differently in the FMU
	virtual int Initialize(int argc, char *argv[]) final;
	

	virtual void Run(void);

	void UpdateOutputs();

	virtual coreTime Step(coreTime time);

	virtual void Finalize(void);

	std::uint64_t GetID() const;

	virtual void Set(index_t vr, double val);
	virtual double Get(index_t vr);

};
#endif
