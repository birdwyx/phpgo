#pragma once
#include <libgo/scheduler.h>

#ifndef co_sched
#define co_sched g_Scheduler
#endif

class GoScheduler{
public:
	static void RunOnce(){
		if (co_sched.IsCoroutine()) {
			zend_error(E_ERROR, "phpgo: error: go_schedule_once must be called outside a go routine\n");
			return;
		}
		co_sched.Run();
	}
	
	static void RunJoinAll(){
		if (co_sched.IsCoroutine()) {
			zend_error(E_ERROR, "phpgo: error: go_schedule_all must be called outside a go routine\n");
			return;
		}	
		co_sched.RunUntilNoTask();
	}
	
	static void RunForever(){
		if (co_sched.IsCoroutine()) {
			zend_error(E_ERROR, "phpgo: error: go_schedule_forever must be called outside a go routine\n");
			return;
		}
		co_sched.RunLoop();
	}
};