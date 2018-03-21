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

#ifndef FMI_COSIMSUBMODEL_H_
#define FMI_COSIMSUBMODEL_H_

#include "griddyn/gridSubModel.h"
#include "fmiSupport.h"
#include <map>

class fmi2CoSimObject;
class outputEstimator;

namespace griddyn
{
namespace fmi
{
/** class defining a subModel interacting with an FMU v2.0 object using cosimulation*/
class fmiCoSimSubModel : public gridSubModel
{
public:
	enum fmiSubModelFlags
	{
		use_output_estimator = object_flag2,
		fixed_output_interval = object_flag3,
		has_derivative_function = object_flag5,
	};
protected:
	std::shared_ptr<fmi2CoSimObject> cs;

  std::vector<outputEstimator *> estimators;  //!<vector of objects used for output estimation
  double localIntegrationTime = 0.01;
private:

	int lastSeqID = 0;
  public:
	  fmiCoSimSubModel(const std::string &newName="fmicosimsubmodel_#", std::shared_ptr<fmi2CoSimObject> fmi=nullptr);

	  fmiCoSimSubModel(std::shared_ptr<fmi2CoSimObject> fmi = nullptr);
  virtual ~fmiCoSimSubModel();
  virtual coreObject * clone(coreObject *obj = nullptr) const override;
  virtual void dynObjectInitializeA (coreTime time0, std::uint32_t flags) override;
  virtual void dynObjectInitializeB (const IOdata &inputs, const IOdata &desiredOutput, IOdata &fieldSet) override;

  virtual void getParameterStrings(stringVec &pstr, paramStringType pstype) const override;
  virtual stringVec getOutputNames() const;
  virtual stringVec getInputNames() const;
  virtual void set (const std::string &param, const std::string &val) override;
  virtual void set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit)  override;

  virtual double get(const std::string &param , gridUnits::units_t unitType = gridUnits::defUnit) const  override;
 
  virtual void timestep (coreTime time, const IOdata &inputs, const solverMode &sMode) override;
  virtual void ioPartialDerivatives(const IOdata &inputs, const stateData &sD, matrixData<double> &md, const IOlocs &inputLocs, const solverMode &sMode) override;
  
  IOdata getOutputs(const IOdata &inputs, const stateData &sD, const solverMode &sMode) const override;
  virtual double getDoutdt(const IOdata &inputs, const stateData &sD, const solverMode &sMode, index_t num = 0) const override;
  virtual double getOutput(const IOdata &inputs, const stateData &sD, const solverMode &sMode, index_t num = 0) const override;

  virtual double getOutput (index_t outputNum = 0) const override;

  virtual void updateLocalCache(const IOdata &inputs, const stateData &sD, const solverMode &sMode) override;
  bool isLoaded() const;
  protected:
  void loadFMU();
  
  void instantiateFMU();
  void makeSettableState();
  void resetState();
  double getPartial(int depIndex, int refIndex, refMode_t mode);

  void loadOutputJac(int index = -1);
 
};

}//namespace fmi
}//namespace griddyn
#endif