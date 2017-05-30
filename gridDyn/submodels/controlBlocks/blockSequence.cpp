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

#include "submodels/controlBlocks/blockSequence.h"
#include "core/coreExceptions.h"
#include "utilities/vectorOps.hpp"
#include  "utilities/matrixData.h"
#include "core/coreObjectTemplates.h"
#include "utilities/stringConversion.h"

blockSequence::blockSequence(const std::string &objName) : basicBlock(objName)
{
	opFlags[use_direct] = true;
}


coreObject *blockSequence::clone(coreObject *obj) const
{
	blockSequence *nobj = cloneBase<blockSequence, basicBlock>(this, obj);
	if (nobj == nullptr)
	{
		return obj;
	}
	
	return nobj;
}

void blockSequence::dynObjectInitializeA(coreTime time0, unsigned long flags)
{
	
	bool diffInput = opFlags[differential_inputActual];
	if (sequence.empty())
	{//create a default sequence with all the blocks
		for (int kk = 0; kk < static_cast<int>(blocks.size()); ++kk)
		{
			sequence.push_back(kk);
		}
	}
	for (auto nn : sequence)
	{
		if (diffInput)
		{
			blocks[nn]->setFlag("differential_input", true);
		}
		blocks[nn]->dynInitializeA(time0, flags);
		diffInput = blocks[nn]->checkFlag(differential_output);
	}
	opFlags[differential_input] = diffInput;
	basicBlock::dynObjectInitializeA(time0, flags);
	updateFlags(); //update the flags for the subObjects;

}
// initial conditions
void blockSequence::dynObjectInitializeB(const IOdata &inputs, const IOdata &desiredOutput, IOdata &fieldSet)
{
	auto cnt = sequence.size();
	if (desiredOutput.empty())
	{
		IOdata inAct{ inputs.size() > 0 ? inputs[0] + bias : 0.0,getRateInput(inputs) };

		for (decltype(cnt) ii = 0; ii<cnt; ++ii)
		{
			blocks[sequence[ii]]->dynInitializeB(inAct, noInputs, inAct);
		}
		basicBlock::dynObjectInitializeB(inAct,desiredOutput,fieldSet);
	}
	else if (inputs.empty())
	{
		IOdata inReq;
		inReq.resize(2);
		basicBlock::dynObjectInitializeB(noInputs, desiredOutput, inReq);
		for (int ii = static_cast<int>(cnt-1); ii>=0; --ii)
		{
			blocks[sequence[ii]]->dynInitializeB(noInputs, inReq, inReq);
		}
		fieldSet = inReq;
		fieldSet[0] -= bias;
	}
	else// we have inputs and outputs
	{
		fieldSet = desiredOutput;  //the field is the output
	}
	

}


void blockSequence::add(coreObject *obj)
{
	if (dynamic_cast<basicBlock *>(obj))
	{
		add(static_cast<basicBlock *>(obj));
	}
	else
	{
		throw(unrecognizedObjectException(this));
	}
}

void blockSequence::add(basicBlock *blk)
{
	if (blk->locIndex == kNullLocation)
	{
		blk->locIndex = static_cast<index_t>(blocks.size());
	}

	if (blocks.size() < blk->locIndex)
	{
		blocks.resize(blk->locIndex + 1);
	}
	if (blocks[blk->locIndex])
	{
		remove(blocks[blk->locIndex]);
	}
	blocks[blk->locIndex] = blk;
	addSubObject(blk);
		
}


void blockSequence::updateLocalCache(const IOdata &, const stateData &sD, const solverMode &sMode)
{
	if (!sD.updateRequired(seqID))
	{
		return;
	}

	if (blockOutputs.size() != sequence.size())
	{
		blockOutputs.resize(sequence.size());
		blockDoutDt.resize(sequence.size());
	}
	for (index_t kk = 0; kk < sequence.size(); ++kk)
	{
		basicBlock *blk = blocks[sequence[kk]];
		blockOutputs[kk] = blk->getBlockOutput(sD, sMode);
		blockDoutDt[kk] = (blk->checkFlag(differential_output)) ? blk->getBlockDoutDt(sD, sMode) : 0.0;
	}
	
	seqID = sD.seqID;
}

double blockSequence::step(coreTime ttime, double input)
{
	//compute a core sample time then cycle through all the objects at that sampling rate
	input = input + bias;
	double drate = (input - prevInput) / (ttime - prevTime);
	while (prevTime < ttime)
	{
		coreTime newTime = std::min(ttime, prevTime + sampleTime);
		double blockInput = prevInput + drate*(newTime - prevTime);
		for (auto &blkIn : sequence)
		{
			blockInput = blocks[blkIn]->step(newTime, blockInput);
		}
		prevTime = newTime;
	}
	return basicBlock::step(ttime, input);
	
}

void blockSequence::residElements(double input, double didt, const stateData &sD, double resid[], const solverMode &sMode)
{
	updateLocalCache(noInputs, sD, sMode);
	size_t cnt = sequence.size();
	double in = input+bias;
	double indt = didt;
	for (size_t ii=0;ii<cnt;++ii)
	{
		blocks[sequence[ii]]->residElements(in, indt, sD, resid, sMode);
		in = blockOutputs[sequence[ii]];
		indt = blockDoutDt[sequence[ii]];
	}
	limiterResidElements(in, indt, sD, resid, sMode);
	
}

void blockSequence::derivElements(double input, double didt, const stateData &sD, double deriv[], const solverMode &sMode)
{
	updateLocalCache(noInputs, sD, sMode);
	size_t cnt = sequence.size();
	double in = input+bias;
	double indt = didt;
	for (size_t ii = 0; ii < cnt; ++ii)
	{
		blocks[sequence[ii]]->derivElements(in, indt, sD, deriv, sMode);
		in = blockOutputs[sequence[ii]];
		indt = blockDoutDt[sequence[ii]];
	}
	basicBlock::derivElements(in, indt, sD, deriv, sMode);
}

void blockSequence::algElements(double input, const stateData &sD, double update[], const solverMode &sMode)
{
	updateLocalCache(noInputs, sD, sMode);
	size_t cnt = sequence.size();
	double in = input+bias;
	for (size_t ii = 0; ii<cnt; ++ii)
	{
		blocks[sequence[ii]]->algElements(in, sD, update, sMode);
		in = blockOutputs[sequence[ii]];
	}
	basicBlock::algElements(input, sD, update, sMode);
}


void blockSequence::jacElements(double input, double didt, const stateData &sD, matrixData<double> &ad, index_t argLoc, const solverMode &sMode)
{

	updateLocalCache(noInputs, sD, sMode);
	size_t cnt = sequence.size();
	double in = input+bias;
	double indt = didt;
	index_t aLoc = argLoc;
	for (size_t ii = 0; ii<cnt; ++ii)
	{
		blocks[sequence[ii]]->jacElements(in, indt, sD, ad, aLoc, sMode);
		in = blockOutputs[sequence[ii]];
		indt = blockDoutDt[sequence[ii]];
		aLoc = blocks[sequence[ii]]->getOutputLoc(sMode,0);
	}
	basicBlock::jacElements(in, indt, sD, ad, aLoc, sMode);

	
}


void blockSequence::rootTest(const IOdata &inputs, const stateData &sD, double root[], const solverMode &sMode)
{
	updateLocalCache(noInputs, sD, sMode);
	size_t cnt = sequence.size();
	IOdata inAct{ inputs.size() > 0 ? inputs[0] + bias : kNullVal,getRateInput(inputs) };
	
	for (size_t ii = 0; ii<cnt; ++ii)
	{
		blocks[sequence[ii]]->rootTest(inAct, sD, root, sMode);
		inAct[0] = blockOutputs[sequence[ii]];
		inAct[1] = blockDoutDt[sequence[ii]];
	}
	basicBlock::rootTest(inAct, sD, root, sMode);
	

}


void blockSequence::rootTrigger(coreTime ttime, const IOdata &inputs, const std::vector<int> &rootMask, const solverMode &sMode)
{
	size_t cnt = sequence.size();
	IOdata inAct{ inputs.size() > 0 ? inputs[0] + bias : kNullVal,getRateInput(inputs) };

	for (size_t ii = 0; ii<cnt; ++ii)
	{
		blocks[sequence[ii]]->rootTrigger(ttime,inAct, rootMask, sMode);
		inAct[0] = blockOutputs[sequence[ii]];
		inAct[1] = blockDoutDt[sequence[ii]];
	}
	basicBlock::rootTrigger(ttime,inAct, rootMask, sMode);
}

change_code blockSequence::rootCheck(const IOdata &inputs, const stateData &sD, const solverMode &sMode, check_level_t level)
{
	change_code ret = change_code::no_change;
	updateLocalCache(noInputs, sD, sMode);
	size_t cnt = sequence.size();
	IOdata inAct{ inputs.size() > 0 ? inputs[0] + bias : kNullVal,getRateInput(inputs) };

	for (size_t ii = 0; ii<cnt; ++ii)
	{
		auto lret=blocks[sequence[ii]]->rootCheck(inAct, sD, sMode,level);
		inAct[0] = blockOutputs[sequence[ii]];
		inAct[1] = blockDoutDt[sequence[ii]];
		ret = std::max(ret, lret);
	}
	auto lret=basicBlock::rootCheck(inAct, sD, sMode,level);
	ret = std::max(ret, lret);
	return ret;
}

void blockSequence::setFlag(const std::string &flag, bool val)
{
	if (flag == "differential_input")
	{
		opFlags.set(differential_inputActual, val);
	}
	else
	{
		basicBlock::setFlag(flag, val);
	}

}
// set parameters
void blockSequence::set(const std::string &param, const std::string &val)
{
	if (param == "sequence")
	{

	}
	else
	{
		basicBlock::set(param, val);
	}

}

void blockSequence::set(const std::string &param, double val, gridUnits::units_t unitType)
{

	if (param[0] == '#')
	{

	}
	else
	{
		basicBlock::set(param, val, unitType);
	}


}

double blockSequence::subBlockOutput(index_t blockNum) const
{
	if (blockNum < blocks.size())
	{
		return blocks[blockNum]->getBlockOutput();
	}
	return kNullVal;
}

double blockSequence::subBlockOutput(const std::string &blockname) const
{
	auto blk = static_cast<basicBlock *>(find(blockname));
	if (blk)
	{
		return blk->getBlockOutput();
	}
	return kNullVal;
}

coreObject* blockSequence::getSubObject(const std::string & typeName, index_t num) const
{
	if (typeName=="block")
	{		
		if (num < blocks.size())
		{
			return blocks[num];
		}
	}
	return gridObject::getSubObject(typeName, num);
}

coreObject * blockSequence::findByUserID(const std::string & typeName, index_t searchID) const
{
	if (typeName == "block")
	{
		for (auto &blk : blocks)
		{
			if (blk->getUserID() == searchID)
			{
				return blk;
			}
		}
	}
	return gridObject::findByUserID(typeName, searchID);
}