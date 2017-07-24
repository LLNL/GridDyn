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

#ifndef UTILITIES_LOGGER_H_
#define UTILITIES_LOGGER_H_
#pragma once

#include "simpleQueue.hpp"
#include <fstream>
#include <atomic>
#include <thread>             // std::thread
#include <condition_variable> // std::condition_variable

namespace utilities
{
/** class implementing a thread safe logger 
@details the logger uses a queing mechanism and condition variable to store messages to a queue and print/display them
in a single thread allowing for asynchronous logging
*/
class logger
{

private:
	simpleQueue<std::string> loggingQueue;  //!< the actual queue containing the strings to log
	std::atomic<bool> emptyFlag{ true };	//!< a flag indicating that the queue is empty
	std::ofstream outFile;	//!< the stream to write the log messages
	std::mutex cvMutex;	//!< a mutex for linking with the condition variable
	std::condition_variable notifier;	//!< a notification system
	std::thread loggingThread;	//!< the thread object containing the thread running the actual logger
public:
	int consoleLevel = 100;	//!< level below which we need to print to the consold
	int fileLevel = 100;	//!< level below which we need to print to a file
public:
	/** default constructor*/
	logger();
	/**destructor*/
	~logger();
	/** open a file to write the log messages
	@param[in] file the name of the file to write messages to*/
	void openFile(const std::string &file);
	/** function to start the logging thread
	@param[in] cLevel the console print level
	@param[in] fLevel the file print level  messages coming in below these levels will be printed*/
	void startLogging(int cLevel, int fLevel);
	/** overload of ::startLogging with unspecified logging levels*/
	void startLogging()
	{
		startLogging(consoleLevel, fileLevel);
	}
	/** log a message at a particular level
	@param[in] level the level of the message
	@param[in] logMessage the actual message to log
	*/
	void log(int level, std::string logMessage);
	/** message to log without regard for levels*
	@param[in] logMessage the message to log
	*/
	void log(std::string logMessage)
	{
		log(-100000, logMessage);
	}
	/** check if the logging thread is running*/
	bool isRunning() const;
	/** flush the log queue*/
	void flush();
	/** alter the printing levels
	@param[in] cLevel the level to print to the console
	@param[in] fLevel the leve to print to the file if it is open*/
	void changeLevels(int cLevel, int fLevel);
private:
	/** actual loop function to run the logger*/
	void loggerLoop();
	/** reset the empty flag*/
	void clearEmptyFlag();
};

/** logging class that handle the logs immediately with no threading or synchronization*/
class loggerNoThread
{
private:
	std::ofstream outFile;  //!< the file stream to write the log messages to
public:
	int consoleLevel = 100;	//!< level below which we need to print to the consold
	int fileLevel = 100;	//!< level below which we need to print to a file
public:
	/** default constructor*/
	loggerNoThread();
	/** open a file to write the log messages
	@param[in] file the name of the file to write messages to*/
	void openFile(const std::string &file);
	/** function to start the logging thread
	@param[in] cLevel the console print level
	@param[in] fLevel the file print level  messages coming in below these levels will be printed*/
	void startLogging(int cLevel, int fLevel);
	/** overload of ::startLogging with unspecified logging levels*/
	void startLogging()
	{
		startLogging(consoleLevel, fileLevel);
	}
	//NOTE:: the interface for log in the noThreadLogging is slightly different
	//due to the threaded logger making use of move semantics which isn't that useful in the noThreadLogger
	/** log a message at a particular level
	@param[in] level the level of the message
	@param[in] logMessage the actual message to log
	*/
	void log(int level, const std::string &logMessage);
	/** message to log without regard for levels*
	@param[in] logMessage the message to log
	*/
	void log(const std::string &logMessage)
	{
		log(-100000, logMessage);
	}
	/** check if the logging thread is running*/
	bool isRunning() const;
	/** flush the log queue*/
	void flush();
	/** alter the printing levels
	@param[in] cLevel the level to print to the console
	@param[in] fLevel the leve to print to the file if it is open*/
	void changeLevels(int cLevel, int fLevel);

};
}//namespace utilities;
#endif
