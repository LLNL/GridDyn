/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil;  eval: (c-set-offset 'innamespace 0); -*- */
/*
* LLNS Copyright Start
* Copyright (c) 2017, Lawrence Livermore National Security
* This work was performed under the auspices of the U.S. Department
* of Energy by Lawrence Livermore National Laboratory in part under
* Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
* Produced at the Lawrence Livermore National Laboratory.
* All rights reserved.
* For details, see the LICENSE file.
* LLNS Copyright End
*/

#include "objectGrabbers.h"
#include "gridBus.h"
#include "gridObjects.h"
#include "gridArea.h"
#include "linkModels/gridLink.h"
#include "loadModels/gridLoad.h"
#include "generators/gridDynGenerator.h"
#include "relays/gridRelay.h"
#include "relays/sensor.h"
#include "stringOps.h"
#include "mapOps.h"
#include "solvers/solverMode.h"
#include <map>

using namespace gridUnits;

static const fobjectPair nullPair{ nullptr,defUnit };
static const fvecPair nullVecPair{ nullptr,defUnit };

/** map of all the alternate strings that can be used*/
/* *INDENT-OFF* */
static const std::map<std::string, std::string> stringTranslate
{
	{"voltage","v"},
	{"vol","v"},
	{"link","linkreal"},
	{"linkp","linkreal"},
	{"linkepower","linereal"},
	{"loadq","loadreactive"},
	{"loadreactivepower","loadreactive"},
	{"load","loadreal"},
	{"loadp","loadreal"},
	{"loadpower","loadreal"},
	{"reactivegen","genreactive"},
	{"genq","genreactive"},
	{"gen","genreal"},
	{"generation","genreal"},
	{"genp","genreal"},
	{"genpower","genreal"},
	{"realgen","genreal"},
	{"f","freq"},
	{"frequency","freq"},
	{"omega","freq"},
	{"a","angle"},
	{"phase","angle"},
	{"busgenerationreal","busgenreal"},
	{"busp","busgenreal"},
	{"buspower","busgenreal"},
	{"busgen","busgenreal"},
	{"busload","busloadreal"},
	{"busloadp","busloadreal"},
	{ "busloadq","busloadreactive" },
	{"busgenerationreactive","busgenreactive"},
	{"busq","busgenreactive"},
	{"linkrealpower","linkreal"},
	{ "linkp1","linkreal" },
	{"linkq","linkreactive"},
	{"linkreactivepower","linkreactive"},
	{ "linkrealpower1","linkreal" },
	{ "linkq1","linkreactive" },
	{ "linkreactivepower1","linkreactive" },
	{"linkreal1","linkreal"},
	{"linkreactive1","linkreactive"},
	{"linkrealpower2","linkreal2"},
	{ "linkq2","linkreactive2" },
	{ "linkreactivepower2","linkreactive2" },
	{"linkp2","linkreal2"},
	{"p","real"},
	{"power","real"},
	{"q","reactive"},
	{"impedance","z"},
	{"admittance","y"},
	{ "impedance1","z" },
	{ "admittance1","y" },
	{ "z1","z" },
	{ "y1","y" },
	{ "impedance2","z2" },
	{ "admittance2","y2" },
	{"status","connected"},
	{"breaker","switch"},
	{"breaker1","switch"},
	{"switch1","switch"},
	{ "breaker2","switch2" },
	{"i","current"},
	{ "i1","current" },
	{"current1","current"},
	{"i2","current2"},
	{"imagcurrent1","imagcurrent"},
	{"realcurrent1","realcurrent"},
	{"lossreal","loss"},
	{"angle1","angle"},
	{"absangle1","absangle"},
	{"voltage1","voltage"},
	{"v1","voltage"},
	{ "v2","voltage2" },
	{"output0","output"},
	{"cv","output"},
	{"o0","output"},
	{"currentvalue","output"},
	{"deriv0","deriv"},
	{"dodt","deriv"},{ "dodt0","deriv" },
	{"doutdt","deriv"},{ "doutdt0","deriv" }
	
};

static const std::map<std::string, fobjectPair> objectFunctions
{
	{ "connected",{ [](coreObject *obj) {return static_cast<double>(static_cast<gridObject *>(obj)->isConnected()); },defUnit } },
	{ "enabled",{ [](coreObject *obj) {return static_cast<double>(static_cast<gridObject *>(obj)->isEnabled()); },defUnit } },
	{ "armed",{ [](coreObject *obj) {return static_cast<double>(static_cast<gridObject *>(obj)->isArmed()); },defUnit } },
	{"output",{ [](coreObject *obj) {return static_cast<gridObject *>(obj)->getOutput(0); },defUnit } },
{"deriv",{ [](coreObject *obj) {return static_cast<gridObject *>(obj)->getDoutdt(noInputs,emptyStateData,cLocalbSolverMode,0); },defUnit } }
};

static const std::map<std::string, fobjectPair> busFunctions
{
	{"v",{[](coreObject *obj) {return static_cast<gridBus *>(obj)->getVoltage(); }, puV} },
	{ "angle",{ [](coreObject *obj) {return static_cast<gridBus *>(obj)->getAngle(); }, rad } },
	{ "freq",{ [](coreObject *obj) {return static_cast<gridBus *>(obj)->getFreq(); },puHz } },
	{ "genreal",{ [](coreObject *obj) {return static_cast<gridBus *>(obj)->getGenerationReal(); }, puMW } },
	{ "genreactive",{ [](coreObject *obj) {return static_cast<gridBus *>(obj)->getGenerationReactive(); }, puMW } },
	{ "loadreal",{ [](coreObject *obj) {return static_cast<gridBus *>(obj)->getLoadReal(); }, puMW } },
	{ "loadreactive",{ [](coreObject *obj) {return static_cast<gridBus *>(obj)->getLoadReactive(); }, puMW } },
	{ "linkreal",{ [](coreObject *obj) {return static_cast<gridBus *>(obj)->getLinkReal(); }, puMW } },
	{ "linkreactive",{ [](coreObject *obj) {return static_cast<gridBus *>(obj)->getLinkReactive(); }, puMW } },
};

static const std::map<std::string, fobjectPair> areaFunctions
{
	{ "avgfreq",{ [](coreObject *obj) {return static_cast<gridArea *>(obj)->getAvgFreq(); },puHz } },
	{ "genreal",{ [](coreObject *obj) {return static_cast<gridArea *>(obj)->getGenerationReal(); }, puMW } },
	{ "genreactive",{ [](coreObject *obj) {return static_cast<gridArea *>(obj)->getGenerationReactive(); }, puMW } },
	{ "loadreal",{ [](coreObject *obj) {return static_cast<gridArea *>(obj)->getLoadReal(); }, puMW } },
	{ "loadreactive",{ [](coreObject *obj) {return static_cast<gridArea *>(obj)->getLoadReactive(); }, puMW } },
	{ "loss",{ [](coreObject *obj) {return static_cast<gridArea *>(obj)->getLoss(); }, puMW } },
	{ "tieflow",{ [](coreObject *obj) {return static_cast<gridArea *>(obj)->getTieFlowReal(); }, puMW } },
};

static const std::map<std::string, fvecPair> areaVecFunctions
{
	{ "v",{ [](coreObject *obj,std::vector<double> &data) {return static_cast<gridArea *>(obj)->getVoltage(data); }, puV } },
	{ "angle",{ [](coreObject *obj,std::vector<double> &data) {return static_cast<gridArea *>(obj)->getAngle(data); }, rad } },
	{ "freq",{ [](coreObject *obj,std::vector<double> &data) {return static_cast<gridArea *>(obj)->getFreq(data); },puHz } },
	{ "busgenreal",{ [](coreObject *obj,std::vector<double> &data) {return static_cast<gridArea *>(obj)->getBusGenerationReal(data); }, puMW } },
	{ "busgenreactive",{ [](coreObject *obj,std::vector<double> &data) {return static_cast<gridArea *>(obj)->getBusGenerationReactive(data); }, puMW } },
	{ "busloadreal",{ [](coreObject *obj,std::vector<double> &data) {return static_cast<gridArea *>(obj)->getBusLoadReal(data); }, puMW } },
	{ "busloadreactive",{ [](coreObject *obj,std::vector<double> &data) {return static_cast<gridArea *>(obj)->getBusLoadReactive(data); }, puMW } },
	{ "linkreal",{ [](coreObject *obj,std::vector<double> &data) {return static_cast<gridArea *>(obj)->getLinkRealPower(data); }, puMW } },
	{ "linkreactive",{ [](coreObject *obj,std::vector<double> &data) {return static_cast<gridArea *>(obj)->getLinkReactivePower(data); }, puMW } },
	{ "linkloss",{ [](coreObject *obj,std::vector<double> &data) {return static_cast<gridArea *>(obj)->getLinkLoss(data); }, puMW } },

};

static const std::map<std::string, descVecFunc> areaVecDescFunctions
{
	{ "v", [](coreObject *obj,stringVec &desc) {return static_cast<gridArea *>(obj)->getBusName(desc); } },
	{ "angle", [](coreObject *obj,stringVec &desc) {return static_cast<gridArea *>(obj)->getBusName(desc); } },
	{ "freq", [](coreObject *obj,stringVec &desc) {return static_cast<gridArea *>(obj)->getBusName(desc); } },
	{ "busgenreal", [](coreObject *obj,stringVec &desc) {return static_cast<gridArea *>(obj)->getBusName(desc); } },
	{ "busgenreactive", [](coreObject *obj,stringVec &desc) {return static_cast<gridArea *>(obj)->getBusName(desc); } },
	{ "busloadreal", [](coreObject *obj,stringVec &desc) {return static_cast<gridArea *>(obj)->getBusName(desc); } },
	{ "busloadreactive", [](coreObject *obj,stringVec &desc) {return static_cast<gridArea *>(obj)->getBusName(desc); } },
	{ "linkreal", [](coreObject *obj,stringVec &desc) {return static_cast<gridArea *>(obj)->getLinkName(desc);  } },
	{ "linkreactive", [](coreObject *obj,stringVec &desc) {return static_cast<gridArea *>(obj)->getLinkName(desc); } },
	{ "linkloss", [](coreObject *obj,stringVec &desc) {return static_cast<gridArea *>(obj)->getLinkName(desc);  } },

};

static const std::map<std::string, fobjectPair> loadFunctions
{
	{ "real",{ [](coreObject *obj) {return static_cast<gridLoad *>(obj)->getRealPower(); }, puMW } },
	{ "reactive",{ [](coreObject *obj) {return static_cast<gridLoad *>(obj)->getReactivePower(); }, puMW } },
	{ "loadreal",{ [](coreObject *obj) {return static_cast<gridLoad *>(obj)->getRealPower(); },puMW } },
	{ "loadreactive",{ [](coreObject *obj) {return static_cast<gridLoad *>(obj)->getReactivePower(); }, puMW } },
};

static const std::map<std::string, fobjectPair> genFunctions
{
	{ "real",{ [](coreObject *obj) {return static_cast<gridDynGenerator *>(obj)->getRealPower(); }, puMW } },
	{ "reactive",{ [](coreObject *obj) {return static_cast<gridDynGenerator *>(obj)->getReactivePower(); }, puMW } },
	{ "genreal",{ [](coreObject *obj) {return static_cast<gridDynGenerator *>(obj)->getRealPower(); },puMW } },
	{ "genreactive",{ [](coreObject *obj) {return static_cast<gridDynGenerator *>(obj)->getReactivePower(); }, puMW } },
	{ "pset",{[](coreObject *obj) {return static_cast<gridDynGenerator *>(obj)->getPset(); },puMW } },
	{ "adjup",{ [](coreObject *obj) {return static_cast<gridDynGenerator *>(obj)->getAdjustableCapacityUp(); },puMW } },
	{ "adjdown",{ [](coreObject *obj) {return static_cast<gridDynGenerator *>(obj)->getAdjustableCapacityDown(); },puMW } },
	{ "pmax",{ [](coreObject *obj) {return static_cast<gridDynGenerator *>(obj)->getPmax(); },puMW } },
	{ "pmin",{ [](coreObject *obj) {return static_cast<gridDynGenerator *>(obj)->getPmin(); },puMW } },
	{ "qmax",{ [](coreObject *obj) {return static_cast<gridDynGenerator *>(obj)->getQmax(); },puMW } },
	{ "qmin",{ [](coreObject *obj) {return static_cast<gridDynGenerator *>(obj)->getQmin(); },puMW } },
	{"freq",{ [](coreObject *obj) {return static_cast<gridDynGenerator *>(obj)->getFreq(emptyStateData,cLocalSolverMode); },puHz } },
	{ "angle",{ [](coreObject *obj) {return static_cast<gridDynGenerator *>(obj)->getAngle(emptyStateData,cLocalSolverMode); },rad } },
};

static const std::map<std::string, fobjectPair> linkFunctions
{
	{ "real",{ [](coreObject *obj) {return static_cast<gridLink *>(obj)->getRealPower(1); }, puMW } },
	{ "reactive",{ [](coreObject *obj) {return static_cast<gridLink *>(obj)->getReactivePower(1); }, puMW } },
	{ "linkreal",{ [](coreObject *obj) {return static_cast<gridLink *>(obj)->getRealPower(1); },puMW } },
	{ "linkreactive",{ [](coreObject *obj) {return static_cast<gridLink *>(obj)->getReactivePower(1); }, puMW } },
	{ "z",{ [](coreObject *obj) {return static_cast<gridLink *>(obj)->getTotalImpedance(1); },puOhm } },
	{ "y",{ [](coreObject *obj) {return 1.0/(static_cast<gridLink *>(obj)->getTotalImpedance(1)); }, puOhm } },
	{ "r",{ [](coreObject *obj) {return static_cast<gridLink *>(obj)->getRealImpedance(1); },puOhm } },
	{ "x",{ [](coreObject *obj) {return 1.0 / (static_cast<gridLink *>(obj)->getImagImpedance(1)); }, puOhm } },
	{ "current",{ [](coreObject *obj) {return (static_cast<gridLink *>(obj)->getCurrent(1)); }, puA } },
	{ "realcurrent",{ [](coreObject *obj) {return (static_cast<gridLink *>(obj)->getRealCurrent(1)); }, puA } },
	{ "imagcurrent",{ [](coreObject *obj) {return (static_cast<gridLink *>(obj)->getImagCurrent(1)); }, puA } },
	{ "voltage",{ [](coreObject *obj) {return (static_cast<gridLink *>(obj)->getVoltage(1)); }, puV } },
	{ "absangle",{ [](coreObject *obj) {return (static_cast<gridLink *>(obj)->getAbsAngle(1)); }, rad } },
	//functions for to side
	{ "real2",{ [](coreObject *obj) {return static_cast<gridLink *>(obj)->getRealPower(2); }, puMW } },
	{ "reactive2",{ [](coreObject *obj) {return static_cast<gridLink *>(obj)->getReactivePower(2); }, puMW } },
	{ "linkreal2",{ [](coreObject *obj) {return static_cast<gridLink *>(obj)->getRealPower(2); },puMW } },
	{ "linkreactive2",{ [](coreObject *obj) {return static_cast<gridLink *>(obj)->getReactivePower(2); }, puMW } },
	{ "z2",{ [](coreObject *obj) {return static_cast<gridLink *>(obj)->getTotalImpedance(2); },puOhm } },
	{ "y2",{ [](coreObject *obj) {return 1.0 / (static_cast<gridLink *>(obj)->getTotalImpedance(2)); }, puOhm } },
	{ "r2",{ [](coreObject *obj) {return static_cast<gridLink *>(obj)->getRealImpedance(2); },puOhm } },
	{ "x2",{ [](coreObject *obj) {return 1.0 / (static_cast<gridLink *>(obj)->getImagImpedance(2)); }, puOhm } },
	{ "current2",{ [](coreObject *obj) {return (static_cast<gridLink *>(obj)->getCurrent(2)); }, puA } },
	{ "realcurrent2",{ [](coreObject *obj) {return (static_cast<gridLink *>(obj)->getRealCurrent(2)); }, puA } },
	{ "imagcurrent2",{ [](coreObject *obj) {return (static_cast<gridLink *>(obj)->getImagCurrent(2)); }, puA } },
	{ "voltage2",{ [](coreObject *obj) {return (static_cast<gridLink *>(obj)->getVoltage(2)); }, puV } },
	{ "absangle2",{ [](coreObject *obj) {return (static_cast<gridLink *>(obj)->getAbsAngle(2)); }, rad } },

	//non numbered fields
	{ "angle",{ [](coreObject *obj) {return (static_cast<gridLink *>(obj)->getAngle()); }, rad } },
	{ "loss",{ [](coreObject *obj) {return (static_cast<gridLink *>(obj)->getLoss()); }, puMW } },
	{ "lossreactive",{ [](coreObject *obj) {return (static_cast<gridLink *>(obj)->getReactiveLoss()); }, puMW } },
	{ "attached",{ [](coreObject *obj) {return static_cast<double> (((!static_cast<gridLink *>(obj)->checkFlag(gridLink::switch1_open_flag)) || (!static_cast<gridLink *>(obj)->checkFlag(gridLink::switch2_open_flag))) && (static_cast<gridLink *>(obj)->isEnabled())); }, defUnit } },
};

/* *INDENT-ON* */

fobjectPair getObjectFunction(gridObject *, const std::string &field)
{
	
	std::string nfstr = mapFind(stringTranslate, field, field);
	auto funcfind = objectFunctions.find(nfstr);
	if (funcfind != objectFunctions.end())
	{
		return funcfind->second;
	}
	std::string fld;
	auto num = stringOps::trailingStringInt(field, fld);
	if ((fld == "output") || (fld == "o") || (fld == "out"))
	{
		return{ [num](coreObject *obj) {return static_cast<gridObject *>(obj)->getOutput(num); }, defUnit };
	}
	if ((fld == "deriv") || (fld == "dodt") || (fld == "doutdt"))
	{
		return{ [num](coreObject *obj) {return static_cast<gridObject *>(obj)->getDoutdt(noInputs, emptyStateData, cLocalbSolverMode, 0); }, defUnit };
	}
	
	return nullPair;
}

fobjectPair getObjectFunction(gridBus *, const std::string &field)
{
	std::string nfstr = mapFind(stringTranslate, field, field);

	auto funcfind = busFunctions.find(nfstr);
	if (funcfind != busFunctions.end())
	{
		return funcfind->second;
	}
	return getObjectFunction(static_cast<gridObject *>(nullptr), field);
	
}

fobjectPair getObjectFunction(gridLoad *, const std::string &field)
{
	std::string nfstr = mapFind(stringTranslate, field, field);
	auto funcfind = loadFunctions.find(nfstr);
	if (funcfind != loadFunctions.end())
	{
		return funcfind->second;
	}

	return getObjectFunction(static_cast<gridObject *>(nullptr), field);

}

fobjectPair getObjectFunction(gridDynGenerator *, const std::string &field)
{
	std::string nfstr = mapFind(stringTranslate, field, field);
	auto funcfind = genFunctions.find(nfstr);
	if (funcfind != genFunctions.end())
	{
		return funcfind->second;
	}

	return getObjectFunction(static_cast<gridObject *>(nullptr), field);
}

fobjectPair getObjectFunction(gridArea *, const std::string &field)
{
	std::string nfstr = mapFind(stringTranslate, field, field);
	auto funcfind = areaFunctions.find(nfstr);
	if (funcfind != areaFunctions.end())
	{
		return funcfind->second;
	}

	return getObjectFunction(static_cast<gridObject *>(nullptr), field);

}


fobjectPair getObjectFunction(gridLink *, const std::string &field)
{
	std::string nfstr = mapFind(stringTranslate, field, field);
	auto funcfind = linkFunctions.find(nfstr);
	if (funcfind != linkFunctions.end())
	{
		return funcfind->second;
	}

	return getObjectFunction(static_cast<gridObject *>(nullptr), field);


}




fobjectPair getObjectFunction(gridRelay *rel, const std::string &field)
{
	fobjectPair retPair(nullptr,defUnit);
	std::string nfstr = mapFind(stringTranslate, field, field);
		auto funcfind = objectFunctions.find(nfstr);
		if (funcfind != objectFunctions.end())
		{
			return funcfind->second;
		}
		std::string fld;
	int num = stringOps::trailingStringInt(field, fld, 0);
	if ((field == "cv") || (field == "currentvalue") || (field == "value") || (field == "output"))
	{
		retPair.first = [](coreObject *obj)->double {
			return static_cast<gridRelay *>(obj)->getOutput(0);
		};
	}
	else if (fld == "status")
	{
		retPair.first = [num](coreObject *obj) {
			return static_cast<double> (static_cast<gridRelay *>(obj)->getConditionStatus(num));
		};
	}
	else if ((fld == "output") || (fld == "o"))
	{ //TODO:: PT figure out why the basic getOutput function doesn't work here
		retPair.first = [=](coreObject *obj) {
			return static_cast<gridRelay *>(obj)->getOutput(noInputs,emptyStateData, cLocalSolverMode, num);
		};
	}
	else if ((fld == "block") || (fld == "b"))
	{
		if (dynamic_cast<sensor *> (rel))
		{
			retPair.first = [=](coreObject *obj) {
				return static_cast<sensor *> (obj)->getBlockOutput(emptyStateData, cLocalSolverMode, num);
			};
		}
	}
	else if ((fld == "condition") || (fld == "c"))
	{
		retPair.first = [num](coreObject *obj) {
			return static_cast<gridRelay *>(obj)->getConditionValue(num);
		};
	}
	else if ((fld == "input") || (fld == "i"))
	{
		if (dynamic_cast<sensor *> (rel))
		{
			retPair.first = [num](coreObject *obj) {
				return static_cast<sensor *> (obj)->getInput(emptyStateData, cLocalSolverMode, num);
			};
		}
	}
	else
	{
		if (dynamic_cast<sensor *> (rel))
		{
			//try to lookup named output for sensors
			index_t outIndex = static_cast<sensor *> (rel)->lookupOutput(field);
			if (outIndex != kNullLocation)
			{
				retPair.first = [outIndex](coreObject *obj) {
					return static_cast<sensor *> (obj)->getOutput(noInputs,emptyStateData, cLocalSolverMode, outIndex);
				};
			}
		}
	}

	return retPair;

}

fvecPair getObjectVectorFunction(gridObject *obj, const std::string & /*field*/)
{
	//TODO:: add a vector function for all the outputs
	static_cast<void>(obj);
	return nullVecPair;
}

fvecPair getObjectVectorFunction(gridArea *, const std::string &field)
{
	std::string nfstr = mapFind(stringTranslate, field, field);
	auto funcfind = areaVecFunctions.find(nfstr);
	if (funcfind != areaVecFunctions.end())
	{
		return funcfind->second;
	}
    return nullVecPair;
}


descVecFunc getObjectVectorDescFunction(gridObject *obj, const std::string & /*field*/)
{
	static_cast<void>(obj);
	return nullptr;
}

descVecFunc getObjectVectorDescFunction(gridArea *, const std::string &field)
{
	std::string nfstr = mapFind(stringTranslate, field, field);
	auto funcfind = areaVecDescFunctions.find(nfstr);
	if (funcfind != areaVecDescFunctions.end())
	{
		return funcfind->second;
	}
	return nullptr;
}