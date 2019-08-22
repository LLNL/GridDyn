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

#include "../gridSubModel.h"
#include "gmlc/utilities/vectorOps.hpp"

namespace griddyn
{
class Area;
class schedulerRamp;

/** in development object to manage the dispatch of reserve generation
 */
class reserveDispatcher : public coreObject
{
  public:
  protected:
    double thresholdStart = kBigNum;
    double thresholdStop = kBigNum;
    double currDispatch = 0.0;
    double reserveAvailable = 0.0;
    coreTime dispatchTime = negTime;
    coreTime dispatchInterval = 60.0 * 5.0;

    count_t schedCount;
    std::vector<schedulerRamp *> schedList;
    std::vector<double> resAvailable;
    std::vector<double> resUsed;

  public:
    explicit reserveDispatcher (const std::string &objName = "reserveDispatch_#");
    virtual coreObject *clone (coreObject *obj = nullptr) const override;
    virtual ~reserveDispatcher ();

    virtual double dynInitializeA (coreTime time0, double dispatchSet);

    void moveSchedulers (reserveDispatcher *rD);

    virtual double updateP (coreTime time, double pShort);
    virtual double testP (coreTime time, double pShort);
    double getOutput (index_t /*num*/ = 0) { return currDispatch; }

    virtual void add (schedulerRamp *sched);
    virtual void add (coreObject *obj) override;

    virtual void remove (schedulerRamp *sched);
    virtual void remove (coreObject *obj) override;

    virtual void set (const std::string &param, const std::string &val) override;
    virtual void
    set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;

    double getAvailable () { return reserveAvailable; }

    virtual void schedChange ();

  protected:
    virtual void checkGen ();
    virtual void dispatch (double level);
};

}  // namespace griddyn
