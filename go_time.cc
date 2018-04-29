#include "stdinc.h"
#include "php_phpgo.h"
#include "go_time.h"
#include "zend_API.h"
#include "go_chan.h"
#include <iostream>

THREAD_LOCAL TimerSet GoTime::tls_timer_set;

bool GoTime::CreateTimer(
	const char* chan_name,
	uint64_t    nano_seconds, 
    bool        is_periodic,
	bool&       go_creation
){
	static THREAD_LOCAL co_chan<TimerData*>* td_chan = nullptr;
	
	go_creation = false;
	if( !td_chan ){
		td_chan = new co_chan<TimerData*>(MAX_TIMER_CHAN_CAPACITY);
		if( !td_chan ) {
			zend_error(E_ERROR, "faile to creat channel for go timer processor");
			return false;
		}

		CreateGoRoutine(td_chan); 
		go_creation = true;
	}
	
	// push the timer data to chan
	auto td = new TimerData(chan_name, nano_seconds, is_periodic);
	*td_chan << td;
	
	return true;
}

void GoTime::CreateGoRoutine( co_chan<TimerData*>* td_chan ){
	go [=](){
		while(true){
			TimerData* td = nullptr;

			/*proccess expired timer*/
			while( td = PopExpired() ){
				zval* z = nullptr;
				void* timer_chan = nullptr;
				defer{
					delete td; 
					if(z) zval_ptr_dtor(&z); 
					if(timer_chan) {
						// finish using the timer_chan, del-ref it
						// the timer_chan was just add-ref'ed by GoChan::Create() below
						GoChan::Destroy(timer_chan); 
					}
				}; // will defer work in while?? will need to confirm

				timer_chan = GoChan::Create(1, td->chan_name, strlen(td->chan_name), true /*copy=true*/);
				if(!timer_chan){
					zend_error(E_WARNING, "faile to get channel for timer %s", td->chan_name);
					continue;
				}
				//GoChan::Close(timer_chan); // close chan to inform timer creator
				
				//return the current time,in the same format as microtime()
				struct timeval tp = {0}; char str[36];
				gettimeofday(&tp, NULL);
				snprintf(str, 32, "%.8F %ld", tp.tv_usec / 1000000.00, tp.tv_sec);
				
				TSRMLS_FETCH();
				MAKE_STD_ZVAL(z);
				ZVAL_STRING(z,str,1);
				//ZVAL_LONG(z,1);
				
				if( !GoChan::TryPush(timer_chan, z TSRMLS_CC) ){
					zend_error(E_WARNING, "faile to activate timer %s on expiry: push to timer channel failed", td->chan_name);
					continue;
				}
			}

			/*check if any new timer to create*/
			while( td_chan->TryPop(td) ){
				if(td)	PlaceTimer(td);
			}
			
			usleep( 10*1000 );  // sleep for a 10ms
		}
	};
}

void GoTime::PlaceTimer(TimerData* td){
	td->expire_tick = Clock() + td->delta;;
	tls_timer_set.insert(td);		
}

TimerData* GoTime::PopExpired(){
	auto it = tls_timer_set.begin();
	if( it == tls_timer_set.end() )
		return nullptr;
	
	auto td = *it;
	if( td->expire_tick > Clock() ) return nullptr;

	tls_timer_set.erase(it);
	if( td->is_periodic ){
		auto td_next = new TimerData(*td);
		PlaceTimer(td_next);
	}
	return td;
}

