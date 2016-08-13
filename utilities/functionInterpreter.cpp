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

//disable a funny warning (bug in visual studio 2015)
#ifdef _MSC_VER
#if _MSC_VER >= 1900
#pragma warning( disable : 4592)
#endif
#endif


#include "functionInterpreter.h"
#include "vectorOps.hpp"
#include "gridRandom.h"
#include "stringOps.h"
#include <map>
#include <algorithm>
#include <cmath>

static gridRandom rnumGen;
static const double local_pi= 3.141592653589793;
static const double local_nan = std::nan("0");
static const double local_inf = 1e48;

static std::map <std::string, std::function<double()> >  FuncList0
{
	/* *INDENT-OFF* */
	std::make_pair("inf", []() {return local_inf; }),
	std::make_pair("nan", []() {return local_nan; }),
	std::make_pair("pi", []() {return local_pi; }),
	std::make_pair("rand", []() {return rnumGen.randNumber(gridRandom::dist_type_t::uniform); }),
	std::make_pair("randn", []() {return rnumGen.randNumber(gridRandom::dist_type_t::normal); }),
	std::make_pair("randexp", []() {return rnumGen.randNumber(gridRandom::dist_type_t::exponential); }),
	std::make_pair("randlogn", []() {return rnumGen.randNumber(gridRandom::dist_type_t::lognormal); }),
	/* *INDENT-ON* */
};


static std::map <std::string, std::function<double(double)> >  FuncList1
{
	/* *INDENT-OFF* */
	std::make_pair("sin", [](double val) {return sin(val); }),
	std::make_pair("cos", [](double val) {return cos(val); }),
	std::make_pair("tan", [](double val) {return tan(val); }),
	std::make_pair("sinh", [](double val) {return sinh(val); }),
	std::make_pair("cosh", [](double val) {return cosh(val); }),
	std::make_pair("tanh", [](double val) {return tanh(val); }),
	std::make_pair("abs", [](double val) {return std::abs(val); }),
	std::make_pair("sign", [](double val) {return (val > 0.0) ? (1.0) : ((val < 0.0) ? -1.0 : 0.0); }),
	std::make_pair("asin", [](double val) {return asin(val); }),
	std::make_pair("acos", [](double val) {return acos(val); }),
	std::make_pair("atan", [](double val) {return atan(val); }),
	std::make_pair("sqrt", [](double val) {return sqrt(val); }),
	std::make_pair("cbrt", [](double val) {return cbrt(val); }),
	std::make_pair("log", [](double val) {return log(val); }),
	std::make_pair("log10", [](double val) {return log10(val); }),
	std::make_pair("log2", [](double val) {return log2(val); }),
	std::make_pair("exp", [](double val) {return exp(val); }),
	std::make_pair("exp2", [](double val) {return exp2(val); }),
	std::make_pair("ceil", [](double val) {return ceil(val); }),
	std::make_pair("floor", [](double val) {return floor(val); }),
	std::make_pair("round", [](double val) {return round(val); }),
	std::make_pair("trunc", [](double val) {return trunc(val); }),
	std::make_pair("none", [](double val) {return val; }),
	std::make_pair("dec", [](double val) {double iptr; return std::modf(val, &iptr); }),
	std::make_pair("", [](double val) {return val; }),
	std::make_pair("randexp", [](double val) {return rnumGen.randNumber(gridRandom::dist_type_t::exponential,val,val); })
	/* *INDENT-ON* */
}; 

static std::map <std::string, std::function<double(double)> >  invFuncList1
{
	/* *INDENT-OFF* */
	std::make_pair("sin", [](double val) {return asin(val); }),
	std::make_pair("cos", [](double val) {return acos(val); }),
	std::make_pair("tan", [](double val) {return atan(val); }),
	std::make_pair("sinh", [](double val) {return asinh(val); }),
	std::make_pair("cosh", [](double val) {return acosh(val); }),
	std::make_pair("tanh", [](double val) {return atanh(val); }),
	std::make_pair("abs", [](double val) {return val; }),
	std::make_pair("sign", [](double val) {return (val > 0.0) ? (1.0) : ((val < 0.0) ? -1.0 : 0.0); }),
	std::make_pair("asin", [](double val) {return sin(val); }),
	std::make_pair("acos", [](double val) {return cos(val); }),
	std::make_pair("atan", [](double val) {return tan(val); }),
	std::make_pair("sqrt", [](double val) {return val*val; }),
	std::make_pair("cbrt", [](double val) {return val*val*val; }),
	std::make_pair("log", [](double val) {return exp(val); }),
	std::make_pair("log10", [](double val) {return pow(10,val); }),
	std::make_pair("log2", [](double val) {return pow(2,val); }),
	std::make_pair("exp", [](double val) {return log(val); }),
	std::make_pair("exp2", [](double val) {return log2(val); }),
	std::make_pair("ceil", [](double val) {return val; }),
	std::make_pair("floor", [](double val) {return val; }),
	std::make_pair("round", [](double val) {return val; }),
	std::make_pair("trunc", [](double val) {return val; }),
	std::make_pair("none", [](double val) {return val; }),
	std::make_pair("dec", [](double val) {return val; }),
	std::make_pair("", [](double val) {return val; }),
	/* *INDENT-ON* */
};


static std::map <std::string, std::function<double(double, double)> >  FuncList2
{
	/* *INDENT-OFF* */
	std::make_pair("atan2", [](double val1, double val2) {return atan2(val1, val2); }),
	std::make_pair("pow", [](double val1, double val2) {return pow(val1, val2); }),
	std::make_pair("+", [](double val1, double val2) {return val1 + val2; }),
	std::make_pair("-", [](double val1, double val2) {return val1 - val2; }),
	std::make_pair("/", [](double val1, double val2) {return val1 / val2; }),
	std::make_pair("*", [](double val1, double val2) {return val1 * val2; }),
	std::make_pair("^", [](double val1, double val2) {return pow(val1, val2); }),
	std::make_pair("%", [](double val1, double val2) {return fmod(val1, val2); }),
	std::make_pair("plus", [](double val1, double val2) {return val1 + val2; }),
	std::make_pair("minus", [](double val1, double val2) {return val1 - val2; }),
	std::make_pair("div", [](double val1, double val2) {return val1 / val2; }),
	std::make_pair("mult", [](double val1, double val2) {return val1 * val2; }),
	std::make_pair("product", [](double val1, double val2) {return val1 * val2; }),
	std::make_pair("add", [](double val1, double val2) {return val1 + val2; }),
	std::make_pair("subtract", [](double val1, double val2) {return val1 - val2; }),
	std::make_pair("max", [](double val1, double val2) {return std::max(val1, val2); }),
	std::make_pair("min", [](double val1, double val2) {return std::min(val1, val2); }),
	std::make_pair("mod", [](double val1, double val2) {return fmod(val1, val2); }),
	std::make_pair("hypot", [](double val1, double val2) {return hypot(val1, val2); }),
	std::make_pair("rand", [](double val1, double val2) {return rnumGen.randNumber(gridRandom::dist_type_t::uniform,val1,val2); }),
	std::make_pair("randn", [](double val1, double val2) {return rnumGen.randNumber(gridRandom::dist_type_t::normal,val1,val2); }),
	std::make_pair("randexp", [](double val1, double val2) {return rnumGen.randNumber(gridRandom::dist_type_t::exponential,val1,val2); }),
	std::make_pair("randlogn", [](double val1, double val2) {return rnumGen.randNumber(gridRandom::dist_type_t::lognormal,val1,val2); }),
	std::make_pair("randint", [](double val1, double val2) {return ceil(rnumGen.randNumber(gridRandom::dist_type_t::uniform, val1 - 1, val2)); }),
	/* *INDENT-ON* */
};

static std::map <std::string, std::function<double(std::vector<double>)> >  ArrFuncList1
{
	/* *INDENT-OFF* */
	std::make_pair("sum", [](std::vector<double> ar1) {return sum(ar1); }),
	std::make_pair("absmax", [](std::vector<double> ar1) {return absMax(ar1); }),
	std::make_pair("max", [](std::vector<double> ar1) {return *std::max_element(ar1.begin(), ar1.end()); }),
	std::make_pair("min", [](std::vector<double> ar1) {return *std::min_element(ar1.begin(), ar1.end()); }),
	std::make_pair("absmin", [](std::vector<double> ar1) {return absMin(ar1); }),
	std::make_pair("product", [](std::vector<double> ar1) {return product(ar1); }),
	std::make_pair("avg", [](std::vector<double> ar1) {return (sum(ar1) / static_cast<double> (ar1.size())); }),
	std::make_pair("stdev", [](std::vector<double> ar1) {return stdev(ar1); }),
	std::make_pair("median", [](std::vector<double> ar1) {return median(ar1); }),
	/* *INDENT-ON* */
};

static std::map <std::string, std::function<double(std::vector<double>, std::vector<double>)> >  ArrFuncList2
{
	/* *INDENT-OFF* */
	std::make_pair("vecprod", [](std::vector<double> ar1, std::vector<double> ar2) {return mult_sum(ar1, ar2); }),
	/* *INDENT-ON* */
};


bool isFunctionName(const std::string &ftest, function_type ftype)
{
	
	if ((ftype == function_type::all) || (ftype == function_type::arg))
	{
		auto ifind = FuncList1.find(ftest);
		if (ifind != FuncList1.end())
		{
			return true;
		}
	}
	if ((ftype == function_type::all) || (ftype == function_type::arg2))
	{
		auto ifind2 = FuncList2.find(ftest);
		if (ifind2 != FuncList2.end())
		{
			return true;
		}
	}
	if ((ftype == function_type::all) || (ftype == function_type::no_args))
	{
		auto ifind = FuncList0.find(ftest);
		if (ifind != FuncList0.end())
		{
			return true;
		}
	}
	if ((ftype == function_type::all) || (ftype == function_type::vect_arg))
	{
		auto ifind3 = ArrFuncList1.find(ftest);
		if (ifind3 != ArrFuncList1.end())
		{
			return true;
		}
	}
	if ((ftype == function_type::all) || (ftype == function_type::vect_arg2))
	{
		auto ifind4 = ArrFuncList2.find(ftest);
		if (ifind4 != ArrFuncList2.end())
		{
			return true;
		}
	}
	return false;

}


double evalFunction(const std::string &functionName)
{
	std::string temp = convertToLowerCase(functionName);
	auto fret = FuncList0.find(temp);
	return (fret != FuncList0.end()) ? fret->second() :local_nan;
}


double evalFunction(const std::string &functionName, double val)
{
	std::string temp = convertToLowerCase(functionName);
	auto fret = FuncList1.find(temp);
	return (fret != FuncList1.end()) ? fret->second(val) : local_nan;
}


double evalFunction(const std::string &functionName, double val1, double val2)
{
	std::string temp = convertToLowerCase(functionName);
	auto fret = FuncList2.find(temp);
	return (fret != FuncList2.end()) ? fret->second(val1,val2) : local_nan;
}


double evalFunction(const std::string &functionName, std::vector<double> arr)
{
	std::string temp = convertToLowerCase(functionName);

	auto fret = ArrFuncList1.find(temp);
	return (fret != ArrFuncList1.end()) ? fret->second(arr) : local_nan;
}


double evalFunction(const std::string &functionName, std::vector<double> arr1, std::vector<double> arr2)
{
	std::string temp = convertToLowerCase(functionName);

	auto fret = ArrFuncList2.find(temp);
	return (fret != ArrFuncList2.end()) ? fret->second(arr1,arr2) : local_nan;
}

std::function<double()> get0ArgFunction(const std::string &funcName)
{
	std::string temp = convertToLowerCase(funcName);
	auto fret = FuncList0.find(temp);
	return (fret != FuncList0.end()) ? fret->second : nullptr;

}

std::function<double(double)> get1ArgFunction(const std::string &funcName)
{
	std::string temp = convertToLowerCase(funcName);
	auto fret = FuncList1.find(temp);
	return (fret != FuncList1.end()) ? fret->second : nullptr;

}

std::function<double(double, double)> get2ArgFunction(const std::string &funcName)
{
	std::string temp = convertToLowerCase(funcName);

	auto fret = FuncList2.find(temp);
	return (fret != FuncList2.end()) ? fret->second : nullptr;

}

std::function<double(std::vector<double>)> getArrayFunction(const std::string &funcName)
{
	std::string temp = convertToLowerCase(funcName);

	auto fret = ArrFuncList1.find(temp);
	return (fret != ArrFuncList1.end()) ? fret->second : nullptr;
}

std::function<double(std::vector<double>, std::vector<double>)> get2ArrayFunction(const std::string &funcName)
{
	std::string temp = convertToLowerCase(funcName);

	auto fret = ArrFuncList2.find(temp);
	return (fret != ArrFuncList2.end()) ? fret->second : nullptr;

}