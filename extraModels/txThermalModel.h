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

#ifndef TX_THERMAL_MODEL_H_
#define TX_THERMAL_MODEL_H_

#include "relays/sensor.h"


/** @brief basic thermal model of a transformer
*/
class txThermalModel : public sensor
{
public:
	enum thermal_model_flags
	{
		auto_parameter_load = object_flag10,
		enable_parameter_updates = object_flag11,
		enable_alarms = object_flag12,
	};
protected:
	double Ttor = 1.25*3600.0; //!<[s] oil rise time constant
	double DThs = 35.0; //!<[C] hot spot rise temp at rated current over top oil
	double DTtor = 45.0; //!<[C] Oil rise temp at rated current
	double Tgr = 5.0*60.0; //!<[s] winding time constant
	double mp_LR = 6.5; //!< Loss Ratio
	double mp_n = 1.0; //!<oil exponent
	double mp_m = 1.0; //!< winding exponent
	double ambientTemp = 20;  //!<[C] ambient temperature in C
	double dTempdt = 0.0;  //!<[C/s] rate of change of ambient temperature
	double alarmTemp1 = 0; //!<[C] the lower alarm temp
	double alarmTemp2 = 0;  //!<[C] the level 2 alarm temp
	double cutoutTemp = 0;  //!<[C] the temp at which the breakers are tripped
	double alarmDelay = 300;  //!<[s] delay time on the alarms and cutout;
private:
	double rating;  //!< transformer rating
	double Plossr;  //!<  the losses at rated power
	double m_C;   //!< transformer thermal capacity
	double m_k;   //!< transformer radiation constant
public:
	/** @brief constructor*/
	txThermalModel(const std::string &objName="txThermal_$");
	virtual gridCoreObject * clone(gridCoreObject *obj=nullptr) const override;
	virtual int setFlag(const std::string &param, bool val=true) override;
	virtual int set(const std::string &param, const std::string &val) override;

	virtual int set(const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;
	virtual int add(gridCoreObject *obj) final override;
	virtual double get(const std::string & param, gridUnits::units_t unitType = gridUnits::defUnit) const override;

	virtual void dynObjectInitializeA (double time0, unsigned long flags) override;
	virtual void dynObjectInitializeB (IOdata &outputSet) override;

	virtual double timestep(double ttime, const solverMode &sMode) override;
	virtual void updateA(double time) override;
};


#endif