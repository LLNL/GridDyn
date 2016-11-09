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

#include "fmiInfo.h"


fmiVariableSet::fmiVariableSet()
{

}

fmiVariableSet::fmiVariableSet(fmi2ValueReference newvr):cnt(1), vr(newvr)
{

}
fmiVariableSet::fmiVariableSet(const fmiVariableSet &vset):cnt(vset.cnt),vr(vset.vr),vrset(vset.vrset)
{

}
fmiVariableSet::fmiVariableSet(fmiVariableSet &&vset) : cnt(vset.cnt), vr(vset.vr)
{
	if (cnt > 1)
	{
		vrset = std::move(vset.vrset);
	}
}


fmiVariableSet& fmiVariableSet::operator=(const fmiVariableSet & other)
{
	cnt = other.cnt;
	vrset = other.vrset;
	vr = other.vr;
	return *this;
}

fmiVariableSet& fmiVariableSet::operator=(fmiVariableSet && other)
{
	cnt = other.cnt;
	vr = other.vr;
	if (cnt > 1)
	{
		vrset = std::move(other.vrset);
	}
	return *this;
}


const fmi2ValueReference *fmiVariableSet::getValueRef() const
{
	return (cnt <= 1) ? &vr : vrset.data();
}

size_t fmiVariableSet::getVRcount() const
{
	return cnt;
}

fmi_type_t fmiVariableSet::getType() const
{
	return type;
}

void fmiVariableSet::push(fmi2ValueReference newvr)
{
	switch (cnt)
	{
	case 0:
		vr = newvr;
		break;
	case 1: //purposeful fall through
		vrset.push_back(vr);
	default:
		vrset.push_back(newvr);
		break;
	}
	++cnt;
}

void fmiVariableSet::push(const fmiVariableSet &vset)
{
	switch (vset.cnt)
	{
	case 0:
		return;
	case 1:
		push(vset.vr);
		break;
	default:
		switch (cnt)
		{
		case 0:
			vrset = vset.vrset;
			cnt = vset.cnt;
			break;
		case 1:  //purposeful fall through
			vrset.push_back(vr);
		default:
			vrset.insert(vrset.end(), vset.vrset.begin(), vset.vrset.end());
			cnt = vrset.size();
			break;
		}
		break;
	}
}

void fmiVariableSet::setSize(size_t newSize)
{
	vrset.reserve(newSize);
}

void fmiVariableSet::clear()
{
	vrset.clear();
	cnt = 0;
}

void fmiVariableSet::remove(fmi2ValueReference rmvr)
{
	switch (cnt)
	{
	case 0:
		return;
	case 1:
		if (vr == rmvr)
		{
			cnt = 0;
		}
		break;
	case 2:
		if (vrset[0] == rmvr)
		{
			vr = vrset[1];
			vrset.resize(0);
			cnt = 1;
		}
		else if (vrset[1] == rmvr)
		{
			vr = vrset[0];
			vrset.resize(0);
			cnt = 1;
		}
		break;
	default:
	{
		auto beg = vrset.begin();
		auto vrend = vrset.end();
		while (beg != vrend)
		{
			if (*beg == rmvr)
			{
				vrset.erase(beg);
				--cnt;
				return;
			}
			++beg;
		}
	}
	break;
	}

}