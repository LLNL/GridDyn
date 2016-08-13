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

#ifndef GRIDOPTOBJECT_H_
#define GRIDOPTOBJECT_H_
#include "gridCore.h"
#include "optHelperClasses.h"
#include <array>
#include <bitset>
#include <map>




class vectData;
class consData;

template <class Y>
class arrayData;

/** base object for gridDynSimulations
* the basic object for creating a power system encapsulating some common functions and data that is needed by all objects in the simulation
* and defining some common methods for use by all objects.  This object is not really intended to be instantiated directly and is mostly a
* common interface to inheriting objects gridPrimary, gridSecondary, and gridSubModel as it encapsulated common functionality between those objects
**/
class gridOptObject : public gridCoreObject
{
public:
  std::bitset<32> optFlags;        //!< operational flags these flags are designed to be normal false
  count_t numParams = 0; //!< the number of paramaters to store in an archive
  optOffsetTable offsets;        //!<a table of offsets for the different solver modes
protected:
public:
  gridOptObject (const std::string &objName = "optObject_#");
  virtual gridCoreObject * clone (gridCoreObject *obj = nullptr) const override;
  virtual int set (const std::string &param, const std::string &val) override;

  virtual int set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;
  /** set the offsets of an object for a particular optimization mode.
  \param newOffsets the offset set to use.
  \param oMode the optimization mode to use.
  */
  virtual void setOffsets (const optimOffsets &newOffsets, const optimMode &oMode);
  /** set the offsets of an object for a particular optimization mode using a single offset.
  \param offset the offset index all variables are sequential.
  \param oMode the optimization mode to use.
  */
  virtual void setOffset (index_t offset, index_t constraintOffset, const optimMode &oMode);


  // size getter functions

  /** get the objective size.
  \param oMode the optimization mode to use.
  */
  count_t objSize (const optimMode &oMode);
  /** get the number of continuous objective variables.
  \param oMode the optimization mode to use.
  */
  count_t contObjSize (const optimMode &oMode);
  /** get the number of integer objective variables.
  \param oMode the optimization mode to use.
  */
  count_t intObjSize (const optimMode &oMode);
  /** get the number of real power generation objective variables.
  \param oMode the optimization mode to use.
  */
  count_t gSize (const optimMode &oMode);
  /** get the number of reactive power generation objective variables.
  \param oMode the optimization mode to use.
  */
  count_t qSize (const optimMode &oMode);
  /** get the number of voltage objective variables.
  \param oMode the optimization mode to use.
  */
  count_t vSize (const optimMode &oMode);
  /** get the number of angle objective variables.
  \param oMode the optimization mode to use.
  */
  count_t aSize (const optimMode &oMode);
  /** get the number of constaints.
  \param oMode the optimization mode to use.
  */
  count_t constraintSize (const optimMode &oMode);

  /** get the objective size.
  \param oMode the optimization mode to use.
  */
  count_t objSize (const optimMode &oMode) const;
  /** get the number of continuous objective variables.
  \param oMode the optimization mode to use.
  */
  count_t contObjSize (const optimMode &oMode) const;
  /** get the number of integer objective variables.
  \param oMode the optimization mode to use.
  */
  count_t intObjSize (const optimMode &oMode) const;
  /** get the number of real power generation objective variables.
  \param oMode the optimization mode to use.
  */
  count_t gSize (const optimMode &oMode) const;
  /** get the number of reactive power generation objective variables.
  \param oMode the optimization mode to use.
  */
  count_t qSize (const optimMode &oMode) const;
  /** get the number of voltage objective variables.
  \param oMode the optimization mode to use.
  */
  count_t vSize (const optimMode &oMode) const;
  /** get the number of angle objective variables.
  \param oMode the optimization mode to use.
  */
  count_t aSize (const optimMode &oMode) const;
  /** get the number of constaints.
  \param oMode the optimization mode to use.
  */
  count_t constraintSize (const optimMode &oMode) const;

  /** intitialize the object.
  \param flags the optimization mode to use.
  */
  virtual void objectInitializeA (unsigned long flags);

  /** intitialize the object.
  \param flags the optimization mode to use.
  */
  void initializeA (unsigned long flags);

  /** intitialize the object.
  \param flags the optimization mode to use.
  */
  virtual void objectInitializeB (unsigned long flags);

  /** intitialize the object.
  \param flags the optimization mode to use.
  */
  void initializeB (unsigned long flags);
  /** compute the sizes and store them in the offsetTables.
  \param oMode the optimization mode to use.
  */
  virtual void loadSizes (const optimMode &oMode);

  /** set the objective variable values to the objects.
  \param val  the output objective variable values
  \param oMode the optimization mode to use.
  */
  virtual void setValues (const optimData *oD, const optimMode &oMode);

  /** get a guess from the object as to the value.
  \param val  the output objective variable values
  \param oMode the optimization mode to use.
  *///for saving the state
  virtual void guess (double ttime, double val[], const optimMode &oMode);

  /** set the tolerances.
  \param oMode the optimization mode to use.
  */
  virtual void getTols (double tols[], const optimMode &oMode);

  /** indicate if a variable is continuous or integer.
  \param sdata  the vector of indices for integer value 0 continuous, 1 integer
  \param oMode the optimization mode to use.
  */
  virtual void getVariableType (double sdata[], const optimMode &oMode);

  /**load the upper and lower limit for an objective variable
  \param upLimit  the upper limit
  \param lowerLimit  the lower limit
  \param oMode the optimization mode to use.
  */
  virtual void valueBounds (double ttime, double upLimit[], double lowerLimit[], const optimMode &);

  /** load the linear objective parameters
  \param val  the current object variable values
  \param linObj the structure to store the objective parameters
  \param oMode the optimization mode to use.
  */
  virtual void linearObj (const optimData *oD, vectData * linObj, const optimMode &oMode);
  /** load the quadrative objective parameters
  \param val  the current object variable values
  \param linObj the structure to store the linear objective parameters
  \param quadObj the structure to store the 2nd order objective parameters
  \param oMode the optimization mode to use.
  */
  virtual void quadraticObj (const optimData *oD, vectData * linObj, vectData * quadObj, const optimMode &oMode);

  /** compute the objective value
  \param val  the current object variable values
  \param oMode the optimization mode to use.
  \return the objective value
  */
  virtual double objValue (const optimData *oD, const optimMode &oMode);

  /** compute the objective value partial derivatives
  \param val  the current object variable values
  \param deriv the vector containting all \frac{dC}{dO_i}
  \param oMode the optimization mode to use.
  */
  virtual void derivative (const optimData *oD, double deriv[], const optimMode &oMode);

  /** compute the jacobian entries for the objective value
  \param val  the current object variable values
  \param arrayData the structure for storing \frac{dC_i}{dO_j}
  \param oMode the optimization mode to use.
  */
  virtual void jacobianElements (const optimData *oD, arrayData<double> *, const optimMode &oMode);

  //constraint functions
  /** get the linear constraint operations
  \param val  the current object variable values
  \param consData the structure for the constraint parameters storing the coefficients, the upper and lower limit
  \param oMode the optimization mode to use.
  */
  virtual void getConstraints (const optimData *oD, arrayData<double> * cons, double upperLimit[], double lowerLimit[], const optimMode &oMode);

  /** get the linear constraint operations
  \param val  the current object variable values
  \param cVals the computed value of the constraint
  \param oMode the optimization mode to use.
  */
  virtual void constraintValue (const optimData *oD, double cVals[], const optimMode &oMode);
  /** get the jacobian array of the constraints
  \param val  the current object variable values
  \param arrayData the structure for the constraint jacobian entries
  \param oMode the optimization mode to use.
  */
  virtual void constraintJacobianElements (const optimData *oD, arrayData<double> *, const optimMode &);
  /** get the names of the objective variables
  \param objNames  the location to store the names
  \param oMode the optimization mode to use.
  \param prefix (optional) string to place before the objective name
  */
  virtual void getObjName (stringVec &objNames, const optimMode &oMode, const std::string &prefix = "" );

  /** get a specific bus
  \param index  the index of the bus to return
  \return the gridOptObject represented by the bus index
  */
  virtual gridOptObject * getBus (index_t index) const;

  /** get a specific area
  \param index  the index of the area to return
  \return the gridOptObject represented by the area index
  */
  virtual gridOptObject * getArea (index_t index) const;

  /** get a specific Link
  \param index  the index of the area to return
  \return the gridOptObject represented by the link  index
  */
  virtual gridOptObject * getLink (index_t index) const;
};




void printObjStateNames (gridOptObject *obj,const optimMode &oMode);

#endif
