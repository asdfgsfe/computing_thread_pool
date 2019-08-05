#ifndef __THREAD_POOL_H__
#define __THREAD_POOL_H__

#include <pthread.h>
#include <functional>
#include <deque>
#include <vector>
#include <string>
#include <boost/noncopyable.h>
#include "Thead.h"
#include "Mutex.h"
#include "Condition.h"

class TheadPool : boost::noncoptyable
{
	using Task = std::fuction<void ()>;
public:
	explicit TheadPool(std::string name = "threadPool");
	~TheadPool();
	void Run(Task t);
	void Start(int threadNums);
	void Stop();

	void SetQueSize(int size) { maxQueSize_ = size; };
	void SetTheadInitCallback(Task cb) { threadInitCallback = std::move(cb); }
	const std::string& Name() const { return name_; }
private:
	bool IsFull() const;
	Task Take() const;
	void RunInThread();
private:
	bool runing_;
	std::string name_;
	int maxqueSize_;
	Task threadInitCallback_;
	std::vector<std::unque_ptr<Thead>> threads_;
	std::deque<Task> tasks_;
	MutexLock mutex_;
	Condition notEmpty_ GUARDED_BY(mutex_);
	Condition notFull_ GUARDED_BY(mutex_);
};

#endif //__THREAD_POOL_H__
