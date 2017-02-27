
/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil;  eval: (c-set-offset 'innamespace 0); -*- */
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

#include "workQueue.h"
#include "griddyn-config.h"

std::shared_ptr<workQueue> workQueue::pInstance;

std::mutex workQueue::instanceLock;

workQueue::workQueue(int threadCount):numWorkers(threadCount)
{
#ifdef DISABLE_MULTITHREADING
	numWorkers = 0;
#else
	if (numWorkers < 0)
	{
		numWorkers = static_cast<int>(std::thread::hardware_concurrency())+1;
	}
	threadpool.resize(numWorkers);
	for (int kk = 0; kk < numWorkers; ++kk)
	{
		threadpool[kk] = std::thread(&workQueue::workerLoop, this);
	}
#endif
}

workQueue::~workQueue()
{
	if (halt == false)
	{
		halt = true;
		queueCondition.notify_all();
	}
	pInstance = nullptr;
	for (auto &thrd : threadpool)
	{
		if (thrd.joinable())
		{
			thrd.join();
		}
	}
}


void workQueue::addWorkBlock(std::shared_ptr<basicWorkBlock> newWork, workPriority priority)
{
	if ((!newWork)||(newWork->isFinished()))
	{
		return;
	}
	if (numWorkers > 0)
	{
		int ccount;
		//new code block for mutex lock scoping
		{
			std::lock_guard<std::mutex> lock(queueLock);
			ccount = numBlocks;
			switch (priority)
			{
			case workPriority::high:
				workToDoHigh.push(newWork);
				break;
			case workPriority::medium:
				workToDoMed.push(newWork);
				break;
			case workPriority::low:
			default:
				workToDoLow.push(newWork);
				break;
			}
			++numBlocks;
		}//mutex is freed at this point
		if (ccount<=numWorkers)
		{
			queueCondition.notify_one();
		}
		
	}
	else
	{
		newWork->execute();
	}
}

void workQueue::addWorkBlock(std::vector<std::shared_ptr<basicWorkBlock>> &newWork, workPriority priority)
{
	if (numWorkers > 0)
	{
		{ //new scope block for the mutex
			std::lock_guard<std::mutex> lock(queueLock);
			switch (priority)
			{
			case workPriority::high:
				for (auto &wb : newWork)
				{
					workToDoHigh.push(wb);
				}
				break;
			case workPriority::medium:
				for (auto &wb : newWork)
				{
					workToDoMed.push(wb);
				}
				break;
			case workPriority::low:
			default:
				for (auto &wb : newWork)
				{
					workToDoLow.push(wb);
				}
				break;
			}
			numBlocks+=static_cast<int>(newWork.size());
			
		}
		queueCondition.notify_all();
	}
	else
	{
		for (auto &wb : newWork)
		{
			wb->execute();
		}
	}
}

std::shared_ptr<basicWorkBlock> workQueue::getWorkBlock()
{
	if (numBlocks==0)
	{
		return nullptr;
	}

	std::shared_ptr<basicWorkBlock> wb;
	std::lock_guard<std::mutex> lock(queueLock);
	if (!workToDoHigh.empty())
	{
		wb = workToDoHigh.front();
		workToDoHigh.pop();
		--numBlocks;
	}
	else if (!workToDoMed.empty())
	{
		if (MedCounter >= priorityRatio)
		{
			if (!workToDoLow.empty())
			{
				wb = workToDoLow.front();
				workToDoLow.pop();
				--numBlocks;
			}
			else
			{
				wb = workToDoMed.front();
				workToDoMed.pop();
				--numBlocks;
			}
			MedCounter = 0;
		}
		else
		{
			wb = workToDoMed.front();
			workToDoMed.pop();
			--numBlocks;
			++MedCounter;
		}
	}
	else if (!workToDoLow.empty())
	{
		wb = workToDoLow.front();
		workToDoLow.pop();
		--numBlocks;
	}

	return wb;
}

std::shared_ptr<workQueue> workQueue::instance(int threadCount)
{
	if (!pInstance)
	{
		//this is to ensure we don't create it multiple times resulting in some weird condition or just a waste of resources

		std::lock_guard<std::mutex> creationLock(instanceLock);
		if (!pInstance)
		{
			pInstance = std::shared_ptr<workQueue>(new workQueue(threadCount));
		}
	}
	return pInstance;
}

int workQueue::getWorkerCount()
{
	if (!pInstance)
	{
		return 0;
	}
	return pInstance->numWorkers;
}

void workQueue::destroyWorkerQueue()
{
	auto tempPtr = pInstance;
	if (tempPtr == pInstance)
	{
		std::lock_guard<std::mutex> creationLock(instanceLock);
		pInstance = nullptr;
	}
	halt = true;
	workToDoHigh = decltype(workToDoHigh)();
	workToDoMed = decltype(workToDoMed)();
	workToDoLow = decltype(workToDoLow)();
	queueCondition.notify_all();
	
}

void workQueue::workerLoop()
{
	std::unique_lock<std::mutex> lv(queueLock,std::defer_lock);
	while (halt == false)
	{
		if (isEmpty())
		{
			lv.lock();
			queueCondition.wait(lv);
			lv.unlock();
		}
		else
		{
			auto wb = getWorkBlock();
			if ((wb)&&(!wb->isFinished()))
			{
				wb->execute();
			}
		}
	}
}