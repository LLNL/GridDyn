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

#include "fncsLibrary.h"
#include "fncsCollector.h"
#include "fncsLoad.h"
#include "fncsSource.h"
#include "core/factoryTemplates.h"
#include "objectFactoryTemplates.h"

#include "fncs.hpp"
#include <regex>
#include <sstream>
#include "stringOps.h"

static childClassFactory<fncsCollector, collector> fncsFac(std::vector<std::string> {"fncs"});

static childTypeFactory<fncsSource, rampSource> fnsrc("source", stringVec{ "fncs" });
static childTypeFactory<fncsLoad, gridRampLoad> fnld("load", "fncs");

void loadFNCSLibrary()
{
	static int loaded = 0;

	if (loaded == 0)
	{
		loaded = 1;
	}
}

void fncsSendComplex(const std::string &key, double real, double imag)
{
	std::complex<double> cv(real, imag);
	std::stringstream ss;
	ss << std::to_string(real);
	if (imag > 0)
	{
		ss << '+' << std::to_string(imag);
	}
	else
	{
		ss << std::to_string(imag);
	}
	ss << 'j';
	fncs::publish(key, ss.str());
}

void fncsSendComplex(const std::string &key, std::complex<double> val)
{
	fncsSendComplex(key, val.real(), val.imag());
}

const std::regex creg("([+-]?(\\d+(\\.\\d+)?|\\.\\d+)([eE][+-]?\\d+)?)\\s*([+-]\\s*(\\d+(\\.\\d+)?|\\.\\d+)([eE][+-]?\\d+)?)[ji]*");

std::complex<double> fncsGetComplex(const std::string &key)
{
	std::string s=fncs::get_value(key);
	if (s.empty())
	{
		return std::complex<double>(kNullVal, kNullVal);
	}
	std::smatch m;
	double re = 0;
	double im = 0;
	std::regex_search(s, m, creg);
	if (m.size() == 9)
	{
		re = doubleReadComplete(m[1], kNullVal);
		im = doubleReadComplete(m[5], kNullVal);
		
	}
	else
	{
		if ((s.back() == 'j') || (s.back() == 'i'))
		{
			s.pop_back();
			im = doubleReadComplete(s, kNullVal);

		}
		else
		{
			re = doubleReadComplete(s, kNullVal);
		}
	}
	return std::complex<double>(re, im);
}

void fncsSendVal(const std::string &key, double val)
{
	fncs::publish(key, std::to_string(val));
}

double fncsGetVal(const std::string &key)
{
	std::string s= fncs::get_value(key);
	return doubleReadComplete(s, kNullVal);
}