/*
 * LLNS Copyright Start
 * Copyright (c) 2014-2018, Lawrence Livermore National Security
 * This work was performed under the auspices of the U.S. Department
 * of Energy by Lawrence Livermore National Laboratory in part under
 * Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
 * Produced at the Lawrence Livermore National Laboratory.
 * All rights reserved.
 * For details, see the LICENSE file.
 * LLNS Copyright End
 */

#pragma once

#include "../Load.h"

namespace griddyn
{
/** primary load class supports 3 main types of loads  constant power, constant impedance, constant current
these loads should for the basis of most non dynamic load models following the ZIP model Z-constant impedance,
I-constant current, P- constant Power
*/
class zipLoad : public Load
{
  public:
    enum load_flags
    {
        convert_to_constant_impedance = object_flag2,
        no_pqvoltage_limit = object_flag3,
    };

  private:
    parameter_t Ip = 0.0;  //!< [pu] real current; (constant current)
    parameter_t Iq = 0.0;  //!< [pu] imaginary current (constant current)
    parameter_t Yp = 0.0;  //!< [pu] the impedance load in MW
    parameter_t Yq = 0.0;  //!< [pu]  the reactive impedance load in MVar
  protected:
    double Pout = 0.0;  //!<[puMW] the actual output power
    double Qout = 0.0;  //!<[puMVA] the actual output power
    // double Psched = 0.0;							//!<[puMW] the scheduled output power

    double Vpqmin = 0.7;  //!< low voltage at which the PQ powers convert to an impedance type load
    double Vpqmax = 1.3;  //!< upper voltage at which the PQ powers convert to an impedance type load
    coreTime lastTime = negTime;

  private:
    double trigVVlow = 1.0 / (0.7 * 0.7);  //!< constant for conversion of PQ loads to constant impedance loads
    double trigVVhigh = 1.0 / (1.3 * 1.3);  //!< constant for conversion of PQ loads to constant impedance loads
  public:
    explicit zipLoad (const std::string &objName = "zip_$");
    zipLoad (double rP, double rQ, const std::string &objName = "zip_$");

    virtual coreObject *clone (coreObject *obj = nullptr) const override;
    virtual void pFlowObjectInitializeA (coreTime time0, std::uint32_t flags) override;

    virtual void dynObjectInitializeA (coreTime time0, std::uint32_t flags) override;

    virtual void timestep (coreTime time, const IOdata &inputs, const solverMode &sMode) override;
    virtual void getParameterStrings (stringVec &pstr, paramStringType pstype) const override;

    virtual void set (const std::string &param, const std::string &val) override;
    virtual void
    set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;
    virtual void setFlag (const std::string &flag, bool val = true) override;

    virtual double get (const std::string &param, gridUnits::units_t unitType = gridUnits::defUnit) const override;

    virtual void updateLocalCache (const IOdata &inputs, const stateData &sD, const solverMode &sMode) override;

    virtual void
    setState (coreTime time, const double state[], const double dstate_dt[], const solverMode &sMode) override;

    virtual void ioPartialDerivatives (const IOdata &inputs,
                                       const stateData &sD,
                                       matrixData<double> &md,
                                       const IOlocs &inputLocs,
                                       const solverMode &sMode) override;
    virtual void outputPartialDerivatives (const IOdata &inputs,
                                           const stateData &sD,
                                           matrixData<double> &md,
                                           const solverMode &sMode) override;
    virtual count_t outputDependencyCount (index_t num, const solverMode &sMode) const override;

    virtual double
    getRealPower (const IOdata &inputs, const stateData &sD, const solverMode &sMode) const override;
    virtual double
    getReactivePower (const IOdata &inputs, const stateData &sD, const solverMode &sMode) const override;
    virtual double getRealPower (double V) const override;
    virtual double getReactivePower (double V) const override;
    virtual double getRealPower () const override;
    virtual double getReactivePower () const override;  // for saving the state

    friend bool compareLoad (zipLoad *ld1, zipLoad *ld2, bool printDiff);

  protected:
    // getters for the actual property values

    double getYp () const { return Yp; }
    double getYq () const { return Yq; }

    double getIp () const { return Ip; }

    double getIq () const { return Iq; }

    double getr () const;

    double getx () const;

    void setYp (double newYp);

    void setYq (double newYq);

    void setIp (double newIp);

    void setIq (double newIq);

    void setr (double newr);

    void setx (double newx);

    /** compute the voltage adjustment by min and max voltages*/
    double voltageAdjustment (double val, double V) const;
    /** get the Q from power factor*/
    double getQval () const;
};

bool compareLoad (zipLoad *ld1, zipLoad *ld2, bool printDiff = false);

}  // namespace griddyn
