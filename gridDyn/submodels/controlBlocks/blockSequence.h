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

#ifndef BLOCK_SEQUENCE_H_
#define BLOCK_SEQUENCE_H_

#include "subModels/gridControlBlocks.h"
/** @brief class implementing a sequence of blocks as a single block 
A block is defined as a single input single output submodel.  This object takes any number of blocks in a sequence
and processes them in the appropriate fashion.  
*/
class blockSequence: public basicBlock
{

public:
	
protected:
	enum sequence_flags
	{
		differential_inputActual = object_flag11,
	};
	std::vector<basicBlock *> blocks;
	std::vector<int> sequence;
private:
	std::vector<double> blockOutputs;
	std::vector<double> blockDoutDt;
	double sampleTime = kBigNum;
public:
	/** @brief default constructor*/
	explicit blockSequence(const std::string &objName = "block_#");

	virtual coreObject * clone(coreObject *obj = nullptr) const override;
protected:
	virtual void objectInitializeA(gridDyn_time time0, unsigned long flags) override;
	virtual void objectInitializeB(const IOdata &args, const IOdata &outputSet, IOdata &inputSet) override;
public:
	virtual void add(coreObject *obj) override;
	virtual void add(basicBlock *blk);

	virtual void setFlag(const std::string &flag, bool val) override;
	virtual void set(const std::string &param, const std::string &val) override;
	virtual void set(const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;
	virtual index_t findIndex(const std::string &field, const solverMode &sMode) const override;

	virtual void residElements(double input, double didt, const stateData &sD, double resid[], const solverMode &sMode) override;

	
	virtual void derivElements(double input, double didt, const stateData &sD, double deriv[], const solverMode &sMode) override;
	
	virtual void algElements(double input, const stateData &sD, double deriv[], const solverMode &sMode);
	
	virtual void jacElements(double input, double didt, const stateData &sD, matrixData<double> &ad, index_t argLoc, const solverMode &sMode);

	
	virtual double step(gridDyn_time time, double input) override;
	virtual void rootTest(const IOdata &args, const stateData &sD, double roots[], const solverMode &sMode) override;
	virtual void rootTrigger(gridDyn_time ttime, const IOdata &args, const std::vector<int> &rootMask, const solverMode &sMode) override;
	virtual change_code rootCheck(const IOdata &args, const stateData &sD, const solverMode &sMode, check_level_t level) override;
	//virtual void setTime(gridDyn_time time){prevTime=time;};
	double subBlockOutput(index_t blockNum) const;
	double subBlockOutput(const std::string &blockname) const;
private:
	basicBlock *findBlock(const std::string &blockname) const;
};

#endif