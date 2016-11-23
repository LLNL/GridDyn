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

#ifndef AGCONTROL_H_
#define AGCONTROL_H_


#include "gridObjects.h"

class gridArea;
class schedulerReg;
class gridDynGenerator;
class battery;

class pidBlock;
class delayBlock;
class deadbandBlock;
class gridCommunicator;

class AGControl : public gridSubModel
{
public:
  enum agcType
  {
    basicAGC,
    batteryAGC,
    battDR,
  };
protected:
  double KI = 0.005;
  double KP = 1.0;
  double beta = 8.0;
  double deadband = 20;

  double Tf = 8.0;
  double Tr = 15;
  double ACE = 0;
  double fACE = 0;
  double freg = 0;
  double reg = 0;
  double regUpAvailable = 0;
  double regDownAvailable = 0;

  std::shared_ptr<pidBlock> pid;
  std::shared_ptr<delayBlock> filt1;
  std::shared_ptr<delayBlock> filt2;
  std::shared_ptr<deadbandBlock> db;

  count_t schedCount = 0;

  std::vector<schedulerReg *> schedList;
  std::vector<double> upRat;
  std::vector<double> downRat;
  std::shared_ptr<gridCommunicator> comms;
public:
  AGControl (const std::string &objName = "AGC_#");
  virtual gridCoreObject * clone (gridCoreObject *obj = nullptr) const override;
  virtual ~AGControl ();

  virtual void objectInitializeB (const IOdata &args, const IOdata &outputSet, IOdata &inputSet) override;


  void setTime (gridDyn_time time) override;
  virtual void updateA (gridDyn_time time) override;

  virtual void timestep (gridDyn_time ttime, const IOdata &args, const solverMode &sMode) override;

  double getOutput (index_t /*num*/ = 0) const override
  {
    return reg;
  }
  virtual void add (gridCoreObject *obj) override;
  virtual void add (schedulerReg *sched);
  virtual void remove (gridCoreObject *obj) override;
  virtual void set (const std::string &param,  const std::string &val) override;
  virtual void set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;

  double getACE ()
  {
    return ACE;
  }
  double getfACE ()
  {
    return fACE;
  }

  virtual void regChange ();

};

/*
class AGControlBattery:public AGControl
{
public:


protected:
        std::vector <battery *> batList;
        std::vector<int> isBat;
        std::vector<double> batUpRat;
        std::vector<double> batDownRat;
        std::vector<double> genSched;
        size_t batCount;
        double batUpMax;
        double batDownMax;
        double batRolloff;

        double convReg;
        double batReg;
public:
        AGControlBattery();
        virtual gridCoreObject *clone(gridCoreObject *obj = NULL, bool copyName = false) const;
        virtual ~AGControlBattery();


        virtual double objectInitializeA (gridDyn_time time0,double freq0,double tiedev0);

        virtual double updateA(gridDyn_time time, double freq, double tiedev);

        virtual void addGen(schedulerReg *sched);
        virtual void removeSched(schedulerReg *sched);
        virtual void set (const std::string &param, std::string val);
        virtual void set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit);

        virtual void regChange();
protected:
};

*/

#endif
