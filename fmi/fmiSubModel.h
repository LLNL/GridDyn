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

#ifndef FMI_SUBMODEL_H_
#define FMI_SUBMODEL_H_

#include "gridCore.h"
#include "gridObjects.h"
#include <fmilib.h>
#include <FMI2/fmi2_types.h>

#include <map>

struct fmi2_import_t;
struct fmi_import_context;
class outputEstimator
{
public:
	double time;
	double prevValue;
	std::vector<int> stateDep;
	std::vector<int> inputDep;
	std::vector<double> stateDiff;
	std::vector<double> inputDiff;
	std::vector<double> prevInputs;
	std::vector<double> prevStates;
	double timeDiff = 0;

	outputEstimator(std::vector<int> sDep, std::vector<int> iDep);
	double estimate(double time, const IOdata &args, const double state[]);
	bool update(double time, double val, const IOdata &args, const double state[]);
};

class fmiSubModel : public gridSubModel
{
public:
	enum class fmiState_t { fmi_startup, fmi_loaded, fmi_instantiated, fmi_init, fmi_cont_time, fmi_event, fmi_terminated, fmi_error };
	fmi_version_enu_t version = fmi_version_unknown_enu;
	std::string fmu_name;
	std::string fmu_dir;
protected:
	
	fmiState_t fmiState = fmiState_t::fmi_startup;
	count_t m_stateSize = 0;
	count_t m_jacElements = 0;
	count_t m_eventCount = 0;
	fmi_import_context_t* context = nullptr;
	bool valid = false;
public:
	fmiSubModel(fmi_import_context_t *ctx=nullptr);
	virtual ~fmiSubModel();
	std::string extractFMU();
	virtual stringVec getOutputNames() const;
	virtual stringVec getInputNames() const;
	fmiState_t getFmiState() const{ return fmiState; };
	};


class fmiSubModel1: public fmiSubModel
{
public:
	fmiSubModel1(fmi_import_context_t *ctx = nullptr) :fmiSubModel(ctx){};
};



class fmi2vardesc
{
public:
	std::string name;
	fmi2_value_reference_t vr;
	fmi2_base_type_enu_t type;
	fmi2_causality_enu_t caus;
	fmi2_variability_enu_t vari;
	int refMode;
	std::vector<int> dependencies;
	std::vector<int> stateDep;
	std::vector<int> inputDep;
	std::vector<int> derivDep;
	index_t index;
	int reference;
	bool active=true;
	bool deriv=false;
	bool state=false;
};



typedef std::map<std::string, int> fmi2ParamMap;

typedef std::map<fmi2_value_reference_t, int> fmi2RefMap;

class fmiSubModel2 : public fmiSubModel
{
public:
	enum fmi2_flags {
		has_derivative_function = object_flag2,
		use_output_estimator = object_flag3,
		fixed_output_interval = object_flag4,
		reprobe_flag=object_flag5,
	};
  protected:
  

  fmi2_import_t *fmu;

  fmi2ParamMap paramStr; //!<map of parameter names
  fmi2ParamMap inputStr; //!<map of input names
  fmi2ParamMap outputStr; //!map of output names
  fmi2ParamMap localStr; //!<map of local names
  fmi2ParamMap stateStr; //!<map of the state names

  fmi2RefMap refMap; //!< mapping from value reference to index
  std::vector<int> inputIndex;  //!< references to the input value references
  std::vector<int> outputIndex; //!<references to the output value references
  std::vector<int> inputIndexActive;  //!<references to the input value references in active use
  std::vector<int> outputIndexActive; //!<references to the output value references in active use
  std::vector<fmi2_value_reference_t> inputRefActive;  //!<references to the input value references in active use
  std::vector<fmi2_value_reference_t> outputRefActive; //!<references to the output value references in active use
  std::vector<int> stateIndex;  //!< references to the state value references
  std::vector<int>  dstateIndex; //!< references to the state derivative references

  std::vector<fmi2vardesc> varInfo; //!< vector of the parameter types of the same size as param
  std::vector<outputEstimator *> oEst;  //!<vector of objects used for output estimation
  double localIntegrationTime = 0.01;
private:
	fmiState_t prevFmiState = fmiState_t::fmi_startup;
	int lastSeqID = 0;
	std::vector<double> tempState;
	std::vector<double> tempdState;
  public:
	  fmiSubModel2(fmi_import_context_t *ctx = nullptr);
  virtual ~fmiSubModel2();
  virtual gridCoreObject * clone(gridCoreObject *obj = nullptr) const override;
  virtual void objectInitializeA (double time, unsigned long flags) override;
  virtual void objectInitializeB (const IOdata &args, const IOdata &outputSet, IOdata &inputSet) override;

  virtual void getParameterStrings(stringVec &pstr, paramStringType pstype) const override;
  virtual stringVec getOutputNames() const override;
  virtual stringVec getInputNames() const override;
  virtual int set (const std::string &param, const std::string &val) override;
  virtual int set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit)  override;

  virtual double get(const std::string &param , gridUnits::units_t unitType = gridUnits::defUnit) const  override;
  virtual index_t findIndex(const std::string &field, const solverMode &sMode) const  override;
  virtual void loadSizes(const solverMode &sMode, bool dynOnly) override;
  virtual void residual(const IOdata &args, const stateData *sD, double resid[], const solverMode &sMode) override;
  virtual void derivative(const IOdata &args, const stateData *sD, double deriv[], const solverMode &sMode) override;
  virtual void jacobianElements(const IOdata &args, const stateData *sD,
    arrayData<double> *ad,
    const IOlocs &argLocs, const solverMode &sMode) override;
  virtual double timestep (double ttime, const IOdata &args, const solverMode &sMode) override;
  virtual void ioPartialDerivatives(const IOdata &args, const stateData *sD, arrayData<double> *ad, const IOlocs &argLocs, const solverMode &sMode) override;
  virtual void outputPartialDerivatives (const IOdata &args, const stateData *sD, arrayData<double> *ad, const solverMode &sMode) override;
  virtual void rootTest(const IOdata &args, const stateData *sD, double roots[], const solverMode &sMode) override;
  virtual void rootTrigger(double ttime, const IOdata &args, const std::vector<int> &rootMask, const solverMode &sMode) override;

  IOdata getOutputs(const IOdata &args, const stateData *sD, const solverMode &sMode) override;
  virtual double getDoutdt(const stateData *sD, const solverMode &sMode, index_t num = 0) override;
  virtual double getOutput(const IOdata &args, const stateData *sD, const solverMode &sMode, index_t num = 0) const override;

  virtual double getOutput(index_t num = 0) const override;
  virtual double getOutputLoc(const IOdata &args, const stateData *sD, const solverMode &sMode, index_t &currentLoc, index_t num = 0) const override;
  //virtual void setTime(double time){prevTime=time;};


  virtual void setState(double ttime, const double state[], const double dstate_dt[], const solverMode &sMode) override;
  //for saving the state
  virtual void guess(double ttime, double state[], double dstate_dt[], const solverMode &sMode) override;

  virtual void getTols(double tols[], const solverMode &sMode) override;

  virtual void getStateName(stringVec &stNames, const solverMode &sMode, const std::string &prefix = "") const override;
  protected:
  void loadFMU();
  void updateInfo(const IOdata &args, const stateData *sD,const solverMode &sMode);
  void instantiateFMU();
  void updateDependencyInfo();
  void makeSettableState();
  void resetState();
  double getPartial(int depIndex, int refIndex, int mode);
  void probeFMU();
  void loadOutputJac(int index = -1);
  int searchByRef(fmi2_value_reference_t ref);
};

fmiSubModel *makefmiSubModel(const std::string &fmu_path);
/**
extract the FMU to the specified directory and get the version number
@param context   and fmi import context
@param fmu_path a string containing the extracted directory or the fmu file location
@param fmu_dir if empty it contains the location of the extracted directory otherwise the desired extraction location
@return the an enumeration of the fmu version
**/
fmi_version_enu_t getFmuVersion(fmi_import_context_t *context,const std::string &fmu_path, std::string &fmu_dir);

#endif