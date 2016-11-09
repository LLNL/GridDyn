/* -*-Mode:C++; c - file - style:"gnu"; indent - tabs - mode:nil;  eval: (c - set - offset 'innamespace 0); -*- */
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

#ifndef OTHERCONTROLBLOCKS_H_
#define OTHERCONTROLBLOCKS_H_

#include "submodels/gridControlBlocks.h"
#include <functional>

/** @brief class implementing a PID controller
 the derivative operator has a prefilter operation on it with a time constant T1 and the output has a delay of Td*/
class pidBlock : public basicBlock
{

public:
protected:
  double m_P = 1;        //!< proportional control constant
  double m_I = 0;         //!< integral control constant
  double m_D = 0;         //!< differential control constant
  double m_T1 = 0.01;        //!< filtering delay on the input for the differential calculation
  double m_Td = 0.01;        //!< filtering delay on the output
  double iv = 0;             //!< intermediate value for calculations
  bool no_D = true;                     //!< ignore the derivative part of the calculations
public:
  /** @brief default constructor*/
  pidBlock (const std::string &objName = "pidBlock_#");
  /** @brief alternate constructor
  @param[in] P the proportional gain
  @param[in] I the integral gain
  @param[in] D the derivative gain
  */
  pidBlock (double P, double I, double D, const std::string &objName = "pidBlock_#");
  virtual gridCoreObject * clone (gridCoreObject *obj = nullptr) const override;
  virtual void objectInitializeA (double time0, unsigned long flags) override;
  virtual void objectInitializeB (const IOdata &args, const IOdata &outputSet, IOdata &inputSet) override;

  virtual void set (const std::string &param, const std::string &val) override;
  virtual void set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;
  virtual index_t findIndex (const std::string &field, const solverMode &sMode) const override;

  virtual void derivElements (double input, double didt, const stateData *sD, double deriv[], const solverMode &sMode) override;
  //only called if the genModel is not present
  virtual void jacElements (double input, double didt, const stateData *sD, matrixData<double> *ad, index_t argLoc, const solverMode &sMode) override;
  virtual double step (double time, double input) override;
  virtual stringVec localStateNames () const override;
};


/** @brief class implementing a derivative
 block implementing \f$H(S)=\frac{K s}{1+T_1 s} \frac{1}{1+T_2 s}\f$

*/
class filteredDerivativeBlock : public basicBlock
{
protected:
  double m_T1 = 0.1;       //!< delay time constant for the derivative filtering operation
  double m_T2 = 0.1;        //!< filter on the derivative of block 1
public:
  //!< default constructor
  filteredDerivativeBlock (const std::string &objName = "filtDerivBlock_#");
  /** alternate constructor to add in the time constant
  @param[in] t1  the time constant for the prederivative filter
  @param[in] t2 the time constant for the derivative filter
  */
  filteredDerivativeBlock (double t1,double T2, const std::string &objName = "filtDerivBlock_#");
  virtual gridCoreObject * clone (gridCoreObject *obj = nullptr) const override;
protected:
  virtual void objectInitializeA (double time0, unsigned long flags) override;
  virtual void objectInitializeB (const IOdata &args, const IOdata &outputSet, IOdata &inputSet) override;
public:
  virtual void set (const std::string &param, const std::string &val) override;
  virtual void set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;
  //virtual index_t findIndex(const std::string &field, const solverMode &sMode) const;

  virtual void derivElements (double input, double didt, const stateData *sD, double deriv[], const solverMode &sMode) override;
  //only called if the genModel is not present
  virtual void jacElements (double input, double didt, const stateData *sD, matrixData<double> *ad, index_t argLoc, const solverMode &sMode) override;
  virtual double step (double time, double input) override;

  virtual stringVec localStateNames () const override;
  //virtual void setTime(double time){prevTime=time;};
};


/** @brief class implementing a function operation on the input
 a wide assortment of functions are available including trig, logs, and other common math operations*/
class functionBlock : public basicBlock
{

public:
  //!< flags for function block
  enum functionblock_flags
  {
    uses_constantarg = object_flag10,              //!< flag indicating that the function should use a constant argument for the second argument of functions
  };
protected:
  std::function<double(double)> fptr;        //!< function object for single argument functions
  std::function<double(double, double)> fptr2;        //!< function object for multiple argument functions
  double gain = 1.0;         //!< extra gain factor
  double arg2 = 0.0;         //!< second argument for 2 argument functions
public:
  /** @brief default constructor*/
  functionBlock ();
  /** @brief alternate constructor
  @param[in] function the name of the function as a string
  */
  functionBlock (const std::string &function);
  virtual gridCoreObject * clone (gridCoreObject *obj = nullptr) const override;

  virtual void objectInitializeB (const IOdata &args, const IOdata &outputSet, IOdata &inputSet) override;

  virtual void set (const std::string &param, const std::string &val) override;
  virtual void set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;
  //virtual index_t findIndex(const std::string &field, const solverMode &sMode) const;

  //virtual void derivElements(double input, double didt, const stateData *sD, double deriv[], const solverMode &sMode) override;
  virtual void algElements (double input, const stateData *sD, double deriv[], const solverMode &sMode) override;


  //only called if the genModel is not present
  virtual void jacElements (double input, double didt, const stateData *sD, matrixData<double> *ad, index_t argLoc, const solverMode &sMode) override;
  virtual double step (double time, double input) override;
  //virtual void setTime(double time){prevTime=time;};
protected:
  /** @brief load the function objects from a string input
  @param[in] fname the name of the function as a string*/
  void setFunction (const std::string &fname);
};

/** @brief lookup table block*/
class lutBlock : public basicBlock
{

public:
private:
  std::vector<std::pair<double, double>> lut;  //!< the lookup table
  double b = 0;         //!< the intercept of the interpolation function of the current lookup section
  double m = 0;         //!< the slope of the interpolation function of the current lookup section
  double vlower = kBigNum;  //!< the lower value of the current lookup table section
  double vupper = -kBigNum;     //!< the upper value of the current lookup table section
  int lindex = -1;  //!< the index of the current lookup table section
public:
  lutBlock (const std::string &objName = "lutBlock_#");
  virtual gridCoreObject * clone (gridCoreObject *obj = nullptr) const override;
  //virtual void objectInitializeA (double time0, unsigned long flags);
  virtual void objectInitializeB (const IOdata &args, const IOdata &outputSet, IOdata &inputSet) override;

  virtual void set (const std::string &param, const std::string &val) override;
  virtual void set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;

  virtual void algElements (double input, const stateData *sD, double deriv[], const solverMode &sMode) override;
  //virtual double residElements (double input, double didt, const stateData *sD, double resid[], const solverMode &sMode) override;
  virtual void jacElements (double input, double didt, const stateData *sD, matrixData<double> *ad, index_t argLoc, const solverMode &sMode) override;
  virtual double step (double time, double input) override;
  //virtual void setTime(double time){prevTime=time;};
  double computeValue (double input);
};

/** @brief class implementing a generic transfer unction
 block implementing \f$H(S)=\frac{K(b_0+b_1 s +/hdots +b_n s^n}{a_0+a_1 s +/hdots +a_n s^n}\f$
it then converts it to observable canonical form as state space matrices for implementation as part the solver

*/
class transferFunctionBlock : public basicBlock
{

public:
protected:
  std::vector<double> a;        //!< delay time constant
  std::vector<double> b;        //!< upper time constant
private:
  // double rescale = 1;                   //!< containing the original $a_n$ for rescaling if coefficients are changed later
  bool extraOutputState = false;        //!< flag indicating that there is an extra state computation at the end due to direct dependence of B;
public:
  /** constructor to add in the order of the transfer function
  @param[in] order  the order of the transfer function
  */
  explicit transferFunctionBlock (int order = 1);

  /** constructor to add in the name of the block
  @param[in] objName  the name
  */
  explicit transferFunctionBlock(const std::string &objName);
  /** constructor to define the transfer function coefficients assuming $b_0=1$ and all others are 0
  @param[in] Acoef the denominator coefficients
  */
  transferFunctionBlock (std::vector<double> Acoef);
  /** constructor to define the transfer function coefficients
  @param[in] Acoef the denominator coefficients
  @param[in] Bcoef the numerator coefficients
  */
  transferFunctionBlock (std::vector<double> Acoef, std::vector<double> Bcoef);
  virtual gridCoreObject * clone (gridCoreObject *obj = nullptr) const override;
  virtual void objectInitializeA (double time0, unsigned long flags) override;
  virtual void objectInitializeB (const IOdata &args, const IOdata &outputSet, IOdata &inputSet) override;

  virtual void set (const std::string &param, const std::string &val) override;
  virtual void set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;
  virtual index_t findIndex (const std::string &field, const solverMode &sMode) const override;

  virtual void derivElements (double input, double didt, const stateData *sD, double deriv[], const solverMode &sMode) override;
  virtual void residElements (double input, double didt, const stateData *sD, double resid[], const solverMode &sMode) override;
  //only called if the genModel is not present
  virtual void jacElements (double input, double didt, const stateData *sD, matrixData<double> *ad, index_t argLoc, const solverMode &sMode) override;
  virtual double step (double time, double input) override;
  //virtual void setTime(double time){prevTime=time;};
  virtual stringVec localStateNames () const override;
};

#endif
