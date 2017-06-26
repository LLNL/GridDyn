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

#ifndef GRIDLOAD3PHASE_H_
#define GRIDLOAD3PHASE_H_

#include "Load.h"
namespace griddyn
{
class gridBus;
namespace loads
{
enum class phase_type_t
{
	abc,
	pnz,
};
/** primary load class supports 3 main types of loads  constant power, constant impedance, constant current
these loads should for the basis of most non dynamic load models following the ZIP model Z-constant impedance,
I-constant current, P- constant Power
*/
class ThreePhaseLoad : public Load
{
public:
	enum load_flags
	{
		use_abs_angle = object_flag5,
	};
private:
	double Pa=0.0;  //!<[pu] A Phase real power
	double Pb=0.0; //!<[pu] B Phase real power
	double Pc=0.0; //!<[pu] C Phase real power
	double Qa=0.0; //!<[pu] A Phase reactive power
	double Qb=0.0; //!<[pu] B Phase reactive power
	double Qc=0.0; //!<[pu] C Phase reactive power
	double multiplier = 1.0; //!< phase multiplier for amplifying current inputs
public:
	explicit ThreePhaseLoad(const std::string &objName = "load_$");
	ThreePhaseLoad(double rP, double rQ, const std::string &objName = "load_$");


	virtual coreObject * clone(coreObject *obj = nullptr) const override;

	virtual void pFlowObjectInitializeA(coreTime time0, std::uint32_t flags) override;

	virtual void getParameterStrings(stringVec &pstr, paramStringType pstype) const override;

	virtual void set(const std::string &param, const std::string &val) override;
	virtual void set(const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;
	virtual void setFlag(const std::string &flag, bool val = true) override;

	virtual double get(const std::string &param, gridUnits::units_t unitType = gridUnits::defUnit) const override;

	/** set the real output power with the specified units
	@param[in] level the real power output setting
	@param[in] unitType the units on the real power
	*/
	virtual void setLoad(double level, gridUnits::units_t unitType = gridUnits::defUnit) override;
	/** set the real and reactive output power with the specified units
	@param[in] Plevel the real power output setting
	@param[in] Qlevel the reactive power output setting
	@param[in] unitType the units on the real power
	*/
	virtual void setLoad(double Plevel, double Qlevel, gridUnits::units_t unitType = gridUnits::defUnit) override;
                                                                                                        //for saving the state
	virtual IOdata getRealPower3Phase(const IOdata &inputs, const stateData &sD, const solverMode &sMode, phase_type_t type=phase_type_t::abc) const;
	virtual IOdata getReactivePower3Phase(const IOdata &inputs, const stateData &sD, const solverMode &sMode, phase_type_t type = phase_type_t::abc) const;
	/** get the 3 phase real output power that based on the given voltage
	@param[in] V the bus voltage
	@return the real power consumed by the load*/
	virtual IOdata getRealPower3Phase(const IOdata &V, phase_type_t type = phase_type_t::abc) const;
	/** get the 3 phase reactive output power that based on the given voltage
	@param[in] V the bus voltage
	@return the reactive power consumed by the load*/
	virtual IOdata getReactivePower3Phase(const IOdata &V, phase_type_t type = phase_type_t::abc) const;
	virtual IOdata getRealPower3Phase(phase_type_t type = phase_type_t::abc) const;
	virtual IOdata getReactivePower3Phase(phase_type_t type = phase_type_t::abc) const;

	void setPa(double val);
	void setPb(double val);
	void setPc(double val);
	void setQa(double val);
	void setQb(double val);
	void setQc(double val);
private:
	double getBaseAngle() const;

};
}//namespace loads
}//namespace griddyn
#endif

