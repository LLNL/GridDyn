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

#ifndef ISOCCONTROLLER_H_
#define ISOCCONTROLLER_H_

#include "../gridSubModel.h"
namespace griddyn {
class Generator;

class isocController: public gridSubModel {
  protected:
    model_parameter db = 0.005;
    model_parameter upStep = -0.01;
    model_parameter downStep = 0.02 * 0.25;
    model_parameter upPeriod = 1.0;
    model_parameter downPeriod = 0.25;
    model_parameter maxLevel = 1.0;
    model_parameter minLevel = -1.0;
    model_parameter lastFreq = 0.0;
    model_parameter integralTrigger = 0.02;
    model_parameter integratorLevel = 0.0;
    Generator* gen = nullptr;

  public:
    explicit isocController(const std::string& objName = "ISOC_#");
    virtual coreObject* clone(coreObject* obj = nullptr) const override;
    virtual void dynObjectInitializeA(coreTime time0, std::uint32_t flags) override;

    virtual void dynObjectInitializeB(const IOdata& inputs,
                                      const IOdata& desiredOutput,
                                      IOdata& fieldSet) override;

    virtual void updateA(coreTime time) override;

    virtual void timestep(coreTime time, const IOdata& inputs, const solverMode& sMode) override;

    virtual void set(const std::string& param, const std::string& val) override;
    virtual void
        set(const std::string& param, double val, units::unit unitType = units::defunit) override;
    /** set the upper and lower limits usable for the isocController
    @param[in] maxV  the upper limit on the value
    @parma[in] minV  the lower limit on the value
    */
    virtual void setLimits(double minV, double maxV);
    /** set a new target level for the controller
    @param[in] newLevel the targeted level
    */
    virtual void setLevel(double newLevel);

    void setFreq(double freq);
    void deactivate();
    void activate(coreTime time);
};

}  // namespace griddyn
#endif
