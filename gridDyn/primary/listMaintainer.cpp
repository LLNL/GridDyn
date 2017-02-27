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

#include "gridArea.h"
#include "listMaintainer.h"

void fillList(const solverMode &sMode, std::vector<gridPrimary *> &list, std::vector<gridPrimary *> &partlist, const std::vector<gridPrimary *> &possObj);

listMaintainer::listMaintainer(): objectLists(4),partialLists(4),sModeLists(4)
{

}

	void listMaintainer::makeList(const solverMode &sMode, const std::vector<gridPrimary *> &possObjs)
	{
		if (sMode.offsetIndex >= objectLists.size())
		{
			objectLists.resize(sMode.offsetIndex + 1);
			partialLists.resize(sMode.offsetIndex + 1);
			sModeLists.resize(sMode.offsetIndex + 1);
			sModeLists[sMode.offsetIndex] = sMode;
			objectLists[sMode.offsetIndex].reserve(possObjs.size());
			partialLists[sMode.offsetIndex].reserve(possObjs.size());
		}
		objectLists[sMode.offsetIndex].clear();
		partialLists[sMode.offsetIndex].clear();
		fillList(sMode, objectLists[sMode.offsetIndex], partialLists[sMode.offsetIndex],possObjs);
		sModeLists[sMode.offsetIndex] = sMode;
	}

	void listMaintainer::appendList(const solverMode &sMode, const std::vector<gridPrimary *> &possObjs)
	{
		if (sMode.offsetIndex >= objectLists.size())
		{
			objectLists.resize(sMode.offsetIndex + 1);
			partialLists.resize(sMode.offsetIndex + 1);
			sModeLists.resize(sMode.offsetIndex + 1);
			sModeLists[sMode.offsetIndex] = sMode;
			objectLists[sMode.offsetIndex].reserve(possObjs.size());
			partialLists[sMode.offsetIndex].reserve(possObjs.size());
		}
		fillList(sMode, objectLists[sMode.offsetIndex], partialLists[sMode.offsetIndex], possObjs);
	}

	void fillList(const solverMode &sMode, std::vector<gridPrimary *> &list, std::vector<gridPrimary *> &partlist,const std::vector<gridPrimary *> &possObjs)
	{
		for (auto &obj : possObjs)
		{
			if (obj->checkFlag(preEx_requested))
			{
				if (obj->checkFlag(multipart_calculation_capable))
				{
					partlist.push_back(obj);
					list.push_back(obj);
				}
				else if (obj->stateSize(sMode) > 0)
				{
					list.push_back(obj);
				}
			}
			else if(obj->stateSize(sMode) > 0)
			{
				partlist.push_back(obj);
				list.push_back(obj);
			}
		}
	}

	void listMaintainer::makePreList(const std::vector<gridPrimary *> &possObjs)
	{
		preExObjs.clear();
		for (auto &obj : possObjs)
		{
			if (obj->checkFlag(preEx_requested))
			{
				preExObjs.push_back(obj);
			}
		}
	}

	void listMaintainer::preEx(const IOdata &inputs, const stateData &sD, const solverMode &sMode)
	{
		for (auto &obj : preExObjs)
		{
			obj->preEx(inputs,sD, sMode);
		}
	}

	void listMaintainer::jacobianElements(const IOdata &inputs, const stateData &sD, matrixData<double> &ad, const IOlocs &inputLocs, const solverMode &sMode)
	{
		if (!isListValid(sMode))
		{
			return;
		}
#ifdef HAVE_OPENMP
		if (parJac)
		{
			auto &vz = partialLists[sMode.offsetIndex];
			int sz = static_cast<int>(vz.size());
#pragma omp parallel for
			for (int kk = 0; kk < sz; ++kk)
			{
				vz[kk]->jacobianElements(inputs, sD, ad,inputLocs, sMode);
			}
		}
		else
		{
			for (auto &obj : partialLists[sMode.offsetIndex])
			{
				obj->jacobianElements(inputs, sD, ad,inputLocs, sMode);
			}
		}

#else
		for (auto &obj : partialLists[sMode.offsetIndex])
		{
			obj->jacobianElements(inputs, sD, ad,inputLocs, sMode);
		}
#endif
		
	}

	void listMaintainer::residual(const IOdata &inputs, const stateData &sD, double resid[], const solverMode &sMode)
	{
		if (!isListValid(sMode))
		{
			return;
		}
		
#ifdef HAVE_OPENMP
		if (parResid)
		{
			auto &vz = partialLists[sMode.offsetIndex];
			int sz = static_cast<int>(vz.size());
			#pragma omp parallel for
			for (int kk = 0; kk < sz; ++kk)
			{
				vz[kk]->residual(inputs, sD, resid, sMode);
			}
		}
		else
		{
			for (auto &obj : partialLists[sMode.offsetIndex])
			{
				obj->residual(inputs, sD, resid, sMode);
			}
		}
		
#else
		for (auto &obj : partialLists[sMode.offsetIndex])
		{
			obj->residual(inputs, sD, resid, sMode);
		}
#endif
		
		/*
		
		*/
	}

	void listMaintainer::algebraicUpdate(const IOdata &inputs, const stateData &sD, double update[], const solverMode &sMode, double alpha)
	{
		if (!isListValid(sMode))
		{
			return;
		}
		for (auto &obj : partialLists[sMode.offsetIndex])
		{
			obj->algebraicUpdate(inputs, sD, update, sMode,alpha);
		}
	}

	void listMaintainer::derivative(const IOdata &inputs, const stateData &sD, double deriv[], const solverMode &sMode)
	{
		if (!isListValid(sMode))
		{
			return;
		}
#ifdef HAVE_OPENMP
		if (parResid)
		{
			auto &vz = partialLists[sMode.offsetIndex];
			int sz = static_cast<int>(vz.size());
#pragma omp parallel for
			for (int kk = 0; kk < sz; ++kk)
			{
				vz[kk]->derivative(inputs, sD, deriv, sMode);
			}
		}
		else
		{
			for (auto &obj : partialLists[sMode.offsetIndex])
			{
				obj->derivative(inputs, sD, deriv, sMode);
			}
		}

#else
		for (auto &obj : partialLists[sMode.offsetIndex])
		{
			obj->derivative(inputs, sD, deriv, sMode);
		}
#endif
	}


	void listMaintainer::delayedResidual(const IOdata &inputs, const stateData &sD, double resid[], const solverMode &sMode)
	{
		for (auto &obj : preExObjs)
		{
			obj->delayedResidual(inputs, sD, resid, sMode);
		}
	}
	void listMaintainer::delayedDerivative(const IOdata &inputs, const stateData &sD, double deriv[], const solverMode &sMode)
	{
	
		for (auto &obj : preExObjs)
		{
			obj->delayedDerivative(inputs, sD, deriv, sMode);
		}
	}

	void listMaintainer::delayedJacobian(const IOdata &inputs, const stateData &sD, matrixData<double> &ad,const IOlocs &inputLocs, const solverMode &sMode)
	{
		for (auto &obj : preExObjs)
		{
			obj->delayedJacobian(inputs, sD, ad,inputLocs, sMode);
		}
	}

	void listMaintainer::delayedAlgebraicUpdate(const IOdata &inputs, const stateData &sD, double update[], const solverMode &sMode, double alpha)
	{
		for (auto &obj : preExObjs)
		{
			obj->delayedAlgebraicUpdate(inputs, sD, update, sMode, alpha);
		}
	}

	bool listMaintainer::isListValid(const solverMode &sMode) const
	{
		if (sMode.offsetIndex > objectLists.size())
		{
			return false;
		}
		return (sModeLists[sMode.offsetIndex].offsetIndex!=kNullLocation);
	}

	void listMaintainer::invalidate(const solverMode &sMode)
	{
		if (sMode.offsetIndex < objectLists.size())
		{
			sModeLists[sMode.offsetIndex] = solverMode();
		}
	}

	void listMaintainer::invalidate()
	{
		for (index_t kk=0;kk<sModeLists.size();++kk)
		{
			sModeLists[kk] = solverMode();
		}
	}

	decltype(listMaintainer::objectLists[0].begin()) listMaintainer::begin(const solverMode &sMode)
	{
		if (!isListValid(sMode))
		{
			return objectLists[0].end();
		}
		return objectLists[sMode.offsetIndex].begin();
	}

	decltype(listMaintainer::objectLists[0].end()) listMaintainer::end(const solverMode &sMode)
	{
		if (!isListValid(sMode))
		{
			return objectLists[0].end();
		}
		return objectLists[sMode.offsetIndex].end();
	}

	decltype(listMaintainer::objectLists[0].cbegin()) listMaintainer::cbegin(const solverMode &sMode) const
	{
		if (!isListValid(sMode))
		{
			return objectLists[0].cend();
		}
		return objectLists[sMode.offsetIndex].cbegin();
	}

	decltype(listMaintainer::objectLists[0].cend()) listMaintainer::cend(const solverMode &sMode) const
	{
		if (!isListValid(sMode))
		{
			return objectLists[0].cend();
		}
		return objectLists[sMode.offsetIndex].cend();
	}

	decltype(listMaintainer::objectLists[0].rbegin()) listMaintainer::rbegin(const solverMode &sMode)
	{
		if (!isListValid(sMode))
		{
			return objectLists[0].rend();
		}
		return objectLists[sMode.offsetIndex].rbegin();
	}

	decltype(listMaintainer::objectLists[0].rend()) listMaintainer::rend(const solverMode &sMode)
	{
		if (!isListValid(sMode))
		{
			return objectLists[0].rend();
		}
		return objectLists[sMode.offsetIndex].rend();
	}