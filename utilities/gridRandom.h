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

#ifndef GRIDRANDOM_H_
#define GRIDRANDOM_H_

#include <memory>
#include <random>
/** abstract class defining a random distribution*/

class distributionObject
{
  public:
    explicit distributionObject (){};
    virtual ~distributionObject () = default;
    virtual double operator() () = 0;
    virtual void updateParameters (double arg1) = 0;
    virtual void updateParameters (double arg1, double arg2) = 0;
};

/** class defining random number generation*/
class gridRandom
{
  private:
    static std::mt19937 s_gen;  //!< generator  //May need to make a generator per thread
    static std::uniform_real_distribution<double> s_udist;
    static std::exponential_distribution<double> s_expdist;
    static std::normal_distribution<double> s_normdist;
    static std::lognormal_distribution<double> s_lnormdist;
    static std::extreme_value_distribution<double> s_evdist;
    static std::gamma_distribution<double> s_gammadist;
    static std::uniform_int_distribution<int> s_uintdist;
    static int seeded;

  public:
    static void setSeed (unsigned int seed);
    static void setSeed ();
    enum class dist_type_t
    {
        constant,
        uniform,
        exponential,
        normal,
        lognormal,
        extreme_value,
        gamma,
        uniform_int,
    };
    explicit gridRandom (dist_type_t dist = dist_type_t::normal, double arg1 = 0.0, double arg2 = 1.0);
    explicit gridRandom (const std::string &dist_name, double arg1 = 0.0, double arg2 = 1.0);

    void setDistribution (dist_type_t dist);
    dist_type_t getDistribution () const { return m_dist; }
    double operator() ();
    double generate ();
    void setParameters (double p1, double p2 = 1.0);
    static double randNumber (dist_type_t dist);
    static double randNumber (dist_type_t dist, double p1, double p2);

    std::pair<double, double> getPair ();
    std::vector<double> getNewValues (size_t count);
    void getNewValues (std::vector<double> &rvec, size_t count);

    static decltype (s_gen) &getEngine () { return s_gen; };

  private:
    std::unique_ptr<distributionObject> dobj;
    dist_type_t m_dist;
    double param1 = 0.0;
    double param2 = 1.0;
};


template <class DIST>
class randomDistributionObject2 : public distributionObject
{
  private:
    DIST dist;

  public:
    randomDistributionObject2 () {}
    explicit randomDistributionObject2 (double param1) : dist (param1) {}
    randomDistributionObject2 (double param1, double param2) : dist (param1, param2) {}
    virtual double operator() () override { return dist (gridRandom::getEngine ()); }
    virtual void updateParameters (double param1) override { dist = DIST (param1); }
    virtual void updateParameters (double param1, double param2) override { dist = DIST (param1, param2); }
};

template <class DIST>
class randomDistributionObject1 : public distributionObject
{
  private:
    DIST dist;

  public:
    randomDistributionObject1 () {}
    explicit randomDistributionObject1 (double param1) : dist (param1) {}
    virtual double operator() () override { return dist (gridRandom::getEngine ()); }
    virtual void updateParameters (double param1) override { dist = DIST (param1); }
    virtual void updateParameters (double param1, double) override { dist = DIST (param1); }
};

template <>
class randomDistributionObject1<void> : public distributionObject
{
  private:
    double arg1 = 0.0;

  public:
    randomDistributionObject1 () {}
    explicit randomDistributionObject1 (double param1) : arg1 (param1) {}
    virtual double operator() () override { return arg1; }
    virtual void updateParameters (double param1) override { arg1 = param1; }
    virtual void updateParameters (double param1, double) override { arg1 = param1; }
};

gridRandom::dist_type_t getDist (const std::string &dist_name);

#endif