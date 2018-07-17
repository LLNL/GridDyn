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

#ifndef SENSOR_RELAY_H_
#define SENSOR_RELAY_H_

#include "../Relay.h"

namespace griddyn
{

class Block;
class grabberSet;
/** @brief class implementing a sensor relay object
 a sensor can contain a set of basic control blocks and data grabbers which can grab data from any other object
in the system and run in through a set of processes to obtain a result
the result can be output or used as a basis for other relay actions
*/
class sensor : public Relay
{
public:
  /** @brief sensor flags controlling operation
  */
  enum sensor_flags
  {
    direct_IO = object_flag6,  //!< indication that the sensor is directly listing all inputs as outputs with no processing
    link_type_source = object_flag7, //!< indication that the source is a link
    link_type_sink = object_flag8, //!< indicator that the sink is a link object
    no_message_reply = object_flag9, //!< indicator that the sensor should not send message replies
	force_continuous = object_flag10, //!< force continuous operation even if the underlying data sources are not continously available
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
	blockderiv,		//!< direct time derivative of a block
    processed,        //!<processed output
    direct,        //!<direct from an input
  };

protected:

  std::vector<int> blockInputs; //!< the number of the input Source
  //PT leaving outputs as int (vs index_t) as negative values here are meaningful and useful
  std::vector<int> outputs;  //!< locations of output values
  std::vector<stringVec> outputStrings; //!< names for the outputs
  std::vector<outputMode_t> outputMode; //!< output modes corresponding to the outputs
  std::vector<sequenceMode_t> processStatus; //!< the status of the processing sequence
  stringVec inputStrings; //!< vector of input strings
  index_t m_terminal = 0;  //!< the terminal to use on link operations  NOTE: works with link_source and link_sink flags
  count_t instructionCounter = 0; //!< the number of instructions the relay has received
	std::vector<std::shared_ptr<grabberSet>> dataSources;  //the data sources for the output
	std::vector <Block *> filterBlocks; //!< the filtered blocks
	std::vector<std::shared_ptr<grabberSet>> outGrabbers; //!< Grabbers for the output;
	std::vector<std::shared_ptr<Block>> blkptrs;  //!< storage locations for the shared_ptr of blocks
public:
  /** @brief default constructor*/
  explicit sensor (const std::string &objName = "sensor_$");
  virtual coreObject * clone (coreObject *obj = nullptr) const override;
  virtual void setFlag (const std::string &flag, bool val = true) override;
  virtual void set (const std::string &param,  const std::string &val) override;

  virtual void set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;

  virtual double get (const std::string & param, gridUnits::units_t unitType = gridUnits::defUnit) const override;

  virtual void dynObjectInitializeA (coreTime time0, std::uint32_t flags) override;
  virtual void dynObjectInitializeB (const IOdata &inputs, const IOdata & desiredOutput, IOdata &fieldSet) override;
  using Relay::add;
  virtual void add (coreObject *obj) override;
  /** @brief add a filter block to the relay
  @param[in] blk a pointer to a filter block
  */
  virtual void add (Block *blk);
  /** @brief add a shared pointer to a grabberSet
  @param[in] dGr a shared pointer to grabberSet Object
  */
  virtual void add (std::shared_ptr<grabberSet> dGr);

  /** @brief add a shared pointer to a gridGrabber object
  @param[in] dGr a shared pointer to grabberSet Object
  */
  virtual void add(std::shared_ptr<gridGrabber> dGr);

  /** retrieve the grabberSet based on index
  @return a shared_ptr to a grabberset object that is used in the data retrieval*/
  std::shared_ptr<grabberSet> getGrabberSet(index_t grabberNum);

  //dynamic functions for evaluation with a limit exceeded
  virtual void timestep (coreTime time, const IOdata &inputs, const solverMode &sMode) override;
  virtual void jacobianElements (const IOdata &inputs, const stateData &sD, matrixData<double> &md, const IOlocs &inputLocs, const solverMode &sMode) override;

  virtual void residual (const IOdata &inputs, const stateData &sD, double resid[], const solverMode &sMode) override;
  virtual void derivative (const IOdata &inputs, const stateData &sD, double deriv[], const solverMode &sMode) override;
  virtual void algebraicUpdate (const IOdata &inputs, const stateData &sD, double update[], const solverMode &sMode, double alpha) override;

  virtual double getOutput (const IOdata &inputs, const stateData &sD, const solverMode &sMode, index_t outNum = 0) const override;
  virtual double getOutput(index_t outNum = 0) const override;
  virtual index_t getOutputLoc (const solverMode &sMode, index_t outNum) const override;
  virtual IOdata getOutputs (const IOdata &inputs, const stateData &sD, const solverMode &sMode) const override;

  /** @brief get the block output from the sensor
  @param[in] sD  the state data to get the output from
  @param[in] sMode  the solverMode corresponding to the data
  @param[in] blockNumber the number of the block to get the output from
  @return a double with the requested block output
  */
  double getBlockOutput (const stateData &sD,const solverMode &sMode, index_t blockNumber) const;

  /** @brief get the block rate of change from the sensor
  @param[in] sD  the state data to get the output from
  @param[in] sMode  the solverMode corresponding to the data
  @param[in] blockNumber the number of the block to get the output from
  @return a double with the requested block output rate of change
  */
  double getBlockDerivOutput(const stateData &sD, const solverMode &sMode, index_t blockNumber) const;

  /** @brief get the raw sensor input
  @param[in] sD  the state data to get the output from
  @param[in] sMode  the solverMode corresponding to the data
  @param[in] inputNumber the input of the index to get the value
  @return a double with the requested raw input
  */
  double getInput (const stateData &sD, const solverMode &sMode, index_t inputNumber = 0) const;
  virtual void updateA (coreTime time) override;
  virtual void outputPartialDerivatives (const IOdata &inputs, const stateData &sD, matrixData<double> &md, const solverMode &sMode) override;

  virtual void rootTest (const IOdata &inputs, const stateData &sD, double roots[], const solverMode &sMode) override;
  virtual void rootTrigger (coreTime time, const IOdata &inputs, const std::vector<int> &rootMask, const solverMode &sMode) override;
  virtual change_code rootCheck (const IOdata &inputs, const stateData &sD, const solverMode &sMode, check_level_t level) override;

  virtual void receiveMessage (std::uint64_t sourceID, std::shared_ptr<commMessage> message) override;

  virtual void updateObject(coreObject *obj, object_update_mode mode = object_update_mode::direct) override;
  virtual void getObjects(std::vector<coreObject *> &objects) const override;

  virtual const std::vector<stringVec> &outputNames() const override;
protected:
  /** define an output based on a string*/
  void setupOutput(index_t num, const std::string &outputString);
 private:
	 /** @brief generate the input grabbers
	 used in the initialize function
	 */
	 void generateInputGrabbers();
	 /** get the input to a particular block based on inputs and stateData*/
	 double getBlockInput(index_t blockNum, const IOdata &inputs, const stateData &sD, const solverMode &sMode) const;
	 /** get the input to a block based on inputs only*/
	 double getBlockInput(index_t blockNum, const IOdata &inputs) const;
};
}//namespace griddyn
#endif
