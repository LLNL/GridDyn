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

#ifndef GRIDRANDOM_H_
#define GRIDRANDOM_H_

#include <functional>
#include <random>

class gridRandom
{
private:
  static std::mt19937 s_gen;
  static std::uniform_real_distribution<double> s_udist;
  static std::exponential_distribution<double> s_expdist;
  static std::normal_distribution<double> s_normdist;
  static std::lognormal_distribution<double> s_lnormdist;

public:
  static void setSeed (unsigned int seed);
  enum class dist_type_t
  {
    constant, uniform, exponential, normal,lognormal
  };
  gridRandom (dist_type_t dist = dist_type_t::normal);
  gridRandom (std::string &dist_name);

  void setDistribution (dist_type_t dist);
  dist_type_t getDistribution ()
  {
    return m_dist;
  }
  double getNewValue ();
  double getNewValue (double p1, double p2);
  double randNumber (dist_type_t dist);
  double randNumber (dist_type_t dist, double p1, double p2);
  std::vector<double> getNewValues (size_t count);
  std::vector<double> getNewvalues (double p1, double p2,size_t count);
  void getNewValues (std::vector<double> &rvec,size_t count);
  void getNewValues (double p1, double p2,std::vector<double> &rvec, size_t count);
private:
  dist_type_t m_dist;
  std::function<double ()> f1;
  std::function<double(double, double)> f2;
};

gridRandom::dist_type_t getDist (const std::string &dist_name);

#endif