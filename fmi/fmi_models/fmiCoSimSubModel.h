/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil;  eval: (c-set-offset 'innamespace 0); -*- */
/*
* LLNS Copyright Start
* Copyright (c) 2015, Lawrence Livermore National Security
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

#include "gridObjects.h"
#include "fmiSupport.h"
#include <map>



class fmi2CoSim;
class outputEstimator;


/** class defining a subModel interacting with an FMU v2.0 object*/
class fmiCoSimSubModel : public gridSubModel
{
public:
	enum fmiSubModelFlags
	{
		use_output_estimator = object_flag2,
		fixed_output_interval = object_flag3,
		reprobe_flag = object_flag4,
		has_derivative_function = object_flag5,
	};
protected:
	count_t m_stateSize = 0;
	count_t m_jacElements = 0;
	count_t m_eventCount = 0;
	std::shared_ptr<fmi2CoSim> cs;

  std::vector<outputEstimator *> oEst;  //!<vector of objects used for output estimation
  double localIntegrationTime = 0.01;
private:

	int lastSeqID = 0;
	std::vector<double> tempState;
	std::vector<double> tempdState;
  public:
	  fmiCoSimSubModel(const std::string &newName="fmicosimsubmodel_#", std::shared_ptr<fmi2CoSim> fmi=nullptr);

	  fmiCoSimSubModel(std::shared_ptr<fmi2CoSim> fmi = nullptr);
  virtual ~fmiCoSimSubModel();
  virtual gridCoreObject * clone(gridCoreObject *obj = nullptr) const override;
  virtual void objectInitializeA (double time, unsigned long flags) override;
  virtual void objectInitializeB (const IOdata &args, const IOdata &outputSet, IOdata &inputSet) override;

  virtual void getParameterStrings(stringVec &pstr, paramStringType pstype) const override;
  virtual stringVec getOutputNames() const;
  virtual stringVec getInputNames() const;
  virtual void set (const std::string &param, const std::string &val) override;
  virtual void set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit)  override;

  virtual double get(const std::string &param , gridUnits::units_t unitType = gridUnits::defUnit) const  override;
  virtual index_t findIndex(const std::string &field, const solverMode &sMode) const  override;
  virtual void loadSizes(const solverMode &sMode, bool dynOnly) override;
  virtual void residual(const IOdata &args, const stateData *sD, double resid[], const solverMode &sMode) override;
  virtual void derivative(const IOdata &args, const stateData *sD, double deriv[], const solverMode &sMode) override;
  virtual void jacobianElements(const IOdata &args, const stateData *sD,
    matrixData<double> *ad,
    const IOlocs &argLocs, const solverMode &sMode) override;
  virtual void timestep (double ttime, const IOdata &args, const solverMode &sMode) override;
  virtual void ioPartialDerivatives(const IOdata &args, const stateData *sD, matrixData<double> *ad, const IOlocs &argLocs, const solverMode &sMode) override;
  virtual void outputPartialDerivatives (const IOdata &args, const stateData *sD, matrixData<double> *ad, const solverMode &sMode) override;
  virtual void rootTest(const IOdata &args, const stateData *sD, double roots[], const solverMode &sMode) override;
  virtual void rootTrigger(double ttime, const IOdata &args, const std::vector<int> &rootMask, const solverMode &sMode) override;

  IOdata getOutputs(const IOdata &args, const stateData *sD, const solverMode &sMode) override;
  virtual double getDoutdt(const stateData *sD, const solverMode &sMode, index_t num = 0) override;
  virtual double getOutput(const IOdata &args, const stateData *sD, const solverMode &sMode, index_t num = 0) const override;

  virtual double getOutput(index_t num = 0) const override;
  virtual index_t getOutputLoc( const solverMode &sMode, index_t num = 0) const override;
  //virtual void setTime(double time){prevTime=time;};


  virtual void setState(double ttime, const double state[], const double dstate_dt[], const solverMode &sMode) override;
  //for saving the state
  virtual void guess(double ttime, double state[], double dstate_dt[], const solverMode &sMode) override;

  virtual void getTols(double tols[], const solverMode &sMode) override;

  virtual void getStateName(stringVec &stNames, const solverMode &sMode, const std::string &prefix = "") const override;
  bool isLoaded() const;
  protected:
  void loadFMU();
  void updateInfo(const IOdata &args, const stateData *sD,const solverMode &sMode);
  void instantiateFMU();
  void updateDependencyInfo();
  void makeSettableState();
  void resetState();
  double getPartial(int depIndex, int refIndex, refMode_t mode);
  void probeFMU();
  void loadOutputJac(int index = -1);
 
};


#endif