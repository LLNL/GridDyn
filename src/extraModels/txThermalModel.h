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

#ifndef TX_THERMAL_MODEL_H_
#define TX_THERMAL_MODEL_H_

#include "relays/sensor.h"

namespace griddyn {
namespace extra {
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
	virtual coreObject * clone(coreObject *obj=nullptr) const override;
	virtual void setFlag(const std::string &flag, bool val=true) override;
	virtual void set(const std::string &param, const std::string &val) override;

	virtual void set(const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;
	using sensor::add;
	virtual void add(coreObject *obj) override final;
	virtual double get(const std::string & param, gridUnits::units_t unitType = gridUnits::defUnit) const override;

	virtual void dynObjectInitializeA (coreTime time0, std::uint32_t flags) override;
	virtual void dynObjectInitializeB (const IOdata &inputs, const IOdata & desiredOutput, IOdata &fieldSet) override;

	virtual void timestep (coreTime time, const IOdata &inputs, const solverMode &sMode) override;
	virtual void updateA(coreTime time) override;
};

}//namespace extra
}//namespace griddyn
#endif