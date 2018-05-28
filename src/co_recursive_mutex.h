#ifndef __MUTEX_H__
#define __MUTEX_H__

#include <libgo/co_mutex.h>
#include <libgo/scheduler.h>

namespace co{
	
class CoRecursiveMutex
{
    CoMutex  mutex_;
	uint64_t task_id_;
	uint64_t lock_count_;

public:
    CoRecursiveMutex(bool signaled = true) : task_id_(~0), lock_count_(0){
		if(!signaled){
			mutex_.lock();
			lock_count_ = 1;
			task_id_ = g_Scheduler.GetCurrentTaskID();
		}
	}
	
    ~CoRecursiveMutex() {}

    void lock(){
		auto my_task_id = g_Scheduler.GetCurrentTaskID();
		if(lock_count_ > 0	&& task_id_ == my_task_id){
			// locked by myself
			++lock_count_;
			return;
		}
		
		//1. not locked, or 2. locked by other task
		mutex_.lock();
		++ lock_count_;
		task_id_ = my_task_id;
		return;
	}
	
    bool try_lock(){
		auto my_task_id = g_Scheduler.GetCurrentTaskID();
		if(lock_count_ > 0	&& task_id_ == my_task_id){
			// locked by myself
			++lock_count_;
			return true;
		}
		
		//1. not locked, or 2. locked by other task
		if( !mutex_.try_lock() ) return false;
		++ lock_count_;
		task_id_ = my_task_id;
		return true;
	}
	
    bool is_lock(){
		return mutex_.is_lock();
	}
	
    void unlock(){
		auto my_task_id = g_Scheduler.GetCurrentTaskID();
		/*
		can free mutex not owned by myself
		in theory there could be 2 threads releasing the lock at the same
		time, thus need an extra lock here to ensure the code before <-- is atomic
		however releasing lock owned by other thread is not an often case,
		we chose to ignore this case for optimal performance... 
		*/
		if(lock_count_ > 0 /*&& task_id_ == my_task_id*/)
		{
			--lock_count_;
			if(lock_count_ == 0){
				task_id_ = ~0;
				mutex_.unlock();
			}
		}
		/*<--*/
	}
};

}
#endif