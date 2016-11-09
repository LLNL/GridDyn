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
};

static const std::map<std::string, fobjectPair> objectFunctions
{
	{ "connected",{ [](gridCoreObject *obj) {return static_cast<double>(static_cast<gridObject *>(obj)->isConnected()); },defUnit } },
	{ "enabled",{ [](gridCoreObject *obj) {return static_cast<double>(static_cast<gridObject *>(obj)->enabled); },defUnit } },
	{ "armed",{ [](gridCoreObject *obj) {return static_cast<double>(static_cast<gridObject *>(obj)->isArmed()); },defUnit } },

};

static const std::map<std::string, fobjectPair> busFunctions
{
	{"v",{[](gridCoreObject *obj) {return static_cast<gridBus *>(obj)->getVoltage(); }, puV} },
	{ "angle",{ [](gridCoreObject *obj) {return static_cast<gridBus *>(obj)->getAngle(); }, rad } },
	{ "freq",{ [](gridCoreObject *obj) {return static_cast<gridBus *>(obj)->getFreq(); },puHz } },
	{ "genreal",{ [](gridCoreObject *obj) {return static_cast<gridBus *>(obj)->getGenerationReal(); }, puMW } },
	{ "genreactive",{ [](gridCoreObject *obj) {return static_cast<gridBus *>(obj)->getGenerationReactive(); }, puMW } },
	{ "loadreal",{ [](gridCoreObject *obj) {return static_cast<gridBus *>(obj)->getLoadReal(); }, puMW } },
	{ "loadreactive",{ [](gridCoreObject *obj) {return static_cast<gridBus *>(obj)->getLoadReactive(); }, puMW } },
	{ "linkreal",{ [](gridCoreObject *obj) {return static_cast<gridBus *>(obj)->getLinkReal(); }, puMW } },
	{ "linkreactive",{ [](gridCoreObject *obj) {return static_cast<gridBus *>(obj)->getLinkReactive(); }, puMW } },
};

static const std::map<std::string, fobjectPair> areaFunctions
{
	{ "avgfreq",{ [](gridCoreObject *obj) {return static_cast<gridArea *>(obj)->getAvgFreq(); },puHz } },
	{ "genreal",{ [](gridCoreObject *obj) {return static_cast<gridArea *>(obj)->getGenerationReal(); }, puMW } },
	{ "genreactive",{ [](gridCoreObject *obj) {return static_cast<gridArea *>(obj)->getGenerationReactive(); }, puMW } },
	{ "loadreal",{ [](gridCoreObject *obj) {return static_cast<gridArea *>(obj)->getLoadReal(); }, puMW } },
	{ "loadreactive",{ [](gridCoreObject *obj) {return static_cast<gridArea *>(obj)->getLoadReactive(); }, puMW } },
	{ "loss",{ [](gridCoreObject *obj) {return static_cast<gridArea *>(obj)->getLoss(); }, puMW } },
	{ "tieflow",{ [](gridCoreObject *obj) {return static_cast<gridArea *>(obj)->getTieFlowReal(); }, puMW } },
};

static const std::map<std::string, fvecPair> areaVecFunctions
{
	{ "v",{ [](gridCoreObject *obj,std::vector<double> &data) {return static_cast<gridArea *>(obj)->getVoltage(data); }, puV } },
	{ "angle",{ [](gridCoreObject *obj,std::vector<double> &data) {return static_cast<gridArea *>(obj)->getAngle(data); }, rad } },
	{ "freq",{ [](gridCoreObject *obj,std::vector<double> &data) {return static_cast<gridArea *>(obj)->getFreq(data); },puHz } },
	{ "busgenreal",{ [](gridCoreObject *obj,std::vector<double> &data) {return static_cast<gridArea *>(obj)->getBusGenerationReal(data); }, puMW } },
	{ "busgenreactive",{ [](gridCoreObject *obj,std::vector<double> &data) {return static_cast<gridArea *>(obj)->getBusGenerationReactive(data); }, puMW } },
	{ "busloadreal",{ [](gridCoreObject *obj,std::vector<double> &data) {return static_cast<gridArea *>(obj)->getBusLoadReal(data); }, puMW } },
	{ "busloadreactive",{ [](gridCoreObject *obj,std::vector<double> &data) {return static_cast<gridArea *>(obj)->getBusLoadReactive(data); }, puMW } },
	{ "linkreal",{ [](gridCoreObject *obj,std::vector<double> &data) {return static_cast<gridArea *>(obj)->getLinkRealPower(data); }, puMW } },
	{ "linkreactive",{ [](gridCoreObject *obj,std::vector<double> &data) {return static_cast<gridArea *>(obj)->getLinkReactivePower(data); }, puMW } },
	{ "linkloss",{ [](gridCoreObject *obj,std::vector<double> &data) {return static_cast<gridArea *>(obj)->getLinkLoss(data); }, puMW } },

};

static const std::map<std::string, descVecFunc> areaVecDescFunctions
{
	{ "v", [](gridCoreObject *obj,stringVec &desc) {return static_cast<gridArea *>(obj)->getBusName(desc); } },
	{ "angle", [](gridCoreObject *obj,stringVec &desc) {return static_cast<gridArea *>(obj)->getBusName(desc); } },
	{ "freq", [](gridCoreObject *obj,stringVec &desc) {return static_cast<gridArea *>(obj)->getBusName(desc); } },
	{ "busgenreal", [](gridCoreObject *obj,stringVec &desc) {return static_cast<gridArea *>(obj)->getBusName(desc); } },
	{ "busgenreactive", [](gridCoreObject *obj,stringVec &desc) {return static_cast<gridArea *>(obj)->getBusName(desc); } },
	{ "busloadreal", [](gridCoreObject *obj,stringVec &desc) {return static_cast<gridArea *>(obj)->getBusName(desc); } },
	{ "busloadreactive", [](gridCoreObject *obj,stringVec &desc) {return static_cast<gridArea *>(obj)->getBusName(desc); } },
	{ "linkreal", [](gridCoreObject *obj,stringVec &desc) {return static_cast<gridArea *>(obj)->getLinkName(desc);  } },
	{ "linkreactive", [](gridCoreObject *obj,stringVec &desc) {return static_cast<gridArea *>(obj)->getLinkName(desc); } },
	{ "linkloss", [](gridCoreObject *obj,stringVec &desc) {return static_cast<gridArea *>(obj)->getLinkName(desc);  } },

};

static const std::map<std::string, fobjectPair> loadFunctions
{
	{ "real",{ [](gridCoreObject *obj) {return static_cast<gridLoad *>(obj)->getRealPower(); }, puMW } },
	{ "reactive",{ [](gridCoreObject *obj) {return static_cast<gridLoad *>(obj)->getReactivePower(); }, puMW } },
	{ "loadreal",{ [](gridCoreObject *obj) {return static_cast<gridLoad *>(obj)->getRealPower(); },puMW } },
	{ "loadreactive",{ [](gridCoreObject *obj) {return static_cast<gridLoad *>(obj)->getReactivePower(); }, puMW } },
};

static const std::map<std::string, fobjectPair> genFunctions
{
	{ "real",{ [](gridCoreObject *obj) {return static_cast<gridDynGenerator *>(obj)->getRealPower(); }, puMW } },
	{ "reactive",{ [](gridCoreObject *obj) {return static_cast<gridDynGenerator *>(obj)->getReactivePower(); }, puMW } },
	{ "genreal",{ [](gridCoreObject *obj) {return static_cast<gridDynGenerator *>(obj)->getRealPower(); },puMW } },
	{ "genreactive",{ [](gridCoreObject *obj) {return static_cast<gridDynGenerator *>(obj)->getReactivePower(); }, puMW } },
	{ "pset",{[](gridCoreObject *obj) {return static_cast<gridDynGenerator *>(obj)->getPset(); },puMW } },
	{ "adjup",{ [](gridCoreObject *obj) {return static_cast<gridDynGenerator *>(obj)->getAdjustableCapacityUp(); },puMW } },
	{ "adjdown",{ [](gridCoreObject *obj) {return static_cast<gridDynGenerator *>(obj)->getAdjustableCapacityDown(); },puMW } },
	{ "pmax",{ [](gridCoreObject *obj) {return static_cast<gridDynGenerator *>(obj)->getPmax(); },puMW } },
	{ "pmin",{ [](gridCoreObject *obj) {return static_cast<gridDynGenerator *>(obj)->getPmin(); },puMW } },
	{ "qmax",{ [](gridCoreObject *obj) {return static_cast<gridDynGenerator *>(obj)->getQmax(); },puMW } },
	{ "qmin",{ [](gridCoreObject *obj) {return static_cast<gridDynGenerator *>(obj)->getQmin(); },puMW } },
	{"freq",{ [](gridCoreObject *obj) {return static_cast<gridDynGenerator *>(obj)->getFreq(nullptr,cLocalSolverMode); },puHz } },
	{ "angle",{ [](gridCoreObject *obj) {return static_cast<gridDynGenerator *>(obj)->getAngle(nullptr,cLocalSolverMode); },rad } },
};

static const std::map<std::string, fobjectPair> linkFunctions
{
	{ "real",{ [](gridCoreObject *obj) {return static_cast<gridLink *>(obj)->getRealPower(1); }, puMW } },
	{ "reactive",{ [](gridCoreObject *obj) {return static_cast<gridLink *>(obj)->getReactivePower(1); }, puMW } },
	{ "linkreal",{ [](gridCoreObject *obj) {return static_cast<gridLink *>(obj)->getRealPower(1); },puMW } },
	{ "linkreactive",{ [](gridCoreObject *obj) {return static_cast<gridLink *>(obj)->getReactivePower(1); }, puMW } },
	{ "z",{ [](gridCoreObject *obj) {return static_cast<gridLink *>(obj)->getTotalImpedance(1); },puOhm } },
	{ "y",{ [](gridCoreObject *obj) {return 1.0/(static_cast<gridLink *>(obj)->getTotalImpedance(1)); }, puOhm } },
	{ "r",{ [](gridCoreObject *obj) {return static_cast<gridLink *>(obj)->getRealImpedance(1); },puOhm } },
	{ "x",{ [](gridCoreObject *obj) {return 1.0 / (static_cast<gridLink *>(obj)->getImagImpedance(1)); }, puOhm } },
	{ "current",{ [](gridCoreObject *obj) {return (static_cast<gridLink *>(obj)->getCurrent(1)); }, puA } },
	{ "realcurrent",{ [](gridCoreObject *obj) {return (static_cast<gridLink *>(obj)->getRealCurrent(1)); }, puA } },
	{ "imagcurrent",{ [](gridCoreObject *obj) {return (static_cast<gridLink *>(obj)->getImagCurrent(1)); }, puA } },
	{ "voltage",{ [](gridCoreObject *obj) {return (static_cast<gridLink *>(obj)->getVoltage(1)); }, puV } },
	{ "absangle",{ [](gridCoreObject *obj) {return (static_cast<gridLink *>(obj)->getAbsAngle(1)); }, rad } },
	//functions for to side
	{ "real2",{ [](gridCoreObject *obj) {return static_cast<gridLink *>(obj)->getRealPower(2); }, puMW } },
	{ "reactive2",{ [](gridCoreObject *obj) {return static_cast<gridLink *>(obj)->getReactivePower(2); }, puMW } },
	{ "linkreal2",{ [](gridCoreObject *obj) {return static_cast<gridLink *>(obj)->getRealPower(2); },puMW } },
	{ "linkreactive2",{ [](gridCoreObject *obj) {return static_cast<gridLink *>(obj)->getReactivePower(2); }, puMW } },
	{ "z2",{ [](gridCoreObject *obj) {return static_cast<gridLink *>(obj)->getTotalImpedance(2); },puOhm } },
	{ "y2",{ [](gridCoreObject *obj) {return 1.0 / (static_cast<gridLink *>(obj)->getTotalImpedance(2)); }, puOhm } },
	{ "r2",{ [](gridCoreObject *obj) {return static_cast<gridLink *>(obj)->getRealImpedance(2); },puOhm } },
	{ "x2",{ [](gridCoreObject *obj) {return 1.0 / (static_cast<gridLink *>(obj)->getImagImpedance(2)); }, puOhm } },
	{ "current2",{ [](gridCoreObject *obj) {return (static_cast<gridLink *>(obj)->getCurrent(2)); }, puA } },
	{ "realcurrent2",{ [](gridCoreObject *obj) {return (static_cast<gridLink *>(obj)->getRealCurrent(2)); }, puA } },
	{ "imagcurrent2",{ [](gridCoreObject *obj) {return (static_cast<gridLink *>(obj)->getImagCurrent(2)); }, puA } },
	{ "voltage2",{ [](gridCoreObject *obj) {return (static_cast<gridLink *>(obj)->getVoltage(2)); }, puV } },
	{ "absangle2",{ [](gridCoreObject *obj) {return (static_cast<gridLink *>(obj)->getAbsAngle(2)); }, rad } },

	//non numbered fields
	{ "angle",{ [](gridCoreObject *obj) {return (static_cast<gridLink *>(obj)->getAngle()); }, rad } },
	{ "loss",{ [](gridCoreObject *obj) {return (static_cast<gridLink *>(obj)->getLoss()); }, puMW } },
	{ "lossreactive",{ [](gridCoreObject *obj) {return (static_cast<gridLink *>(obj)->getReactiveLoss()); }, puMW } },
	{ "attached",{ [](gridCoreObject *obj) {return static_cast<double> (((!static_cast<gridLink *>(obj)->checkFlag(gridLink::switch1_open_flag)) || (!static_cast<gridLink *>(obj)->checkFlag(gridLink::switch2_open_flag))) && (static_cast<gridLink *>(obj)->enabled)); }, defUnit } },
};

/* *INDENT-ON* */

fobjectPair getObjectFunction(gridObject *, const std::string &field)
{
	auto fnd = stringTranslate.find(field);
	std::string nfstr = (fnd == stringTranslate.end()) ? field : fnd->second;

	auto funcfind = objectFunctions.find(nfstr);
	if (funcfind != objectFunctions.end())
	{
		return funcfind->second;
	}
	return nullPair;

}

fobjectPair getObjectFunction(gridBus *, const std::string &field)
{
	auto fnd = stringTranslate.find(field);
	std::string nfstr = (fnd == stringTranslate.end()) ? field : fnd->second;

	auto funcfind = busFunctions.find(nfstr);
	if (funcfind != busFunctions.end())
	{
		return funcfind->second;
	}

	funcfind = objectFunctions.find(nfstr);
	if (funcfind != objectFunctions.end())
	{
		return funcfind->second;
	}

	return nullPair;
}

fobjectPair getObjectFunction(gridLoad *, const std::string &field)
{
	auto fnd = stringTranslate.find(field);
	std::string nfstr = (fnd == stringTranslate.end()) ? field : fnd->second;
	auto funcfind = loadFunctions.find(nfstr);
	if (funcfind != loadFunctions.end())
	{
		return funcfind->second;
	}

	funcfind = objectFunctions.find(nfstr);
	if (funcfind != objectFunctions.end())
	{
		return funcfind->second;
	}

	return nullPair;

}

fobjectPair getObjectFunction(gridDynGenerator *, const std::string &field)
{
	auto fnd = stringTranslate.find(field);
	std::string nfstr = (fnd == stringTranslate.end()) ? field : fnd->second;
	auto funcfind = genFunctions.find(nfstr);
	if (funcfind != genFunctions.end())
	{
		return funcfind->second;
	}

	funcfind = objectFunctions.find(nfstr);
	if (funcfind != objectFunctions.end())
	{
		return funcfind->second;
	}

	return nullPair;
}

fobjectPair getObjectFunction(gridArea *, const std::string &field)
{
	auto fnd = stringTranslate.find(field);
	std::string nfstr = (fnd == stringTranslate.end()) ? field : fnd->second;
	auto funcfind = areaFunctions.find(nfstr);
	if (funcfind != areaFunctions.end())
	{
		return funcfind->second;
	}

	funcfind = objectFunctions.find(nfstr);
	if (funcfind != objectFunctions.end())
	{
		return funcfind->second;
	}

	return nullPair;

}


fobjectPair getObjectFunction(gridLink *, const std::string &field)
{
	auto fnd = stringTranslate.find(field);
	std::string nfstr = (fnd == stringTranslate.end()) ? field : fnd->second;
	auto funcfind = linkFunctions.find(nfstr);
	if (funcfind != linkFunctions.end())
	{
		return funcfind->second;
	}

	funcfind = objectFunctions.find(nfstr);
	if (funcfind != objectFunctions.end())
	{
		return funcfind->second;
	}

	return nullPair;


}




fobjectPair getObjectFunction(gridRelay *rel, const std::string &field)
{
	fobjectPair retPair(nullptr,defUnit);
	auto fnd = stringTranslate.find(field);
	std::string nfstr = (fnd == stringTranslate.end()) ? field : fnd->second;
		auto funcfind = objectFunctions.find(nfstr);
		if (funcfind != objectFunctions.end())
		{
			return funcfind->second;
		}
		std::string fld;
	int num = trailingStringInt(field, fld, 0);
	if ((field == "cv") || (field == "currentvalue") || (field == "value") || (field == "output"))
	{
		retPair.first = [](gridCoreObject *obj)->double {
			return static_cast<gridRelay *>(obj)->getOutput(0);
		};
	}
	else if (fld == "status")
	{
		retPair.first = [num](gridCoreObject *obj) {
			return static_cast<double> (static_cast<gridRelay *>(obj)->getConditionStatus(num));
		};
	}
	else if ((fld == "output") || (fld == "o"))
	{ //TODO:: PT figure out why the basic getOutput function doesn't work here
		retPair.first = [=](gridCoreObject *obj) {
			return static_cast<gridRelay *>(obj)->getOutput(nullptr, cLocalSolverMode, num);
		};
	}
	else if ((fld == "block") || (fld == "b"))
	{
		if (dynamic_cast<sensor *> (rel))
		{
			retPair.first = [=](gridCoreObject *obj) {
				return static_cast<sensor *> (obj)->getBlockOutput(nullptr, cLocalSolverMode, num);
			};
		}
	}
	else if ((fld == "condition") || (fld == "c"))
	{
		retPair.first = [num](gridCoreObject *obj) {
			return static_cast<gridRelay *>(obj)->getConditionValue(num);
		};
	}
	else if ((fld == "input") || (fld == "i"))
	{
		if (dynamic_cast<sensor *> (rel))
		{
			retPair.first = [num](gridCoreObject *obj) {
				return static_cast<sensor *> (obj)->getInput(nullptr, cLocalSolverMode, num);
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
				retPair.first = [outIndex](gridCoreObject *obj) {
					return static_cast<sensor *> (obj)->getOutput(nullptr, cLocalSolverMode, outIndex);
				};
			}
		}
	}

	return retPair;

}

fvecPair getObjectVectorFunction(gridObject *obj, const std::string & /*field*/)
{
	static_cast<void>(obj);
	return nullVecPair;
}

fvecPair getObjectVectorFunction(gridArea *, const std::string &field)
{
	auto fnd = stringTranslate.find(field);
	std::string nfstr = (fnd == stringTranslate.end()) ? field : fnd->second;
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
	auto fnd = stringTranslate.find(field);
	std::string nfstr = (fnd == stringTranslate.end()) ? field : fnd->second;
	auto funcfind = areaVecDescFunctions.find(nfstr);
	if (funcfind != areaVecDescFunctions.end())
	{
		return funcfind->second;
	}
	return nullptr;
}