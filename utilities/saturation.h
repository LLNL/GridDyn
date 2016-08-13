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
#include <functional>
#include <string>


class saturation
{
public:
  enum class satType_t
  {
    none, quadratic, scaled_quadratic, exponential,linear
  };
private:
  double s10 = 0;
  double s12 = 0;
  double A = 0;
  double B = 0;
  std::function<double(double)> satFunc;
  std::function<double(double)> dFunc;
public:
  satType_t type = satType_t::scaled_quadratic;

  saturation (int ntype = 2);
  saturation (satType_t sT);

  void setParam (double S1, double S2);
  void setParam (double V1, double S1, double V2, double S2);
  void setType (int ntype);
  void setType (satType_t sT);
  void setType (const std::string &stype);
  double operator() (double val);
  double compute (double val);
  double deriv (double val);
  double inv (double val);
private:
  void computeParam ();
  void loadFunctions ();
};

