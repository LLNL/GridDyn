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

#ifndef TX_LIFESPAN_H_
#define TX_LIFESPAN_H_

#include "relays/sensor.h"


/** @brief class modeling a transformer lifespan based on thermal effects
*/
class txLifeSpan : public sensor
{
public:
	enum lifespan_model_flags
	{
		useIECmethod= object_flag11,
		no_disconnect=object_flag12,  //!< flag indicating that the object should create a short circuit instead of disconnecting when life reaches 0
	};
protected:
	double initialLife=150000.0;  //!< initial life in hours
	double agingConstant = 14594.0;  //!< aging constant default value of 14594 based on research 15000 is another commonly used value
	double baseTemp = 110;		//!< the temperature base for the lifespan equations
	double agingFactor = 1.0;	//!<  factor for accelerated or decelerated aging based on insulation properties

private:
	double Faa = 0.0;
public:
	txLifeSpan(const std::string &objName="txlifeSpan_$");
	gridCoreObject * clone(gridCoreObject *obj=nullptr) const override;
	virtual int setFlag(const std::string &flag, bool val=true) override;
	virtual int set (const std::string &param, const std::string &val) override;

	virtual int set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;
	virtual int add(gridCoreObject *obj) override final;
	virtual double get(const std::string & param, gridUnits::units_t unitType = gridUnits::defUnit) const override;

	virtual void dynObjectInitializeA (double time0, unsigned long flags) override;
	virtual void dynObjectInitializeB(IOdata &outputSet) override;

	virtual double timestep(double ttime, const solverMode &sMode) override;
	virtual void updateA(double time) override;

	void actionTaken(index_t conditionNum, index_t ActionNum, change_code actionReturn, double /*actionTime*/) override;
};


#endif
