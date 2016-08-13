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

#include "gridRandom.h"

#include <ctime>
#include <algorithm>

std::mt19937 gridRandom::s_gen;
std::uniform_real_distribution<double> gridRandom::s_udist;
std::exponential_distribution<double> gridRandom::s_expdist;
std::normal_distribution<double> gridRandom::s_normdist;
std::lognormal_distribution<double> gridRandom::s_lnormdist;

gridRandom::gridRandom (std::string &dist_name)
{
  setDistribution (getDist (dist_name));
  s_gen.seed (static_cast<unsigned int> (time (0)));
}

gridRandom::gridRandom (dist_type_t dist)
{
  setDistribution (dist);
  s_gen.seed (static_cast<unsigned int> (time (0)));
}

void gridRandom::setSeed (unsigned int seed)
{
  s_gen.seed (seed);
}

void gridRandom::setDistribution (dist_type_t dist)
{
  m_dist = dist;
  switch (dist)
    {
    case dist_type_t::constant:
      f1 = [](){
          return 1.0;
        };
      f2 = [](double arg1, double ){
          return arg1;
        };
      break;
    case dist_type_t::exponential:
      f1 = [ = ](){
          return s_expdist (s_gen);
        };
      f2 = [ = ](double arg1, double ){
          return arg1 * s_expdist (s_gen);
        };
      break;
    case dist_type_t::normal:
      f1 = [ = ](){
          return s_normdist (s_gen);
        };
      f2 = [ = ](double arg1, double arg2){
          return s_normdist (s_gen) * arg2 + arg1;
        };
      break;
    case dist_type_t::uniform:
      f1 = [ = ](){
          return s_udist (s_gen);
        };
      f2 = [ = ](double arg1, double arg2){
          return s_udist (s_gen) * (arg2 - arg1) + arg1;
        };
      break;
    case dist_type_t::lognormal:
      f1 = [ = ](){
          return s_lnormdist (s_gen);
        };
      f2 = [ = ](double arg1, double arg2){
          return s_lnormdist (s_gen) * arg2 + arg1;
        };
      break;
    }
}

double gridRandom::randNumber (dist_type_t dist)
{
  switch (dist)
    {
    case dist_type_t::constant:
      return 1.0;
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
      break;
    default:
      return 0.0;
    }
}

double gridRandom::randNumber (dist_type_t dist, double arg1, double arg2)
{
  switch (dist)
    {
    case dist_type_t::constant:
      return arg1;
      break;
    case dist_type_t::exponential:
      return arg1 * s_expdist (s_gen);
      break;
    case dist_type_t::normal:
      return s_normdist (s_gen) * arg2 + arg1;
      break;
    case dist_type_t::uniform:
      return s_udist (s_gen) * (arg2 - arg1) + arg1;
      break;
    case dist_type_t::lognormal:
      return s_lnormdist (s_gen) * arg2 + arg1;
      break;
    default:
      return 0.0;
    }
}

double gridRandom::getNewValue ()
{
  return f1 ();
}

double gridRandom::getNewValue (double p1, double p2)
{
  return f2 (p1, p2);
}

std::vector<double> gridRandom::getNewValues (size_t count)
{
  std::vector<double> rv (count);
  std::generate (rv.begin (),rv.end (),f1);
  return rv;
}

std::vector<double> gridRandom::getNewvalues (double p1, double p2, size_t count)
{
  std::vector<double> rv (count);
  std::generate (rv.begin (), rv.end (), [ = ] {return f2 (p1,p2);
                 });
  return rv;
}

void gridRandom::getNewValues (std::vector<double> &rvec, size_t count)
{
  std::generate (rvec.begin (), rvec.begin () + count - 1, f1);
}

void gridRandom::getNewValues (double p1, double p2, std::vector<double> &rvec, size_t count)
{
  std::generate (rvec.begin (), rvec.begin () + count - 1, [ = ] {return f2 (p1, p2);
                 });
}


gridRandom::dist_type_t getDist (const std::string &dist_name)
{
  gridRandom::dist_type_t ndist = gridRandom::dist_type_t::constant;
  if (dist_name == "constant")
    {
      ndist = gridRandom::dist_type_t::constant;
    }
  if (dist_name == "uniform")
    {
      ndist = gridRandom::dist_type_t::uniform;
    }
  if (dist_name == "lognormal")
    {
      ndist = gridRandom::dist_type_t::lognormal;
    }
  else if (dist_name == "normal")
    {
      ndist = gridRandom::dist_type_t::normal;
    }
  else if (dist_name == "exponential")
    {
      ndist = gridRandom::dist_type_t::exponential;
    }
  return ndist;

}
