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

#ifndef GRIDDYN_SIMULATION_FILE_OPS_H_
#define GRIDDYN_SIMULATION_FILE_OPS_H_

#include <string>

class solverMode;
class gridDynSimulation;

/** @brief save the current state to a file
@param[in] gds  the gridDynSimulation object to operate from
@param[in] fname the name of the file to save the state to
*/
void saveState (gridDynSimulation *gds, const std::string &fname, const solverMode &sMode = cEmptySolverMode, bool append = false);

/** @brief save the bus data to a csv file
@param[in] gds  the gridDynSimulation object to operate from
@param[in] fname the name of the file to save the bus data to
*/
void saveBusData(gridDynSimulation *gds, const std::string &fname);

/** @brief save the line data to a csv file
@param[in] gds  the gridDynSimulation object to operate from
@param[in] fname the name of the file to save the line data to
*/
void saveLineData(gridDynSimulation *gds, const std::string &fname);

/** @brief save the current state to an XML file
@param[in] gds  the gridDynSimulation object to operate from
@param[in] fname the name of the file to save the state to
*/
void saveStateXML (gridDynSimulation *gds, const std::string &fname, const solverMode &sMode = cEmptySolverMode);

/** @brief save the current state to a binary file
@param[in] gds  the gridDynSimulation object to operate from
@param[in] fname the name of the file to save the state to
@param[in] sMode the solverMode to save the state
@param[in] append  boolean indicating the file should be appended
*/
void saveStateBinary (gridDynSimulation *gds, const std::string &fname, const solverMode &sMode = cEmptySolverMode, bool append = true);

/** @brief load a state vector from a file
@param[in] gds  the gridDynSimulation object to operate from
@param[in] fname the name of the file to load
*/
void loadState (gridDynSimulation *gds, const std::string &fname, const solverMode &sMode = cEmptySolverMode);

/** @brief load a binary state file
@param[in] gds  the gridDynSimulation object to operate from
@param[in] fname the name of the file to load
*/
void loadStateBinary (gridDynSimulation *gds, const std::string &fname, const solverMode &sMode = cEmptySolverMode);

/** @brief load a state vector from an XML file
@param[in] gds  the gridDynSimulation object to operate from
@param[in] fname the name of the file to load
*/
void loadStateXML (gridDynSimulation *gds, const std::string &fname, const solverMode &sMode = cEmptySolverMode);

/** @brief capture a jacobian and a state to a file
@param[in] gds  the gridDynSimulation object to operate from
@param[in] fname the name of the file for storage
@param[in] sMode the solverMode to get the state from
*/
void captureJacState (gridDynSimulation *gds, const std::string &fname, const solverMode &sMode = cEmptySolverMode);

/** @brief capture the jacobian data to a file
@param[in] gds  the gridDynSimulation object to operate from
@param[in] fname the name of the file for storage
@param[in] sMode the solverMode to get the state from
*/
void saveJacobian (gridDynSimulation *gds, const std::string &fname, const solverMode &sMode = cEmptySolverMode);

/** @brief save the powerflow results to a file
@param[in] gds  the gridDynSimulation object to operate from
@param[in] fname the name of the file for storage
*/
void savePowerFlow (gridDynSimulation *gds, const std::string &fname);

/** @brief save the powerflow results to a cdf formatted file
@param[in] gds  the gridDynSimulation object to operate from
@param[in] fname the name of the file for storage
*/
void savePowerFlowCdf (gridDynSimulation *gds, const std::string &fname);

/** @brief save the powerflow results to an XML file
@param[in] gds  the gridDynSimulation object to operate from
@param[in] fname the name of the file for storage
*/
void savePowerFlowXML (gridDynSimulation *gds, const std::string &fname);

/** @brief save the powerflow results to a formatted Text file
 this creates a text file for human readability not machine readability
@param[in] gds  the gridDynSimulation object to operate from
@param[in] fname the name of the file for storage
*/
void savePowerFlowTXT (gridDynSimulation *gds, const std::string &fname);

/** @brief save the powerflow results to a binary file
@param[in] gds  the gridDynSimulation object to operate from
@param[in] fname the name of the file for storage
*/
void savePowerFlowBinary (gridDynSimulation *gds, const std::string &fname);

/** @brief save the powerflow results to a csv file
@param[in] gds  the gridDynSimulation object to operate from
@param[in] fname the name of the file for storage
*/
void savePowerFlowCSV (gridDynSimulation *gds, const std::string &fname);

/** @brief load the powerflow results from a file
@param[in] gds  the gridDynSimulation object to operate from
@param[in] fname the name of the file to load
*/
void loadPowerFlow (gridDynSimulation *gds, const std::string &fname);

/** @brief load the powerflow results from an XML file
@param[in] gds  the gridDynSimulation object to operate from
@param[in] fname the name of the file to load
*/
void loadPowerFlowXML (gridDynSimulation *gds, const std::string &fname);

/** @brief load the powerflow results from an CDF file
@param[in] gds  the gridDynSimulation object to operate from
@param[in] fname the name of the file to load
*/
void loadPowerFlowCdf (gridDynSimulation *gds, const std::string &fname);

/** @brief load the powerflow results from a binary file
@param[in] gds  the gridDynSimulation object to operate from
@param[in] fname the name of the file to load
*/
void loadPowerFlowBinary (gridDynSimulation *gds, const std::string &fname);

/** @brief load the powerflow results from a csv file
@param[in] gds  the gridDynSimulation object to operate from
@param[in] fname the name of the file to load
*/
void loadPowerFlowCSV (gridDynSimulation *gds, const std::string &fname);


#endif
