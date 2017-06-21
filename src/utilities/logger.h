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
class logger
{

private:
	simpleQueue<std::string> loggingQueue;
	std::atomic<bool> emptyFlag{ true };
	std::ofstream outFile;
	std::mutex cvMutex;
	std::condition_variable notifier;
	std::thread loggingThread;
public:
	int consoleLevel = 100;
	int fileLevel = 100;
public:
	logger();
	~logger();
	void openFile(const std::string &file);
	void startLogging(int cLevel, int fLevel);
	void startLogging()
	{
		startLogging(consoleLevel, fileLevel);
	}
	void log(int level, std::string logMessage);
	void log(std::string logMessage)
	{
		log(-100000, logMessage);
	}
	bool isRunning() const;
	void flush();
	void changeLevels(int cLevel, int fLevel);
private:
	void loggerLoop();
	void clearEmptyFlag();
};

class loggerNoThread
{
private:
	std::ofstream outFile;
public:
	int consoleLevel = 100;
	int fileLevel = 100;
public:
	loggerNoThread();
	void openFile(const std::string &file);
	void startLogging(int cLevel, int fLevel);
	void startLogging()
	{
		startLogging(consoleLevel, fileLevel);
	}
	void log(int level, const std::string &logMessage);
	void log(const std::string &logMessage)
	{
		log(-100000, logMessage);
	}
	bool isRunning() const;
	void flush();
	void changeLevels(int cLevel, int fLevel);

};
}//namespace utilities;
#endif
