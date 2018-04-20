#pragma once
#include <libgo/scheduler.h>

#ifndef co_sched
#define co_sched g_Scheduler
#endif

class GoScheduler{
public:
	static uint32_t Run(){
		if (co_sched.IsCoroutine()) {
			zend_error(E_ERROR, "phpgo: error: run must be called outside a go routine\n");
			return 0;
		}
		return co_sched.Run();
	}
	
	static void Join(uint32_t tasks_left){
		if (co_sched.IsCoroutine()) {
			zend_error(E_ERROR, "phpgo: error: join must be called outside a go routine\n");
			return;
		}	
		co_sched.RunUntilNoTask(tasks_left);
	}
	
	static void Loop(){
		if (co_sched.IsCoroutine()) {
			zend_error(E_ERROR, "phpgo: error: loop must be called outside a go routine\n");
			return;
		}
		co_sched.RunLoop();
	}
};