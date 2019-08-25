/*
* LLNS Copyright Start
* Copyright (c) 2014-2018, Lawrence Livermore National Security
* This work was performed under the auspices of the U.S. Department
* of Energy by Lawrence Livermore National Laboratory in part under
* Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
* Produced at the Lawrence Livermore National Laboratory.
* All rights reserved.
* For details, see the LICENSE file.
* LLNS Copyright End
*/

#ifndef TRANSFER_FUNCTION_BLOCK_H_
#define TRANSFER_FUNCTION_BLOCK_H_

#include "../Block.h"

namespace griddyn
{
namespace blocks
{
/** @brief class implementing a generic transfer unction
 block implementing \f$H(S)=\frac{K(b_0+b_1 s +/hdots +b_n s^n}{a_0+a_1 s +/hdots +a_n s^n}\f$
it then converts it to observable canonical form as state space matrices for implementation as part the solver

*/
class transferFunctionBlock : public Block
{

public:
protected:
  std::vector<double> a;        //!< lower time constant
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
  explicit transferFunctionBlock (std::vector<double> Acoef);
  /** constructor to define the transfer function coefficients
  @param[in] Acoef the denominator coefficients
  @param[in] Bcoef the numerator coefficients
  */
  transferFunctionBlock (std::vector<double> Acoef, std::vector<double> Bcoef);
  virtual coreObject * clone (coreObject *obj = nullptr) const override;
  virtual void dynObjectInitializeA (coreTime time0, std::uint32_t flags) override;
  virtual void dynObjectInitializeB (const IOdata &inputs, const IOdata &desiredOutput, IOdata &fieldSet) override;

  virtual void set (const std::string &param, const std::string &val) override;
  virtual void set (const std::string &param, double val, units::unit unitType = units::defunit) override;
  virtual index_t findIndex (const std::string &field, const solverMode &sMode) const override;

  virtual void blockDerivative (double input, double didt, const stateData &sD, double deriv[], const solverMode &sMode) override;
  virtual void blockResidual (double input, double didt, const stateData &sD, double resid[], const solverMode &sMode) override;
  //only called if the genModel is not present
  virtual void blockJacobianElements (double input, double didt, const stateData &sD, matrixData<double> &md, index_t argLoc, const solverMode &sMode) override;
  virtual double step (coreTime time, double inputA) override;
  //virtual void setTime(coreTime time){prevTime=time;};
  virtual stringVec localStateNames () const override;
};
}//namespace blocks
}//namespace griddyn
#endif
