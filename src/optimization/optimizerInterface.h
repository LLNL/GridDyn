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

#ifndef _OPTIM_INTERFACE_H_
#define _OPTIM_INTERFACE_H_

#include "griddyn/griddyn-config.h"
#include "optHelperClasses.h"

#include "utilities/vectData.hpp"
#include <memory>
#include <string>
#include <vector>

namespace griddyn
{
class gridDynOptimization;

class optimizerInterface
{
  public:
    vectData<double> v1;
    // solver outputs
    std::vector<double> values;  //!< mask vector for which roots were found

    optimMode mode;
    double rtol = 1e-6;  //!< solver relative solution tolerance
    bool sparse = false;
    bool constantJacobian = false;

  protected:
    std::string name;
    void *solverMem = nullptr;
    bool allocated = false;
    bool initialized = false;  //!< flag indicating if these vectors have been initialized
    gridDynOptimization *m_gdo = nullptr;
    count_t svsize = 0;

  public:
    optimizerInterface (const std::string &optName = "optim");

    optimizerInterface (gridDynOptimization *gdo, const optimMode &oMode);
    virtual ~optimizerInterface () {}
    virtual double *val_data () { return values.data (); }

    virtual int allocate (count_t size)
    {
        values.resize (size);
        return 0;
    }
    virtual void dynObjectInitializeA (double /*t0*/) {}

    virtual double get (const std::string &param) const;
    virtual int solve (double /*tStop*/, double & /*tReturn*/) { return -101; }
    void initializeJacArray (count_t size);
    bool isInitialized () const { return initialized; }

    virtual void logSolverStats (int /*logLevel*/, bool /*iconly*/ = false) {}
    virtual void logErrorWeights (int /*logLevel*/) {}

    count_t getSize () const { return svsize; }
    virtual void setOptimizationData (gridDynOptimization *gdo, const optimMode &oMode);
    virtual int check_flag (void *flagvalue, const std::string &funcname, int opt, bool printError = true);
    void setName (std::string newName) { name = newName; }
    const std::string &getName () const { return name; }
};

class basicOptimizer : public optimizerInterface
{
  private:
  public:
    explicit basicOptimizer (const std::string &optName = "basic");

    basicOptimizer (gridDynOptimization *gdo, const optimMode &oMode);

    int allocate (count_t size) override;
    void dynObjectInitializeA (double t0) override;
};

std::shared_ptr<optimizerInterface> makeOptimizer (gridDynOptimization *gdo, const optimMode &oMode);

std::shared_ptr<optimizerInterface> makeOptimizer (const std::string &type);

}  // namespace griddyn

#endif
