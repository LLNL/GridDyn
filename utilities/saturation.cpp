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
#include "saturation.h"

#include <cmath>

saturation::saturation (int ntype)
{
  switch (ntype)
    {
    case 0:
      type = satType_t::none;
      satFunc = [](double){
          return 0;
        };
      dFunc = [](double){
          return 0;
        };
      break;
    case 1:
      type = satType_t::quadratic;
      satFunc = [&](double val){
          return (B * (val - A) * (val - A));
        };
      dFunc = [&](double val){
          return (2 * B * (val - A));
        };
      break;
    case 2:
      type = satType_t::scaled_quadratic;
      satFunc = [&](double val){
          return (B * (val - A) * (val - A) / val);
        };
      dFunc = [&](double val){
          double v1 = (val - A);
          return (B * (2 * val * v1 - v1 * v1) / (val * val));
        };
      break;
    case 3:
      type = satType_t::exponential;
      satFunc = [&](double val){
          return (B * pow (val, A));
        };
      dFunc = [&](double val){
          return (A * B * pow (val, A - 1));
        };
      break;
    case 4:
      type = satType_t::linear;
      satFunc = [&](double val){
          return (val <= A) ? 0 : (B * (val - A));
        };
      dFunc = [&](double val){
          return (val <= A) ? 0 : B;
        };
      break;
    }

}

saturation::saturation (satType_t sT) : type (sT)
{


}

void saturation::setType (const std::string &stype)
{
  if (stype == "none")
    {
      type = satType_t::none;
    }
  else if (stype == "quadratic")
    {
      type = satType_t::quadratic;
    }
  else if (stype == "scaled_quadratic")
    {
      type = satType_t::scaled_quadratic;
    }
  else if (stype == "exponential")
    {
      type = satType_t::exponential;
    }
  else if (stype == "linear")
    {
      type = satType_t::linear;
    }
  loadFunctions ();
  computeParam ();
}

void saturation::setParam (double S1, double S2)
{
  s10 = S1;
  s12 = S2;
  computeParam ();
}

void saturation::setParam (double V1, double S1, double V2, double S2)
{
  double ssv;
  switch (type)
    {
    case satType_t::none:
      A = 0;
      B = 0;
      break;
    case satType_t::quadratic:
      ssv = sqrt (S1 / S2);
      A = -(V2 * ssv - V1) / (V1 - ssv);
      B = S1 / ((V1 - A) * (V1 - A));
      break;
    case satType_t::scaled_quadratic:
      ssv = sqrt ((S1 * V1) / (S2 * V2));
      A = -(V2 * ssv - V1) / (V1 - ssv);
      B = S1 / ((V1 - A) * (V1 - A));
      break;
    case satType_t::exponential:
      A = log (S1 / S2) / log (V1 / V2);
      B = S1 / (pow (V1, A));
      break;
    case satType_t::linear:
      B = (S2 - S1) / (V2 - V1);
      A = V1 - S1 / B;
      break;

    }
  s10 = compute (1.0);
  s12 = compute (1.2);
}

void saturation::setType (int ntype)
{
  switch (ntype)
    {
    case 0:
      type = satType_t::none;

      break;
    case 1:
      type = satType_t::quadratic;
      break;
    case 2:
      type = satType_t::scaled_quadratic;
      break;
    case 3:
      type = satType_t::exponential;
      break;
    case 4:
      type = satType_t::linear;
      break;
    }
  loadFunctions ();
  computeParam ();
}

void saturation::setType (satType_t sT)
{
  type = sT;
  loadFunctions ();
  computeParam ();
}
double saturation::operator() (double val)
{
  return satFunc (val);
}
double saturation::compute (double val)
{
  return satFunc (val);
}
double saturation::deriv (double val)
{
  return dFunc (val);
}
double saturation::inv (double val)
{
  double ret = 0.5;
  double temp;
  if (val < 0.00001)
    {
      return 0.5;
    }
  switch (type)
    {
    case satType_t::none:
      break;
    case satType_t::quadratic:
      ret = sqrt (val / B) + A;
      break;
    case satType_t::scaled_quadratic:
      temp = (2 * A + val / B);
      ret = (temp + sqrt (temp - 4 * A * A)) / 2;
      break;
    case satType_t::exponential:
      ret = pow (val / B, 1 / A);
      break;
    case satType_t::linear:
      ret = A + val / B;
      break;

    }
  return ret;
}

void saturation::computeParam ()
{
  double ssv;
  switch (type)
    {
    case satType_t::none:
      A = 0;
      B = 0;
      break;
    case satType_t::quadratic:
      ssv = sqrt (s10 / s12);
      A = -(1.2 * ssv - 1.0) / (1.0 - ssv);
      B = s10 / ((1.0 - A) * (1.0 - A));
      break;
    case satType_t::scaled_quadratic:
      ssv = sqrt ((s10 * 1.0) / (s12 * 1.2));
      A = -(1.2 * ssv - 1.0) / (1.0 - ssv);
      B = s10 / ((1.0 - A) * (1.0 - A));
      break;
    case satType_t::exponential:
      A = -log (s10 / s12) / log (1.2);
      B = s10;
      break;
    case satType_t::linear:
      B = (s12 - s10) / 0.2;
      A = 1.0 - s10 / B;
      break;

    }
}

void saturation::loadFunctions ()
{
  switch (type)
    {
    case satType_t::none:
      satFunc = [](double){
          return 0;
        };
      dFunc = [](double){
          return 0;
        };
      break;
    case satType_t::quadratic:
      satFunc = [&](double val){
          return (B * (val - A) * (val - A));
        };
      dFunc = [&](double val){
          return (2 * B * (val - A));
        };
      break;
    case satType_t::scaled_quadratic:
      satFunc = [&](double val){
          return (B * (val - A) * (val - A) / val);
        };
      dFunc = [&](double val){
          double v1 = (val - A);
          return (B * (2 * val * v1 - v1 * v1) / (val * val));
        };
      break;
    case satType_t::exponential:
      satFunc = [&](double val){
          return (B * pow (val, A));
        };
      dFunc = [&](double val){
          return (A * B * pow (val, A - 1));
        };
      break;
    case satType_t::linear:
      satFunc = [&](double val){
          return (val <= A) ? 0 : (B * (val - A));
        };
      dFunc = [&](double val){
          return (val <= A) ? 0 : B;
        };
      break;
    }
}
