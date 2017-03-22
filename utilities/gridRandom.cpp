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

#include "gridRandom.h"
#include "mapOps.h"
#include "vectorOps.hpp"
#include <algorithm>
#include <ctime>

std::mt19937 gridRandom::s_gen;
std::uniform_real_distribution<double> gridRandom::s_udist;
std::exponential_distribution<double> gridRandom::s_expdist;
std::normal_distribution<double> gridRandom::s_normdist;
std::lognormal_distribution<double> gridRandom::s_lnormdist;
std::gamma_distribution<double> gridRandom::s_gammadist;
std::extreme_value_distribution<double> gridRandom::s_evdist;
std::uniform_int_distribution<int> gridRandom::s_uintdist;

int gridRandom::seeded = -1;


gridRandom::gridRandom (dist_type_t dist , double arg1, double arg2):param1(arg1),param2(arg2)
{
  setDistribution (dist);
  if (seeded < 0)
  {
	  s_gen.seed(static_cast<unsigned int> (time(0)));
	  seeded = 1;
  }
  
}

gridRandom::gridRandom(const std::string &dist_name, double arg1, double arg2):gridRandom(getDist(dist_name),arg1,arg2)
{
	
}

void gridRandom::setParameters(double p1, double p2)
{
	param1 = p1;
	param2 = p2;
	dobj->updateParameters(p1, p2);
}

void gridRandom::setSeed (unsigned int seed)
{
  s_gen.seed (seed);
}

void gridRandom::setSeed()
{
	s_gen.seed(static_cast<unsigned int> (time(0)));
}


void gridRandom::setDistribution (dist_type_t dist)
{
  m_dist = dist;
  switch (dist)
    {
    case dist_type_t::constant:
		dobj = std::make_unique<randomDistributionObject1<void>>(param1);
      break;
    case dist_type_t::exponential:
		dobj = std::make_unique<randomDistributionObject1<std::exponential_distribution<double>>>(1.0/param1);
      break;
	case dist_type_t::extreme_value:
		dobj = std::make_unique<randomDistributionObject2<std::extreme_value_distribution<double>>>(param1,param2);
		break;
	case dist_type_t::gamma:
		dobj = std::make_unique<randomDistributionObject2<std::gamma_distribution<double>>>(param1, param2);
		break;
    case dist_type_t::normal:
		dobj = std::make_unique<randomDistributionObject2<std::normal_distribution<double>>>(param1, param2);
      break;
    case dist_type_t::uniform:
		dobj = std::make_unique<randomDistributionObject2<std::uniform_real_distribution<double>>>(param1, param2);
      break;
    case dist_type_t::lognormal:
		dobj = std::make_unique<randomDistributionObject2<std::lognormal_distribution<double>>>(param1, param2);
		break;
	case dist_type_t::uniform_int:
		dobj = std::make_unique<randomDistributionObject2<std::uniform_int_distribution<int>>>(static_cast<int>(param1), static_cast<int>(param2));
      break;
    }
}

double gridRandom::randNumber (dist_type_t dist)
{
  switch (dist)
    {
    case dist_type_t::constant:
	default:
      return 0.0;
      break;
	case dist_type_t::gamma:
		return s_gammadist(s_gen);
		break;
	case dist_type_t::extreme_value:
		return s_evdist(s_gen);
		break;
    case dist_type_t::exponential:
      return s_expdist (s_gen);
      break;
    case dist_type_t::normal:
      return s_normdist (s_gen);
      break;
    case dist_type_t::uniform:
      return s_udist (s_gen);
      break;
    case dist_type_t::lognormal:
      return s_lnormdist (s_gen);
	case dist_type_t::uniform_int:
		return static_cast<double>(s_uintdist(s_gen));
      break;
    }
}

double gridRandom::randNumber (dist_type_t dist, double arg1, double arg2)
{
  switch (dist)
    {
    case dist_type_t::constant:
      return arg1;
      break;
	case dist_type_t::gamma:
		return s_gammadist(s_gen,std::gamma_distribution<double>::param_type(arg1,arg2));
		break;
	case dist_type_t::extreme_value:
		return s_evdist(s_gen, std::extreme_value_distribution<double>::param_type(arg1, arg2));
		break;
    case dist_type_t::exponential:
      return s_expdist (s_gen, std::exponential_distribution<double>::param_type(1.0/arg1));
      break;
    case dist_type_t::normal:
      return s_normdist (s_gen, std::normal_distribution<double>::param_type(arg1, arg2));
      break;
    case dist_type_t::uniform:
      return s_udist (s_gen, std::uniform_real_distribution<double>::param_type(arg1, arg2));
      break;
    case dist_type_t::lognormal:
      return s_lnormdist (s_gen, std::lognormal_distribution<double>::param_type(arg1, arg2));
      break;
	case dist_type_t::uniform_int:
		return static_cast<double>(s_uintdist(s_gen, std::uniform_int_distribution<int>::param_type(static_cast<int>(arg1), static_cast<int>(arg2))));
    default:
      return 0.0;
    }
}

double gridRandom::operator()()
{
	return generate();
}

double gridRandom::generate()
{
	return (*dobj)();
}

std::vector<double> gridRandom::getNewValues (size_t count)
{
  std::vector<double> rv (count);
  std::generate(rv.begin(), rv.end(), [=]() {return (*dobj)(); });
  return rv;
}


void gridRandom::getNewValues (std::vector<double> &rvec, size_t count)
{
	ensureSizeAtLeast(rvec, count);
  std::generate (rvec.begin (), rvec.begin () + count - 1, [=]() {return (*dobj)(); });
}


std::pair<double, double> gridRandom::getPair()
{
	return std::make_pair((*dobj)(), (*dobj)());
}

static const std::map<std::string, gridRandom::dist_type_t> distmap
{
	{"constant", gridRandom::dist_type_t::constant},
	{ "const", gridRandom::dist_type_t::constant },
	{ "uniform", gridRandom::dist_type_t::uniform },
	{ "lognormal", gridRandom::dist_type_t::lognormal },
	{ "extreme", gridRandom::dist_type_t::extreme_value },
	{ "exponential", gridRandom::dist_type_t::exponential },
	{ "gamma", gridRandom::dist_type_t::gamma },
	{ "normal", gridRandom::dist_type_t::normal },
	{ "gaussian", gridRandom::dist_type_t::normal },
	{ "guassian", gridRandom::dist_type_t::normal }, //common spelling mistake
	{"uniform_int",gridRandom::dist_type_t::uniform_int},
};


gridRandom::dist_type_t getDist (const std::string &dist_name)
{
	return mapFind(distmap, dist_name, gridRandom::dist_type_t::constant);
}
