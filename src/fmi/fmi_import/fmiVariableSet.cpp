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

#include "fmiInfo.h"
#include <algorithm>

fmiVariableSet::fmiVariableSet() = default;

fmiVariableSet::fmiVariableSet(fmi2ValueReference newvr) :vrset({ newvr })
{

}
fmiVariableSet::fmiVariableSet(const fmiVariableSet &vset) = default;

fmiVariableSet::fmiVariableSet(fmiVariableSet &&vset) = default;


fmiVariableSet& fmiVariableSet::operator=(const fmiVariableSet & other) = default;


fmiVariableSet& fmiVariableSet::operator=(fmiVariableSet && other) = default;


const fmi2ValueReference *fmiVariableSet::getValueRef() const
{
	return vrset.data();
}

size_t fmiVariableSet::getVRcount() const
{
	return vrset.size();
}

fmi_variable_type_t fmiVariableSet::getType() const
{
	return type.value();
}

void fmiVariableSet::push(fmi2ValueReference newvr)
{
	vrset.push_back(newvr);
}

void fmiVariableSet::push(const fmiVariableSet &vset)
{
	vrset.reserve(vset.vrset.size() + vrset.size());
	vrset.insert(vrset.end(), vset.vrset.begin(), vset.vrset.end());
}

void fmiVariableSet::reserve(size_t newSize)
{
	vrset.reserve(newSize);
}

void fmiVariableSet::clear()
{
	vrset.clear();
}

void fmiVariableSet::remove(fmi2ValueReference rmvr)
{
	auto rm=std::remove(vrset.begin(), vrset.end(), rmvr);
	vrset.erase(rm, vrset.end());

}