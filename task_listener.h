#pragma once
#include "phpgo_context.h"

using namespace co;
	
class PhpgoTaskListener : public Scheduler::TaskListener{
	
	// the go routine sepecific EG's are stored in task local storage
	// thus each task has it's own copy
	kls_key_t                         phpgo_context_key;
	Scheduler::TaskListener*          old_task_listener;
	
public:
	PhpgoTaskListener(Scheduler::TaskListener* task_listener){
		old_task_listener = task_listener;
		phpgo_context_key = TaskLocalStorage::CreateKey("PhpgoContext");
	}
	
	/*
	virtual void onStart(uint64_t task_id) noexcept {
		printf("onStart(%ld)", task_id);
	}*/
	
	/**
	 * 协程切入后（包括协程首次运行的时候）调用
	 * 协程首次运行会在onStart之前调用。
	 * （本方法运行在协程中）
	 *
	 * @prarm task_id 协程ID
	 * @prarm eptr
	 */
	virtual void onSwapIn(uint64_t task_id) noexcept {
		//printf("---------->onSwapIn(%ld)<-----------\n", task_id);
		
		if(old_task_listener){
			old_task_listener->onSwapIn(task_id);
		}

		PhpgoSchedulerContext* sched_ctx = &scheduler_ctx; /*scheduler_ctx is thread local*/
		PhpgoContext* ctx = (PhpgoContext*)TaskLocalStorage::GetSpecific(phpgo_context_key);

		// first time swap into a task:
		// return - not to do the PHPGO_SAVE/LOAD_CONTEXT since they will be done in the
		// phpgo_go()
		if(!ctx) return;
		
		// running -> sched_ctx and ctx -> running
		bool including_http_globals = ctx->go_routine_options & GoRoutineOptions::gro_isolate_http_globals;
		sched_ctx->SwapOut(including_http_globals);
		ctx->SwapIn(including_http_globals);

		//printf("---------->onSwapIn(%ld) returns<-----------\n", task_id);
	}

	/**
	 * 协程切出前调用
	 * （本方法运行在协程中）
	 *
	 * @prarm task_id 协程ID
	 * @prarm eptr
	 */
	virtual void onSwapOut(uint64_t task_id) noexcept {
		//printf("---------->onSwapOut(%ld)<-----------\n", task_id);
		if(old_task_listener){
			old_task_listener->onSwapOut(task_id);
		}

		PhpgoSchedulerContext* sched_ctx = &scheduler_ctx;
		PhpgoContext* ctx = (PhpgoContext*)TaskLocalStorage::GetSpecific(phpgo_context_key);
		if(!ctx) return;
		
		// running -> ctx and sched_ctx -> running
		bool including_http_globals = ctx->go_routine_options & GoRoutineOptions::gro_isolate_http_globals;
		ctx->SwapOut(including_http_globals);
		sched_ctx->SwapIn(including_http_globals);
		
		//printf("---------->onSwapOut(%ld) returns<-----------\n", task_id);
	}
};

