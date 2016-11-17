/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil;  c-set-offset 'innamespace 0; -*- */
/*
* LLNS Copyright Start
* Copyright (c) 2014, Lawrence Livermore National Security
* This work was performed under the auspices of the U.S. Department
* of Energy by Lawrence Livermore National Laboratory in part under
* Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
* Produced at the Lawrence Livermore National Laboratory.
* All rights reserved.
* For details, see the LICENSE file.
* LLNS Copyright End
*/

#ifndef SENSOR_RELAY_H_
#define SENSOR_RELAY_H_

#include "gridRelay.h"

class basicBlock;
class commMessage;
/** @brief class implementing a sensor relay object
 a sensor can contain a set of basic control blocks and data grabbers which can grab data from any other object
in the system and run in through a set of processes to obtain a result
the result can be output or used as a basis for other relay actions
*/
class sensor : public gridRelay
{
public:
  /** @brief sensor flags controlling operation
  */
  enum sensor_flags
  {
    direct_IO = object_flag6,  //!< indication that the sensor is directly listing all input as outputs with no processing
    link_type_source = object_flag7, //!< indication that the source is a link
    link_type_sink = object_flag8, //!< indicator that the sink is a link object
    no_message_reply = object_flag9, //!< indicator that the sensor should not send message replies

  };
  /** @brief define the possible operation modes for a processing sequence*/
  enum class sequenceMode_t : unsigned char
  {
    normal,  //!< the sequence runs under normal operation with both state and sampled input available
    sampled,  //!< the sequence only runs with periodic sampling
    disabled,  //!< the sequence is disabled
  };
  /** @brief enumeration of the output modes available for the outputs
   the output can be direct from an in input grabber, it can be directly from a block processing object
  or it can a function of block or inputs*/
  enum class outputMode_t : unsigned char
  {
    block,         //!<direct from a filter block output
    processed,        //!<processed output
    direct,        //!<direct from an input
  };
protected:
  std::vector < std::shared_ptr < stateGrabber >> dataSourcesSt; //!< the data state grabbers
  std::vector <std::shared_ptr<gridGrabber>> dataSources;  //!< the data sources for sampled output

  std::vector <std::shared_ptr< basicBlock>> filterBlocks; //!< the filtered blocks
  std::vector<std::vector<int>> processSequence; //!< the sequence of block that define a processing sequence
  //PT leaving outputs as int (vs index_t) as negative values here are meaningful and useful
  std::vector<int> outputs;  //!< locations of output values
  stringVec outputNames; //!< names for the outputs
  std::vector<outputMode_t> outputMode; //!< output modes corresponding to the outputs
  std::vector<sequenceMode_t> processStatus; //!< the status of the processing sequence
  std::vector<std::shared_ptr<gridGrabber>> outGrabber; //!< Grabbers for the output
  std::vector<std::shared_ptr<stateGrabber>> outGrabberSt; //!< Grabbers for the state output
  stringVec inputStrings; //!< vector of input strings
  index_t m_terminal = 0;  //!< the terminal to use on link operations  NOTE: works with link_source and link_sink flags
  count_t outputSize = 1; //!< the size of the output
  count_t instructionCounter = 0; //!< the number of instructions the relay has received

public:
  /** @brief default constructor*/
  explicit sensor (const std::string &objName = "sensor_$");
  virtual gridCoreObject * clone (gridCoreObject *obj = nullptr) const override;
  virtual void setFlag (const std::string &flag, bool val = true) override;
  virtual void set (const std::string &param,  const std::string &val) override;

  virtual void set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;

  virtual double get (const std::string & param, gridUnits::units_t unitType = gridUnits::defUnit) const override;

  virtual void dynObjectInitializeA (double time0, unsigned long flags) override;
  virtual void dynObjectInitializeB (IOdata &outputSet) override;
  using gridRelay::add;
  virtual void add (gridCoreObject *obj) override;
  /** @brief add a filter block to the relay
  @param[in] blk a pointer to a filter block
  @return OBJECT_ADD_SUCCESS if successful OBJECT_ADD_FAILURE otherwise
  */
  virtual void add (basicBlock *blk);
  /** @brief add a shared pointer to a filter block to the relay
  @param[in] blk a shared pointer to a filter block
  @return OBJECT_ADD_SUCCESS if successful OBJECT_ADD_FAILURE otherwise
  */
  virtual void add (std::shared_ptr<basicBlock> blk);
  /** @brief add a shared pointer to a grabber/state Grabber pair
   the stateGrabber may be nullptr if only sampled operation is supported
  @param[in] dGr a shared pointer to gridGrabber Object
  @param[in] dGrst  a shared pointer to a stateGrabber Object
  @return OBJECT_ADD_SUCCESS if successful OBJECT_ADD_FAILURE otherwise
  */
  virtual void add (std::shared_ptr<gridGrabber> dGr, std::shared_ptr<stateGrabber> dGrst = nullptr);
  /** @brief add a shared pointer to gridObject,  must be a basic Block otherwise OBJECT_NOT_RECOGNIZED is returned
  @param[in] obj a shared pointer to a filter block
  @return OBJECT_ADD_SUCCESS if successful OBJECT_ADD_FAILURE otherwise
  */
  virtual void addsp (std::shared_ptr<gridCoreObject> obj) override;



  //dynamic functions for evaluation with a limit exceeded
  virtual void timestep (double ttime, const solverMode &sMode) override;
  virtual void jacobianElements (const stateData *sD, matrixData<double> &ad, const solverMode &sMode) override;
  virtual void setState (double ttime, const double state[], const double dstate_dt[], const solverMode &sMode) override;
  virtual void residual (const stateData *sD, double resid[], const solverMode &sMode) override;
  virtual void derivative (const stateData *sD, double deriv[], const solverMode &sMode) override;
  virtual void algebraicUpdate (const stateData *sD, double update[], const solverMode &sMode, double alpha) override;
  virtual void guess (double ttime, double state[], double dstate_dt[], const solverMode &sMode) override;

  virtual void setOffsets (const solverOffsets &newOffsets, const solverMode &sMode) override;
  virtual void setOffset (index_t offset, const solverMode &sMode) override;
  virtual void setRootOffset (index_t Roffset, const solverMode &sMode) override;
  virtual void loadSizes (const solverMode &sMode, bool dynOnly) override;

  virtual void getStateName (stringVec &stNames, const solverMode &sMode, const std::string &prefix) const override;
  virtual double getOutput (const stateData *sD, const solverMode &sMode, index_t num = 0) const override;
  virtual index_t getOutputLoc (const solverMode &sMode, index_t num) const override;
  virtual IOdata getOutputs ( const stateData *sD, const solverMode &sMode) override;

  /** @brief get the block output from the sensor
  @param[in] sD  the state data to get the output from
  @param[in] sMode  the solverMode corresponding to the data
  @param[in] blockNumber the number of the block to get the output from
  @return a double with the requested block output
  */
  double getBlockOutput (const stateData *sD,const solverMode &sMode, index_t blockNumber) const;

  /** @brief get the raw sensor input
  @param[in] sD  the state data to get the output from
  @param[in] sMode  the solverMode corresponding to the data
  @param[in] inputNumber the input of the index to get the value
  @return a double with the requested raw input
  */
  double getInput (const stateData *sD, const solverMode &sMode, index_t inputNumber = 0) const;
  virtual void updateA (double time) override;
  virtual void updateFlags (bool dynOnly = false) override;
  virtual void outputPartialDerivatives (const stateData *sD, matrixData<double> &ad, const solverMode &sMode) override;

  virtual void rootTest (const stateData *sD, double roots[], const solverMode &sMode) override;
  virtual void rootTrigger (double ttime, const std::vector<int> &rootMask, const solverMode &sMode) override;
  virtual change_code rootCheck (const stateData *sD, const solverMode &sMode, check_level_t level) override;

  virtual void receiveMessage (std::uint64_t sourceID, std::shared_ptr<commMessage> message) override;

  /** @brief translate an output name into an output Index
  @param[in] outName the named output to find
  @return an index for the output number so it can be used with the getOutput function
  */
  index_t lookupOutput (const std::string &outName);

  virtual void updateObject(gridCoreObject *obj, object_update_mode mode = object_update_mode::direct) override;
  virtual void getObjects(std::vector<gridCoreObject *> &objects) const override;
protected:
  /** @brief generate the input grabbers
   used in the initialize function
  */
  void generateInputGrabbers ();
};

#endif
