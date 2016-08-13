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

#ifndef PARAMETER_OPERATOR_H_

#define PARAMETER_OPERATOR_H_

#include "gridCore.h"

#include <type_traits>

#include <map>

const stringVec emptyStringVec;

class baseParamOperator
{
public:
  baseParamOperator ()
  {
  }
  virtual ~baseParamOperator ()
  {
  }

  virtual bool isSettable ()
  {
    return false;
  }
  virtual bool isGettable ()
  {
    return false;
  }
  virtual double get (const gridCoreObject *, gridUnits::units_t) const
  {
    return kNullVal;
  }
  virtual int set (gridCoreObject *, double, gridUnits::units_t)
  {
  }
  virtual const stringVec &setNames () const
  {
    return emptyStringVec;
  }
  virtual const stringVec &getNames () const
  {
    return emptyStringVec;
  }
};

template<class A>
class simpleParamOperator : public baseParamOperator
{
  static_assert (std::is_base_of<gridCoreObject, A>::value, "classes must be inherited from gridCoreObject");
public:
  typedef double (A::*paramPtr);
  stringVec paramNames;
  paramPtr param;
public:
  paramOperator (std::string name, paramPtr newParam) : paramNames {
    name
  }, param (newParam)
  {

  };
  paramOperator (stringVec names, paramPtr newParam) : paramNames (names),
                                                       param (newParam)
  {

  };

  virtual bool isSettable () override
  {
    return true;
  }
  virtual bool isGettable () override
  {
    return true;
  }
  virtual int set (gridCoreObject *obj, double val, gridUnits::units_t unitType) override
  {
    static_cast<A *> (obj)->*param = val;
    return 0;
  }

  virtual double get (const gridCoreObject *obj, gridUnits::units_t unitType) const override
  {
    return static_cast<const A *> (obj)->*param;
  }

  virtual const stringVec &setNames () const override
  {
    return paramNames;
  }
  virtual const stringVec &getNames () const override
  {
    return paramNames;
  }
};

template<class A>
class paramOperator : public baseParamOperator
{
  static_assert (std::is_base_of<gridCoreObject, A>::value, "classes must be inherited from gridCoreObject");
public:
  typedef double (A::*paramPtr);
  stringVec paramNames;
  paramPtr param;
  gridUnits::units_t defUnits;

public:
  paramOperator (std::string name, paramPtr newParam, gridUnits::units_t paramUnits = gridUnits::defUnit) : paramNames { name }, param (newParam), defUnits (paramUnits)
  {

  }
  paramOperator (stringVec names, paramPtr newParam, gridUnits::units_t paramUnits = gridUnits::defUnit) : paramNames (names), param (newParam), defUnits (paramUnits)
  {

  }

  virtual bool isSettable () override
  {
    return true;
  }
  virtual bool isGettable () override
  {
    return true;
  }

  virtual int set (gridCoreObject *obj, double val, gridUnits::units_t unitType) override
  {

    if (defUnits != gridUnits::defUnit)
      {
        double v2 = unitConversion (val, unitType, defUnits, obj->systemBasePower);
        if (v2 == kNullVal)
          {
            return INVALID_PARAMETER_UNIT;
          }
        static_cast<A *> (obj)->*param = v2;
      }
    else
      {
        static_cast<A *> (obj)->*param = val;
      }
    return 0;
  }

  virtual double get (const gridCoreObject *obj, gridUnits::units_t unitType) const override
  {
    if (defUnits != gridUnits::defUnit)
      {

        return unitConversion (static_cast<const A *> (obj)->*param, defUnits, unitType, obj->systemBasePower);
      }
    else
      {
        return static_cast<const A *> (obj)->*param;
      }
  }
  virtual const stringVec &setNames () const override
  {
    return paramNames;
  }
  virtual const stringVec &getNames () const override
  {
    return paramNames;
  }
};


template<class A>
class paramLimitOperator : public paramOperator<A>
{
  static_assert (std::is_base_of<gridCoreObject, A>::value, "classes must be inherited from gridCoreObject");
public:
  double upperLimit = kBigNum;
  double lowerLimit = -kBigNum;
public:
  paramOperator (std::string name, paramPtr newParam, double lowerLim, double upperLim, gridUnits::units_t paramUnits = gridUnits::defUnit) : paramOperator (name,newParam,paramUnits),
                                                                                                                                              lowerLimit (lowerLim),
                                                                                                                                              upperLimit (upperLim)
  {

  };
  paramOperator (stringVec names, paramPtr newParam,  double lowerLim, double upperLim,gridUnits::units_t paramUnits = gridUnits::defUnit) : paramOperator (name, newParam, paramUnits),
                                                                                                                                             lowerLimit (lowerLim),
                                                                                                                                             upperLimit (upperLim)
  {

  };

  virtual void set (gridCoreObject *obj, double val, gridUnits::units_t unitType) override
  {
    double v2 = (defUnits != gridUnits::defUnit) ? unitConversion (val, unitType, defUnits, obj->systemBasePower) : val;
    if (val == kNullVal)
      {
        return INVALID_PARAMETER_UNIT
      }
    if (v2 > upperLim)
      {
        return PARAMETER_VALUE_ABOVE_LIMIT;
      }
    if (v2 < lowerLim)
      {
        PARAMETER_VALUE_BELOW_LIMIT;
      }
    static_cast<A *> (obj)->*param = v2;

  }

};

class paramContainer
{
private:
  std::shared_ptr<baseParamOperator> paramOp;
public:
  paramContainer (std::shared_ptr<baseParamOperator> paramB) : paramOp (paramB)
  {
  }
  bool isSettable ()
  {
    return paramOp->isSettable ();
  }
  bool isGettable ()
  {
    return paramOp->isGettable ();
  }
  double get (const gridCoreObject *obj, gridUnits::units_t unitType) const
  {
    return paramOp->get (obj, unitType);
  }
  void set (gridCoreObject *obj, double val, gridUnits::units_t unitType)
  {
    paramOp->set (obj, val, unitType);
  }

  const stringVec &setNames () const
  {
    return paramOp->setNames ();
  }
  const stringVec &getNames () const
  {
    return paramOp->getNames ();
  }
};

template<class A, class B>
class objParamHelper
{
  static_assert (std::is_base_of<B, A>::value, "classes A and B must have parent child relationship");
  static_assert (std::is_base_of<gridCoreObject, B>::value, "classes must be inherited from gridCoreObject");
  static_assert (std::is_base_of<gridCoreObject, A>::value, "classes must be inherited from gridCoreObject");
public:
  objParamHelper ()
  {

  }
  objParamHelper (std::vector<paramContainer> &params)
  {
    for (auto &pc : params)
      {
        for (auto &pn : pc.setNames ())
          {
            setParamMap.insert (pn, pO);
          }
        for (auto &pn : pc.getNames ())
          {
            getParamMap.insert (pn, pO);
          }
      }

  }
private:
  std::map<std::string, paramContainer> setParamMap;
  std::map<std::string, paramContainer> getParamMap;
};

#endif
