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

#ifndef BLOCK_SEQUENCE_H_
#define BLOCK_SEQUENCE_H_

#include "submodels/gridControlBlocks.h"
/** @brief class implementing a sequence of blocks as a single block 
A block is defined as a single input single output subModel.  This object takes any number of blocks in a sequence
and processes them in the appropriate fashion.  
*/
class blockSequence: public basicBlock
{

public:
	
protected:
	enum sequence_flags
	{
		differential_inputActual = object_flag11, //!< flag specifying that the outer input is differential
	};
	std::vector<basicBlock *> blocks;  //!< the building blocks in the sequence
	std::vector<int> sequence;  //!< a numerical ordering of the sequence
private:
	std::vector<double> blockOutputs;		//!< temporary storage for block outputs
	std::vector<double> blockDoutDt;		//!< temporary storage for block ramp outputs
	index_t seqID=0;							//!< sequence ID for last update
	double sampleTime = kBigNum;
public:
	/** @brief default constructor*/
	explicit blockSequence(const std::string &objName = "block_#");

	virtual coreObject * clone(coreObject *obj = nullptr) const override;
protected:
	virtual void dynObjectInitializeA(coreTime time0, unsigned long flags) override;
	virtual void dynObjectInitializeB(const IOdata &inputs, const IOdata &desiredOutput, IOdata &inputSet) override;
public:
	virtual void add(coreObject *obj) override;
	virtual void add(basicBlock *blk);

	virtual void setFlag(const std::string &flag, bool val) override;
	virtual void set(const std::string &param, const std::string &val) override;
	virtual void set(const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;

	virtual void residElements(double input, double didt, const stateData &sD, double resid[], const solverMode &sMode) override;

	
	virtual void derivElements(double input, double didt, const stateData &sD, double deriv[], const solverMode &sMode) override;
	
	virtual void algElements(double input, const stateData &sD, double deriv[], const solverMode &sMode) override;
	
	virtual void jacElements(double input, double didt, const stateData &sD, matrixData<double> &ad, index_t argLoc, const solverMode &sMode) override;

	
	virtual double step(coreTime time, double input) override;
	virtual void rootTest(const IOdata &inputs, const stateData &sD, double roots[], const solverMode &sMode) override;
	virtual void rootTrigger(coreTime ttime, const IOdata &inputs, const std::vector<int> &rootMask, const solverMode &sMode) override;
	virtual change_code rootCheck(const IOdata &inputs, const stateData &sD, const solverMode &sMode, check_level_t level) override;
	//virtual void setTime(coreTime time){prevTime=time;};
	/** get the output of one of the component blocks
	*@param[in] blockNum the index of the block to the get the output of 
	@return the output value of the requested block
	*/
	double subBlockOutput(index_t blockNum) const;
	/** get the output of one of the component blocks by name
	*@param[in] blockname the name of the block to the get the output of
	@return the output value of the requested block
	*/
	double subBlockOutput(const std::string &blockname) const;

	virtual void updateLocalCache(const IOdata &inputs, const stateData &sD, const solverMode &sMode) override;

	virtual coreObject* getSubObject(const std::string & typeName, index_t num) const override;

	virtual coreObject * findByUserID(const std::string & typeName, index_t searchID) const override;
};

#endif