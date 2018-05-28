#pragma once
#include "co_recursive_mutex.h"

/*
 * mutex
 */
class GoMutex{
public:
	static void* Create(bool signaled){
		return new ::co::CoRecursiveMutex(signaled);
	}

	static void Lock(void* mutex){
		auto mutex_obj = (::co::CoRecursiveMutex*)mutex;
		
		// if it's not in any go routine,
		// run the scheduler (so that the go routines will run and may
		// release lock) until the lock is obtained 
		if( !co_sched.IsCoroutine() ){
			while( !mutex_obj->try_lock() ) {
				co_sched.Run();
			}
			return;
		}
		
		// in a go routine, it's safe to just lock 
		mutex_obj->lock();
	}

	static void Unlock(void* mutex){
		auto mutex_obj = (::co::CoRecursiveMutex*)mutex;
		mutex_obj->unlock();
	}

	static bool TryLock(void* mutex){
		auto mutex_obj = (::co::CoRecursiveMutex*)mutex;
		return mutex_obj->try_lock();
	}

	static bool IsLock(void* mutex){
		auto mutex_obj = (::co::CoRecursiveMutex*)mutex;
		return mutex_obj->is_lock();
	}

	static void Destroy(void* mutex){
		auto mutex_obj = (::co::CoRecursiveMutex*)mutex;
		delete mutex_obj;
	}
};