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
#ifndef _FMI_COORDINATOR_H_
#define _FMI_COORDINATOR_H_

#include "core/coreObject.h"

class fmiEvent;
class fmiCollector;

/** class to manage the linkages from the FMI to the GridDyn objects*/
class fmiCoordinator : public coreObject
{
private:
	typedef struct
	{
		std::string name;
		fmiEvent *evnt;
	}inputSet;

	typedef struct
	{
		std::string name;
		int column;
		index_t outIndex;
		fmiCollector *col;
	}outputSet;

	using vrInputPair = std::pair<index_t, inputSet>;
	using vrOutputPair = std::pair<index_t, outputSet>;

	std::vector <vrInputPair>  inputVR;
	std::vector <vrInputPair>  paramVR;
	std::vector <vrOutputPair>  outputVR;
	std::vector<double> outputPoints;
	std::vector<fmiCollector *> collectors;
	index_t nextVR = 0;
	
public:

	explicit fmiCoordinator(const std::string &name = "");
	void registerParameter(const std::string &name, fmiEvent *evnt);
	void registerInput(const std::string &name,  fmiEvent *evnt);

	void registerOutput(const std::string &name,  int column, fmiCollector *col);

	void sendInput(index_t vr, double val);
	double getOutput(index_t vr);
	void updateOutputs(coreTime time);
	const std::string &getFMIName() const;

	std::vector <vrInputPair> &getInputs()
	{
		return inputVR;
	}
	std::vector <vrInputPair> &getParameters()
	{
		return paramVR;
	}
	std::vector <vrOutputPair> &getOutputs()
	{
		return outputVR;
	}
};
#endif
