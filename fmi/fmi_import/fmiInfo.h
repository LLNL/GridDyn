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

#ifndef FMI_INFORMATION_H_
#define FMI_INFORMATION_H_


#include "matrixDataOrdered.h"
#include "FMI2/fmi2TypesPlatform.h"
#include <string>
#include <vector>
#include <bitset>
#include <map>
#include <memory>

enum class fmutype_t
{
	unknown,
	modelExchange,
	cosimulation,
};

enum class fmi_variability_type_t
{
	continuous,
	constant,
	fixed,
	tunable,
	discrete,
	unknown,
};

fmi_variability_type_t variabilityFromString(const std::string &vstring);
std::string toString(fmi_variability_type_t variability);

enum class fmi_causality_type_t
{
	local,
	parameter,
	calculatedParameter,
	input,
	output,
	independent,
	unknown,
	any,
};

fmi_causality_type_t causalityFromString(const std::string &vstring);
std::string toString(fmi_causality_type_t causality);

enum class fmi_type_t
{
	real,
	integer,
	boolean,
	string,
	enumeration,
	unknown,
	numeric,  //!< not used in an fmu but intended to catch all numeric in search operations
};

fmi_type_t typeFromString(const std::string &vstring);
std::string toString(fmi_type_t dependencies);

enum class fmi_dependencies_t
{
	unknown,
	dependent,
	constant,
	fixed,
	tunable,
	discrete,
	independent,
};

fmi_dependencies_t dependenciesFromString(const std::string &vstring);
std::string toString(fmi_dependencies_t dependencies);

enum fmuCapabilityFlags:int
{
	modelExchangeCapable,
	coSimulationCapable,
	canGetAndSetFMUstate,
	providesDirectionalDerivative,
	canSerializeFMUstate,
	needsExecutionTool,
	completedIntegratorStepNotNeeded,
	canHandleVariableCommunicationStepSize,
	canInterpolateInputs,
	canRunAsynchronously,
	canBeInstantiatedOnlyOncePerProcess,
	canNotUseMemoryManagementFunctions,

};

class fmuDefaultExpirement
{
public:
	double startTime = 0.0;
	double stopTime = 0.0;
	double stepSize = 0.00;
	double tolerance = 1e-8;
};

class variableInformation
{
public:
	int index=-1;
	int derivativeIndex = -1;
	fmi2ValueReference valueRef = 0;
	int aliasIndex = 0;
	std::string name;
	std::string description;
	std::string declType;
	std::string unit;
	std::string initial;
	bool multiSet = false;
	bool reinit = false;
	bool derivative = false;
	bool isAlias = false;
	fmi_variability_type_t variability = fmi_variability_type_t::continuous;
	fmi_causality_type_t causality = fmi_causality_type_t::local;
	fmi_type_t type = fmi_type_t::real;
	double start=0;
	double min=-1e48;
	double max=1e48;

};

typedef struct
{
	std::string name;
	double factor;
	double offset;
} unitDef;

/** data class for storing fmi unit information*/
class fmiUnit
{
public:
	std::string name;
	double factor = 1.0;
	double offset = 0.0;
	std::vector<unitDef> baseUnits;
	
	std::vector<unitDef> displayUnits;

};

class fmiTypeDefinition
{
public:
	std::string name;
	std::string description;
	std::string quantity;
	std::string unit;
	std::string displayUnit;
	fmi_type_t type;
	bool relativeQuantity=false;
	bool unbounded=false;
	double min;
	double max;
	double nominal;
};

/** class for storing references to fmi variables*/
class fmiVariableSet
{
public:
	fmiVariableSet();
	fmiVariableSet(fmi2ValueReference newvr);
	fmiVariableSet(const fmiVariableSet &vset);
	fmiVariableSet(fmiVariableSet &&vset);

	fmiVariableSet& operator=(const fmiVariableSet & other);
	fmiVariableSet& operator=(fmiVariableSet && other);

	const fmi2ValueReference *getValueRef() const;
	size_t getVRcount() const;
	fmi_type_t getType() const;
	void push(fmi2ValueReference newvr);
	void push(const fmiVariableSet &vset);
	void setSize(size_t newSize);
	void remove(fmi2ValueReference rmvr);
	void clear();
private:
	fmi_type_t type = fmi_type_t::real;
	fmi2ValueReference vr = 0;
	size_t cnt = 0;
	std::vector<fmi2ValueReference> vrset;
};

class readerElement;
/** class to extract and store the information in an FMU XML file*/
class fmiInfo
{
private:
	std::map<std::string, std::string> headerInfo; //!< the header information contained in strings
	double fmiVersion; //!< the fmi version used
	int numberOfEvents; //!< the number of defined events
	int maxOrder;  //!< the maximum derivative order for CoSimulation FMU's
	std::bitset<32> capabilities; //!< bitset containing the capabilities of the FMU
	std::vector<variableInformation> variables;  //!< information all all the defined variables
	std::vector<fmiUnit> units;  //!< all the units defined in the FMU
	fmuDefaultExpirement defaultExpirement;  //!< the information about the specified default experiment

	std::map<std::string, int> variableLookup;  //!< map translating strings to indices into the variables array

	matrixDataRowOrdered<int> outputDep;	//!< the output dependency information
	matrixDataRowOrdered<int> derivDep;	//!< the derivative dependency information
	matrixDataRowOrdered<int> unknownDep; //!< the initial unknown dependency information
	std::vector<int> outputs;	//!< a list of the output indices
	std::vector<int> parameters;	//!< a list of all the parameters
	std::vector<int> local;	//!< a list of the local variables
	std::vector<int> states;	//!< a list of the states
	std::vector<int> deriv;	//!< a list of the derivative information
	std::vector<int> initUnknown;	//!< a list of the unknowns
	std::vector<int> inputs;	//!< a list of the inputs
public:
	fmiInfo();
	fmiInfo(const std::string &xmlFile);
	int loadFile(const std::string &xmlfile);

	bool checkFlag(fmuCapabilityFlags flag) const;

	int getCounts(const std::string &countType) const;
	const std::string &getString(const std::string &field) const;
	double getReal(const std::string &field) const;
	const variableInformation& getVariableInfo(const std::string &variableName) const;
	const variableInformation& getVariableInfo(int index) const;
	fmiVariableSet getReferenceSet(const std::vector<std::string > &variableList) const;
	fmiVariableSet getVariableSet(const std::string &variable) const;
	fmiVariableSet getVariableSet(int index) const;
	fmiVariableSet getOutputReference() const;
	fmiVariableSet getInputReference() const;

	std::vector<std::string> getVariableNames(const std::string &type) const;
	const std::vector<int> &getVariableIndices(const std::string &type) const;
	/** get the variable indices of the derivative dependencies*/
	const std::vector<std::pair<index_t,int>> &getDerivDependencies(int variableIndex) const;
	const std::vector<std::pair<index_t, int>> &getOutputDependencies(int variableIndex) const;
	const std::vector<std::pair<index_t, int>> &getUnknownDependencies(int variableIndex) const;
private:
	void loadFmiHeader(std::shared_ptr<readerElement> &rd);
	void loadVariables(std::shared_ptr<readerElement> &rd);
	void loadUnitInformation(std::shared_ptr<readerElement> &rd);
	void loadStructure(std::shared_ptr<readerElement> &rd);
};


enum class fmuMode
{
	instantiatedMode,
	initializationMode,
	continuousTimeMode,
	eventMode,
	terminated,
	error,
};


bool checkType(const variableInformation& info, fmi_type_t type, fmi_causality_type_t caus);

#endif