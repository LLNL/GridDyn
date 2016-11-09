#pragma once

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

#ifndef WORKQUEUE_H_
#define WORKQUEUE_H_
#include <thread>
#include <memory>
#include <queue>
#include <mutex>
#include <future>
/** basic work block abstract class*/
class basicWorkBlock
{
public:
	basicWorkBlock() {};
	virtual ~basicWorkBlock() {};
	/** run the work block*/
	virtual void execute() = 0;
	/** check if the work is finished
	@return true if the work has been done false otherwise
	*/
	virtual bool isFinished() const=0;
};

/** implementation of a workBlock class
The class takes as an input object of some kind function, std::function, lambda that can be executed
*/
template <typename Func, typename retType>
class workBlock :public basicWorkBlock
{
public:
	workBlock() { reset(); };
	workBlock(const Func &newWork) :loaded(true),workFunc(newWork) { reset(); };
	workBlock(Func &&newWork) :loaded(true),workFunc(newWork) { reset(); };
	~workBlock()
	{

	};
	virtual void execute() override
	{
		if (!finished)
		{
			if (loaded)
			{
				promise_val.set_value(workFunc());
			}
			else
			{
				promise_val.set_value(retType());
			}
			finished = true;
		}
	};
	retType getReturnVal() const
	{
		return future_ret.get();
	};
	void updateWorkFunction(const Func &newWork)
	{
		workFunc = newWork;
		if (finished)
		{
			reset();
		}
		loaded = true;
	};
	void updateWorkFunction(Func &&newWork)
	{
		workFunc = newWork;
		if (finished)
		{
			reset();
		}
		loaded = true;
	};
	bool isFinished() const override
	{
		return finished;
	};
	void wait()
	{
		future_ret.wait();
	};
	void reset()
	{
		finished = false;
		promise_val = std::promise<retType>();
		future_ret = std::shared_future<retType>(promise_val.get_future());
	};
private:
	bool finished = false;
	bool loaded = false;
	std::promise<retType> promise_val;
	std::shared_future<retType> future_ret;
	Func workFunc;
};

/** implementation of a workBlock class with void return type
The class takes as an input object of some kind function, std::function, lambda that can be executed
*/
template <typename Func>
class workBlock<Func,void> :public basicWorkBlock
{
public:
	workBlock() { reset(); };
	workBlock(const Func &newWork) :loaded(true),workFunc(newWork) { reset(); };
	workBlock(Func &&newWork) :loaded(true),workFunc(newWork) { reset(); };
	~workBlock()
	{

	};
	virtual void execute() override
	{
		if (!finished)
		{
			if (loaded)
			{
				workFunc();
			}
			promise_val.set_value();
			finished = true;
		}
	};
	void getReturnVal() const
	{
		future_ret.wait();
	};
	void updateWorkFunction(const Func &newWork)
	{
		workFunc = newWork;
		if (finished)
		{
			reset();
		}
		loaded = true;
	};
	void updateWorkFunction(Func &&newWork)
	{
		workFunc = newWork;
		if (finished)
		{
			reset();
		}
		loaded = true;
	};
	bool isFinished() const override
	{
		return finished;
	};
	void wait() const
	{
		future_ret.wait();
	};
	void reset()
	{
		finished = false;
		promise_val = std::promise<void>();
		future_ret = std::shared_future<void>(promise_val.get_future());
	};
private:
	bool finished = false;
	bool loaded = false;
	std::promise<void> promise_val;
	std::shared_future<void> future_ret;
	Func workFunc;
};

template <typename X>
auto make_workBlock(X fptr)->std::shared_ptr<workBlock<X,decltype(fptr())>>
{
	return std::make_shared<workBlock<X,decltype(fptr())>>(fptr);
}

const int defaultPriorityRatio(4);

/** class defining a work queuing system
implemented with 3 priority levels high medium and low
high is executed as a soon as possible in order
medium is executed with X times more frequently than low with X being defined by the priority ratio
*/
class workQueue
{

public:
	/** enumeration defining the work block priority*/
	enum class workPriority
	{
		medium,
		low,
		high
	};
	/**
	* class destructor must be public so it can be used with shared_ptr
	*/
	~workQueue();  //destructor

	/** get an instance of the workQueue
	* @param[in] threadCount the number of threads
	* Return the current instance of the singleton.
	*
	*/
	static std::shared_ptr<workQueue> instance(int threadCount=-1);
	/** get the number of workers
	static so it can be called before instance is valid
	@return int with the current worker count
	*/
	static int getWorkerCount();
	/** destroy the workQueue*/
	void destroyWorkerQueue();
	/** add a block of work to the workQueue
	@param[in] newWork  the block of new work for the queue
	*/
	void addWorkBlock(std::shared_ptr<basicWorkBlock> newWork,workPriority priority=workPriority::medium);
	/** add a block of work to the workQueue
	@param[in] newWork  a vector of workBlocks to add to the queue
	*/
	void addWorkBlock(std::vector<std::shared_ptr<basicWorkBlock>> &newWork, workPriority priority = workPriority::medium);
	/** check if the queue is empty
	@return true if the queue is empty false if it has work to do
	*/
	bool isEmpty() const { return (numBlocks == 0); };
	/** get the number of remaining blocks*/
	int numBlock() const { return numBlocks; };
	/** set the ratio of medium block executions to low priority block executions
	@param[in] newPriorityRatio  if >0 used as the new ratio otherwise set to default value*/
	void setPriorityRatio(int newPriorityRatio) { priorityRatio = (newPriorityRatio > 0) ? newPriorityRatio : defaultPriorityRatio; };
	/** get the next work block
	@return a shared pointer to a work block
	*/
	std::shared_ptr<basicWorkBlock> getWorkBlock();
private:
	/** the main worker loop*/
	void workerLoop();

	/**
	* Singleton so prevent external construction and copying of this
	* class.
	@param[in] threadCount  the number of threads in the queue (<0 for default value)
	*/
	explicit workQueue(int threadCount);
	workQueue(workQueue const&) = delete;
	workQueue& operator= (workQueue const&) = delete;

	
	/**
	* Singleton instance.
	*/
	static std::shared_ptr<workQueue> pInstance;  //!< the pointer to the singleton
	bool halt=false;  //!< flag indicating the threads should halt
	int priorityRatio = defaultPriorityRatio; //!< the ratio of medium Priority blocks to low priority blocks
	int numWorkers = 0; //!< counter for the number of workers
	int MedCounter = 0; //!< the counter to use low instead of Med
	int numBlocks = 0;	//!< counter for the number of blocks remaining
	std::queue<std::shared_ptr<basicWorkBlock>> workToDoHigh; //!< queue containing the work to do
	std::queue<std::shared_ptr<basicWorkBlock>> workToDoMed; //!< queue containing the work to do
	std::queue<std::shared_ptr<basicWorkBlock>> workToDoLow; //!< queue containing the work to do
	
	std::condition_variable queueCondition; //!< condition variable for waking the threads
	std::mutex queueLock; //!< mutex lock for ensuring concurrent access on the threads
	std::vector<std::thread> threadpool; //!< the threads
};

#endif