#ifndef __WAIT_GROUP_H__
#define __WAIT_GROUP_H__

#include <libgo/co_mutex.h>
#include <libgo/scheduler.h>

namespace co{
	
class CoWaitGroup
{
	CoMutex  mutex_;
	int64_t  count_;

public:
    CoWaitGroup() : count_(0) {}
    ~CoWaitGroup() {}

    int64_t Add(int64_t delta){
		mutex_.lock();
		count_ += delta;
		mutex_.unlock();
		
		return count_;
	}
	
    int64_t Done(){
		return Add(-1);
	}
	
	int64_t Count(){
		return count_;
	}
	
    void Wait(){
		
		// if not in any go routine
		// run the scheduer until the count reaches 0
		// otherwise yield until the count reaches 0
		
		if( !g_Scheduler.GetCurrentTaskID() ){
			while(count_ > 0){
				g_Scheduler.Run();
			}
		}else{
			while( count_ > 0 ){
				g_Scheduler.CoYield();
			}
		}
	}
};

}
#endif