#pragma once
#include "global_defs.h"
#include "defer.h"
#include "zend.h"
#include "TSRM.h"
#include "go_chan.h"
#include <set>
#include <libgo/coroutine.h>

using namespace std;

#define MAX_TIMER_CHAN_CAPACITY (1000)

struct TimerData{
	char     chan_name[MAX_CHAN_NAME_LEN +  sizeof(uint32_t)];
	uint64_t delta;            // in nanoseconds
	bool     is_periodic;
	uint64_t expire_tick;      // the tick @ which this timer should expire

public:
	TimerData(const char* chan_name, uint64_t delta, bool is_periodic){
		strncpy(this->chan_name, chan_name, MAX_CHAN_NAME_LEN);
		*(uint32_t*)&(this->chan_name[MAX_CHAN_NAME_LEN]) = 0; //padding 4 zeros
		this->is_periodic = is_periodic;
		this->delta = delta;
		this->expire_tick = 0;
	}

	TimerData(TimerData& td){
		*this = td;
	}
};

class TickLess{
public:
	bool operator()(const TimerData* td1, const TimerData* td2){
		return td1->expire_tick < td2->expire_tick;
	}
};

typedef multiset<TimerData*, TickLess> TimerSet;

class GoTime{
private: 
	static THREAD_LOCAL TimerSet tls_timer_set;
private:
	static void       CreateGoRoutine( co_chan<TimerData*>* td_chan );
	static void       PlaceTimer( TimerData* td );
	static TimerData* PopExpired();
	static uint64_t   Clock(){
		struct timespec tv;
		clock_gettime(CLOCK_MONOTONIC, &tv);
		return tv.tv_sec * GoTime::Second + tv.tv_nsec;
	}
	
public: 
	static const uint64_t Nanosecond  = 1;
	static const uint64_t Microsecond = 1000 * Nanosecond;
	static const uint64_t Millisecond = 1000 * Microsecond;
	static const uint64_t Second      = 1000 * Millisecond;
	static const uint64_t Minute      = 60 * Second;
	static const uint64_t Hour        = 60 * Minute;
	
public:
	static bool  CreateTimer(const char* chan_name, uint64_t micro_seconds, 
	                         bool is_periodic, bool& go_creation);
	static void  Sleep(uint64_t nanoseconds){
		if(co_sched.IsCoroutine()){
			co_sleep(nanoseconds/Millisecond);
		}else{
			usleep(nanoseconds/Microsecond);
		}
	}

};