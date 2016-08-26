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

#ifndef GRIDLOAD_H_
#define GRIDLOAD_H_

#include "gridObjects.h"

class gridBus;

/** primary load class supports 3 main types of loads  constant power, constant impedance, constant current
these loads should for the basis of most non dynamic load models following the ZIP model Z-constant impedance,
I-constant current, P- constant Power
*/
class gridLoad : public gridSecondary
{
public:
  enum load_flags
  {
    use_power_factor_flag = object_flag1,
    convert_to_constant_impedance = object_flag2,
    no_pqvoltage_limit = object_flag3,
  };
  static int loadCount;      //!<counter for automatic load id's
  double baseVoltage;        //!< base voltage of the load
protected:
  gridBus *bus = nullptr;					//!< pointer to the parent bus
  double Pout;									//!<[puMW] the actual output power
  double Qout;									//!<[puMVA] the actual output power
  double Psched = 0.0;							//!<[puMW] the scheduled output power

  double dPdf = 0.0;                            //!<factor for determining how sensitive Pout is to frequency
  double P = 0.0;                                     //!< [pu] real component of the load (constant Power)
  double Q = 0.0;                                     //!< [pu] imaginary component of the load (constant Power)
  double r = kBigNum;                           //!< [pu] resistive load (constant impedance)
  double x = 0.0;                               //!< [pu] reactive load (constant impedance)
  double Ip = 0.0;                              //!< [pu] real current; (constant current)
  double Iq = 0.0;                              //!< [pu] imaginary current (constant current)
  double Yp = 0.0;                              //!< [pu] the impedance load in MW
  double Yq = 0.0;                              //!< [pu]  the reactive impedance load in MVar
  double M = 0.0;                               //!<load droop factor
  double H = 0.0;                               //!<load inertia used in computing dPdf
  double pfq = 0.0;                             //!<power factor multiply  sqrt((1-pf*pf)/pf*pf)

  double Vpqmin = 0.7;         //!<low voltage at which the PQ powers convert to an impedance type load
  double Vpqmax = 1.3;        //!<upper voltage at which the PQ powers convert to an impedance type load
  double lastTime = 0;        //!<last time for an update
private:
  double trigVVlow = 1.0 / (0.7 * 0.7);       //!< constant for conversion of PQ loads to constant impedance loads
  double trigVVhigh = 1.0 / (1.3 * 1.3);       //!< constant for conversion of PQ loads to constant impedance loads
public:
  gridLoad (const std::string &objName = "load_$");
  gridLoad (double rP, double rQ, const std::string &objName = "load_$");

  virtual ~gridLoad ();
  virtual gridCoreObject * clone (gridCoreObject *obj = nullptr) const override;
  virtual void pFlowObjectInitializeA (double time0, unsigned long flags) override;

  virtual void dynObjectInitializeA (double time0, unsigned long flags) override;

  virtual double timestep (double ttime, const IOdata &args, const solverMode &sMode) override;
  virtual void setTime (double time) override;
  virtual void getParameterStrings (stringVec &pstr, paramStringType pstype) const override;

  virtual int set (const std::string &param,  const std::string &val) override;
  virtual int set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;
  virtual int setFlag (const std::string &flag, bool val = true) override;

  virtual double get (const std::string &param, gridUnits::units_t unitType = gridUnits::defUnit) const override;


  virtual void setLoad (double level, gridUnits::units_t unitType = gridUnits::defUnit);

  virtual void setLoad (double Plevel, double Qlevel, gridUnits::units_t unitType = gridUnits::defUnit);

  virtual void loadUpdate (double ttime)
  {
    lastTime = ttime;
  }
  virtual void loadUpdateForward (double ttime)
  {
    loadUpdate (ttime);
    prevTime = ttime;
  }
  virtual IOdata getOutputs (const IOdata &args, const stateData *sD, const solverMode &sMode) override;

  virtual void ioPartialDerivatives (const IOdata &args, const stateData *sD, arrayData<double> *ad, const IOlocs &argLocs, const solverMode &sMode) override;
  virtual void outputPartialDerivatives  (const IOdata &args, const stateData *sD, arrayData<double> *ad, const solverMode &sMode) override;

  virtual double getschedPower () const
  {
    return Psched;
  }
  virtual double getRealPower (const IOdata &args, const stateData *sD, const solverMode &sMode) override;
  virtual double getReactivePower (const IOdata &args, const stateData *sD, const solverMode &sMode) override;
  virtual double getRealPower (double V) const;
  virtual double getReactivePower (double V) const;
  virtual double getRealPower () const override;
  virtual double getReactivePower () const override;

  virtual void setState (double ttime, const double state[], const double dstate_dt[], const solverMode &sMode) override;                                                                                                                                //for saving the state

  double getdPdf ()
  {
    return dPdf;
  }
  friend bool compareLoad (gridLoad *ld1, gridLoad *ld2, bool printDiff);
private:
  void constructionHelper ();
  void checkFaultChange ();
};


bool compareLoad (gridLoad *ld1, gridLoad *ld2, bool printDiff = false);
#endif
