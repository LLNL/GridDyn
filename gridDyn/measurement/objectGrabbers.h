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

#ifndef OBJECT_GRABBERS_H_
#define OBJECT_GRABBERS_H_
#include "measurement/gridGrabbers.h"
#include "core/helperTemplates.h"
#include "core/objectInterpreter.h"
#include "solvers/solverMode.h"
#include "core/coreExceptions.h"

class gridSubModel;
class gridObject;
class gridBus;
class gridLoad;
class gridLink;
class gridDynGenerator;
class gridArea;
class gridRelay;

using fobjectPair=std::pair<std::function<double(coreObject *)>, gridUnits::units_t>;

fobjectPair getObjectFunction(gridObject *obj, const std::string &field);
 fobjectPair getObjectFunction(gridBus *bus, const std::string &field);
 fobjectPair getObjectFunction(gridLoad *ld, const std::string &field);
fobjectPair getObjectFunction(gridLink *lnk, const std::string &field);
fobjectPair getObjectFunction(gridDynGenerator *gen, const std::string &field);
fobjectPair getObjectFunction(gridArea *area, const std::string &field);
fobjectPair getObjectFunction(gridRelay *rel, const std::string &field);

using fvecPair=std::pair<std::function<void(coreObject *, std::vector<double> &)>, gridUnits::units_t> ;
 
fvecPair getObjectVectorFunction(gridObject *obj, const std::string &field);
 
 fvecPair getObjectVectorFunction(gridArea *area, const std::string &field);


 using descVecFunc = std::function<void(coreObject *, stringVec &)> ;

descVecFunc getObjectVectorDescFunction(gridObject *obj, const std::string &field);
 descVecFunc getObjectVectorDescFunction(gridArea *area, const std::string &field);

 const std::string objEmptyString("");

template <class X>
class objectGrabber:public gridGrabber
{
protected:
	X *tobject; //!< a class specific object pointer
public:
	objectGrabber(const std::string &fld= objEmptyString, X *newObj=nullptr)
	{
		if (newObj)
		{
			updateObject(newObj);
		}
		if (!fld.empty())
		{
			objectGrabber<X>::updateField(fld);
		}
	}
	std::shared_ptr<gridGrabber> clone(std::shared_ptr<gridGrabber> ggb = nullptr) const override
	{
		auto ngb = cloneBase<objectGrabber, gridGrabber>(this, ggb);
		if (!ngb)
		{
			return ggb;
		}

		ngb->tobject = tobject;

		return ngb;
	}

	void updateField(const std::string &fld) override
	{
		field = fld;
		auto fret = getObjectFunction(tobject,fld);
		if (fret.first)
		{
			fptr = fret.first;
			inputUnits = fret.second;
			loaded=checkIfLoaded();
			return;
		}
		auto fvecret = getObjectVectorFunction(tobject,fld);
		if (fvecret.first)
		{
			fptrV = fvecret.first;
			inputUnits = fvecret.second;
			vectorGrab = true;
			fptrN=getObjectVectorDescFunction(tobject,fld);
			loaded = checkIfLoaded();
			return;
		}
		gridGrabber::updateField(fld);
	}

	void updateObject(coreObject *obj, object_update_mode mode = object_update_mode::direct) override
	{
		coreObject *newObject = (mode == object_update_mode::direct) ? obj : findMatchingObject(cobj, obj);
		if (dynamic_cast<X *> (newObject))
		{
			tobject = static_cast<X *> (newObject);
			gridGrabber::updateObject(newObject);
		}
		else
		{
			throw(objectUpdateFailException());
		}
	}
};

template <class X>
class objectOffsetGrabber :public gridGrabber
{
protected:
	X *tobject;
	index_t offset = kInvalidLocation;
public:
	objectOffsetGrabber(const std::string &fld= objEmptyString, X *newObj=nullptr)
	{
		if (newObj)
		{
			updateObject(newObj);
		}
		if (!fld.empty())
		{
			objectOffsetGrabber<X>::updateField(fld);
		}
	}
	objectOffsetGrabber(index_t newOffset, X *newObj=nullptr)
	{
		if (newObj)
		{
			updateObject(newObj);
		}
		
		updateOffset(newOffset);
	}
	std::shared_ptr<gridGrabber> clone(std::shared_ptr<gridGrabber> ggb = nullptr) const override
	{
		auto ngb = cloneBase<objectOffsetGrabber, gridGrabber>(this, ggb);
		if (!ngb)
		{
			return ggb;
		}

		ngb->tobject = tobject;
		ngb->offset = offset;
		return ngb;
	}

	void updateField(const std::string &fld) override
	{
		field = fld;
		auto fret = getObjectFunction(tobject, fld);
		if (fret.first)
		{
			fptr = fret.first;
			inputUnits = fret.second;
			loaded = gridGrabber::checkIfLoaded();
			return;
		}
		auto fvecret = getObjectVectorFunction(tobject, fld);
		if (fvecret.first)
		{
			fptrV = fvecret.first;
			inputUnits = fvecret.second;
			vectorGrab = true;
			fptrN = getObjectVectorDescFunction(tobject, fld);
			loaded = gridGrabber::checkIfLoaded();
			return;
		}
		offset = tobject->findIndex(fld, cLocalbSolverMode);
		
		if (offset == kInvalidLocation)
		{
			gridGrabber::updateField(fld);
		}
		else
		{
			loaded = true;
			makeDescription();
			inputUnits = gridUnits::defUnit;
		}
	}

	void updateObject(coreObject *obj, object_update_mode mode = object_update_mode::direct) override
	{
		coreObject *newObject = (mode == object_update_mode::direct) ? obj : findMatchingObject(cobj, obj);
		if (dynamic_cast<X *> (newObject))
		{
			tobject = static_cast<X *> (newObject);
			if (offset == kInvalidLocation)
			{
				gridGrabber::updateObject(newObject);
			}
			else
			{
				offset = tobject->findIndex(field, cLocalbSolverMode);

				if (offset == kInvalidLocation)
				{
					gridGrabber::updateField(field);
				}
				else
				{
					loaded = true;
					makeDescription();
					inputUnits = gridUnits::defUnit;
				}
			}
		}
		else
		{
			throw(objectUpdateFailException());
		}
	}

	void updateOffset(index_t nOffset)
	{
		offset = nOffset;
		if (tobject)
		{
			if (offset < tobject->stateSize(cLocalSolverMode))
			{
				loaded = true;
				if (!customDesc)
				{
					desc = tobject->getName() + ':' + std::to_string(nOffset);
				}
				
				return;
			}
		}
		loaded = false;
	}

	double grabData() override
	{
		double val = kNullVal;
		if (loaded)
		{
			if (offset != kInvalidLocation)
			{
				if (offset == kNullLocation)
				{
					offset = tobject->findIndex(field, cLocalbSolverMode);
				}
				if (offset != kNullLocation)
				{
					val = tobject->getState(offset);
				}
				else
				{
					val = kNullVal;
				}
				val = val * gain + bias;
			}
			else
			{
				val = gridGrabber::grabData();
			}
		}
		return val;
	}

	void makeDescription() const override
	{
		if (!customDesc)
		{
			if ((loaded) && (field.empty()))
			{
				desc = tobject->getName() + ':' + std::to_string(offset);
			}
			else
			{
				gridGrabber::makeDescription();
			}

		}
		
	}

	bool checkIfLoaded() override
	{
		//check for the offset, otherwise just use the regular check
		if (offset != kInvalidLocation)
		{
			return (cobj!=nullptr);
		}
		return gridGrabber::checkIfLoaded();
	}
	
};
#endif