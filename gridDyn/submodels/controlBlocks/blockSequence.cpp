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

#include "submodels/controlBlocks/blockSequence.h"
#include "core/gridDynExceptions.h"
#include "vectorOps.hpp"
#include "matrixData.h"
#include "gridCoreTemplates.h"
#include "stringConversion.h"

blockSequence::blockSequence(const std::string &objName) : basicBlock(objName)
{
	opFlags.set(use_state);
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

void blockSequence::objectInitializeA(gridDyn_time time0, unsigned long flags)
{
	
	bool diffInput = opFlags[differential_inputActual];
	for (auto nn : sequence)
	{
		if (diffInput)
		{
			blocks[nn]->setFlag("differential_input", true);
		}
		blocks[nn]->initializeA(time0, flags);
		diffInput = blocks[nn]->checkFlag(differential_output);
	}
	opFlags[differential_input] = diffInput;
	basicBlock::objectInitializeA(time0, flags);
	updateFlags(); //update the flags for the subObjects;

}
// initial conditions
void blockSequence::objectInitializeB(const IOdata &args, const IOdata &outputSet, IOdata &fieldSet)
{

	

}


void blockSequence::add(coreObject *obj)
{
	if (dynamic_cast<basicBlock *>(obj))
	{
		add(static_cast<basicBlock *>(obj));
	}
	else
	{
		throw(invalidObjectException(this));
	}
}

void blockSequence::add(basicBlock *blk)
{
	if (blk->locIndex == kNullLocation)
	{
		blk->locIndex = blocks.size();
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


double blockSequence::step(gridDyn_time ttime, double input)
{
	//compute a core sample time then cycle through all the objects at that sampling rate
	return 0.0;
}

void blockSequence::residElements(double input, double didt, const stateData &sD, double resid[], const solverMode &sMode)
{
	if ((opFlags[differential_output]) && (!hasDifferential(sMode)))
	{
		return;
	}

	auto so = offsets.getOffsets(sMode);
	if (so->total.diffSize > 0)
	{
		derivElements(input, didt, sD, resid, sMode);
		for (index_t ii = 0; ii < so->total.diffSize; ++ii)
		{
			resid[so->diffOffset + ii] -= sD.dstate_dt[so->diffOffset + ii];
		}
	}


	if (so->total.algSize > 0)
	{
		algElements(input, sD, resid, sMode);
		for (index_t ii = 0; ii < so->total.algSize; ++ii)
		{
			resid[so->algOffset + ii] -= sD.state[so->algOffset + ii];
		}
	}

}

void blockSequence::derivElements(double input, double didt, const stateData &sD, double deriv[], const solverMode &sMode)
{
	if (opFlags[differential_input])
	{
		auto offset = offsets.getDiffOffset(sMode) + limiter_diff;
		double ival = input + bias;
		deriv[offset] = K * didt;

		if (limiter_diff > 0)
		{
			return basicBlock::derivElements(input, didt, sD, deriv, sMode);
		}
	}
}

void blockSequence::algElements(double input, const stateData &sD, double update[], const solverMode &sMode)
{
	if (!opFlags[differential_input])
	{
		auto offset = offsets.getAlgOffset(sMode) + limiter_alg;
		double ival = input + bias;
		update[offset] = K ;
		//printf("db %f intput=%f val=%f dbstate=%d\n", sD.time, ival, update[offset], static_cast<int>(dbstate));
		if (limiter_alg > 0)
		{
			return basicBlock::algElements(input, sD, update, sMode);
		}
	}
}

index_t blockSequence::findIndex(const std::string &field, const solverMode &sMode) const
{
	return kNullLocation;
}

void blockSequence::jacElements(double input, double didt, const stateData &sD, matrixData<double> &ad, index_t argLoc, const solverMode &sMode)
{
	if ((!opFlags[differential_input]) && (hasAlgebraic(sMode)))
	{
		auto offset = offsets.getAlgOffset(sMode) + limiter_alg;
		ad.assign(offset, offset, -1.0);
		double dido = K ;

		if (argLoc != kNullLocation)
		{
			ad.assign(offset, argLoc, dido);
		}
		if (limiter_alg > 0)
		{
			basicBlock::jacElements(input, didt, sD, ad, argLoc, sMode);
		}
	}
	else if ((opFlags[differential_input]) && (hasDifferential(sMode)))
	{
		auto offset = offsets.getDiffOffset(sMode) + limiter_diff;
		ad.assign(offset, offset, -sD.cj);
		double dido = K ;

		if (argLoc != kNullLocation)
		{
			ad.assign(offset, argLoc, dido * sD.cj);
		}

		if (limiter_diff > 0)
		{
			basicBlock::jacElements(input, didt, sD, ad, argLoc, sMode);
		}
	}
}


void blockSequence::rootTest(const IOdata &args, const stateData &sD, double root[], const solverMode &sMode)
{

	

}


void blockSequence::rootTrigger(gridDyn_time ttime, const IOdata &args, const std::vector<int> &rootMask, const solverMode &sMode)
{
	
}

change_code blockSequence::rootCheck(const IOdata &args, const stateData &sD, const solverMode &sMode, check_level_t /*level*/)
{
	change_code ret = change_code::no_change;
	
	return ret;
}

void blockSequence::setFlag(const std::string &flag, bool val)
{
	auto sfnd = flag.find_last_of(":?");
	if (sfnd != std::string::npos)
	{
		auto blk = findBlock(flag.substr(0, sfnd));
		if (blk)
		{
			blk->setFlag(flag.substr(sfnd + 1, std::string::npos), val);
		}
		else
		{
			throw(unrecognizedParameter());
		}
	}
	else if (flag == "differential_input")
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
	auto sfnd = param.find_last_of(":?");
	if (sfnd != std::string::npos)
	{
		auto blk = findBlock(param.substr(0, sfnd));
		if (blk)
		{
			blk->set(param.substr(sfnd + 1, std::string::npos), val);
		}
		else
		{
			throw(unrecognizedParameter());
		}
	}
	else if (param == "sequence")
	{

	}
	else
	{
		basicBlock::set(param, val);
	}

}

void blockSequence::set(const std::string &param, double val, gridUnits::units_t unitType)
{

	auto sfnd = param.find_last_of(":?");
	if (sfnd != std::string::npos)
	{
		auto blk = findBlock(param.substr(0, sfnd));
		if (blk)
		{
			blk->set(param.substr(sfnd + 1, std::string::npos), val,unitType);
		}
		else
		{
			throw(unrecognizedParameter());
		}
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
	auto blk = findBlock(blockname);
	if (blk)
	{
		return blk->getBlockOutput();
	}
	return kNullVal;
}

basicBlock *blockSequence::findBlock(const std::string &blockname) const
{
	if (blockname.compare(0, 6, "block#") == 0)
	{
		index_t b = numeric_conversion<index_t>(blockname.substr(7),kNullLocation);
		if ((b != kNullLocation) && (b < blocks.size()))
		{
			return blocks[b];
		}
	}
	else
	{
		for (auto &blk : blocks)
		{
			if (!blk)
			{
				continue;
			}
			if ((blockname == blk->getName())|| (blockname == blk->getOutputName()))
			{
				return blk;
			}
		}
	}
	return nullptr;
}