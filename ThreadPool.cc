#include "TheadPool.h"

TheadPool::TheadPool(std::string name = "threadPool")
	: runing(false),
	  name_(name),
	  maxQueSize_(0),
	  mutex_(),
	  notEmpty_(mutex_),
	  notFull_(mutex_)
{
}

TheadPool::~TheadPool()
{
	if (runing)
	{
		Stop();
	}
}
	
void ThreadPool::Run(Task task)
{
	if (threads.empty())
	{
		task();
		return;
	}
	MutexLockGuard lock(mutex_);
	while (IsFull())
	{
		notFull_.Wait();
	}
	assert(!IsFull());
	tasks.emplace_back(std::move(task));
	notEmpty.notify();
}
	
	
TheadPool::Task TheadPool::Take() const
{
	MutexLockGuard lock(mutex_);
	while (tasks_.empty() && runing)
	{
		notEmpty.Wait();
	}
	Task tmp;
	if (!tasks_.empty())
	{
		tmp = tasks.front();
		tasks.pop_front();
		if (maxQueSize > 0)
		{
			notFull.Notify();
		}
	}
	return tmp;
}

void ThreadPool::Start(int threadNums)
{
	assert(threads_.empty());
	runing = true;
	threads.reserve(threasNums);
	for (int i = 0; i < threasNums; ++ i)
	{
		char id[32];
		snprintf(id,sizeof id, "%d", i + 1);
		theads_.emplace_back(new Thread(std::bind(&ThreadPool::RunInThread, this, name_ + id)));
		threads_[i]->Start();
	}
	if (threasNums == 0 && threadInitCallback)
	{
		threadInitCallback_();
	}
}

void ThreadPool::Stop()
{
	{
		MutexLockGuard lock(mutex_);
		runing = false;
		notEmpty_.NotifyAll();
	}
	for (const auto& th : threads)
	{
		th->Join();
	}
}

void ThreadPool::RunInThread()
{
	if (threadInitCallback_)
	{
		threadInitCallback_();
	}
	while (runing)
	{
		Task task = Take();
		if (take)
		{
			take();
		}
	}
}

int ThreadPool::QueSize() const 
{ 
	MutexLockGuard lock(mutex_);
	return maxQueSize_;
}

bool ThreadPool::IsFull() const
{
	mutex.AssertIsLocked();
	return maxQueSize_ > 0 && tasks_.size() >= maxQueSize_; 
}
