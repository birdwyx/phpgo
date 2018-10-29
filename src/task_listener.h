#pragma once
#include "php_phpgo.h"
#include "phpgo_context.h"

using namespace co;

ZEND_EXTERN_MODULE_GLOBALS(phpgo)

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
	
	virtual void onCreated(uint64_t task_id) noexcept {
		printf("onCreated(%ld)", task_id);
		
		g_task_id_created = task_id;
	}
	
	virtual void onStart(uint64_t task_id) noexcept {
		printf("onStart(%ld)", task_id);
		
		PhpgoContext* ctx = (PhpgoContext*)TaskLocalStorage::GetSpecific(phpgo_context_key, task_id);
	}
	
	/**
	 * run by the scheduler each time when the task is about to swap in
	 */
	virtual void onSwapIn(uint64_t task_id) noexcept {
		//printf("---------->onSwapIn(%ld)<-----------\n", task_id);
		/*defer{
			printf("---------->onSwapIn(%ld) returns<-----------\n", task_id);
		};*/
		
		if(old_task_listener){
			old_task_listener->onSwapIn(task_id);
		}

		PhpgoSchedulerContext* sched_ctx = &scheduler_ctx; /*scheduler_ctx is thread local*/
		PhpgoContext* ctx = (PhpgoContext*)TaskLocalStorage::GetSpecific(phpgo_context_key, task_id);

		// first time swap into a task:
		// return - not to do the PHPGO_SAVE/LOAD_CONTEXT since they will be done in the
		// phpgo_go()
		if(!ctx) return;
		
		// running -> sched_ctx and ctx -> running
		bool including_http_globals = ctx->go_routine_options & GoRoutineOptions::gro_isolate_http_globals;
		sched_ctx->SwapOut(including_http_globals);
		ctx->SwapIn();
		
		for(int i=0; i<ctx->listener_count; i++){
			ctx->listeners[i]->onSwapIn(task_id);
		}
	}

	/**
	 * run by the task each time when it's about to swap out
	 */
	virtual void onSwapOut(uint64_t task_id) noexcept {
		//printf("---------->onSwapOut(%ld)<-----------\n", task_id);
		/*defer{
			printf("---------->onSwapOut(%ld) returns<-----------\n", task_id);
		};*/
		
		if(old_task_listener){
			old_task_listener->onSwapOut(task_id);
		}

		PhpgoSchedulerContext* sched_ctx = &scheduler_ctx;
		PhpgoContext* ctx = (PhpgoContext*)TaskLocalStorage::GetSpecific(phpgo_context_key, task_id);
		if(!ctx) return;
		
		// running -> ctx and sched_ctx -> running
		ctx->SwapOut();
		bool including_http_globals = ctx->go_routine_options & GoRoutineOptions::gro_isolate_http_globals;
		sched_ctx->SwapIn(including_http_globals);
		
		for(int i=0; i<ctx->listener_count; i++){
			ctx->listeners[i]->onSwapIn(task_id);
		}
		
		if (ctx->go_routine_finished){
			ctx->Cleanup();
			
			for(int i=0; i<ctx->listener_count; i++){
				ctx->listeners[i]->onEnd(ctx->listeners[i]);
			}
		
			TaskLocalStorage::FreeSpecifics(task_id);
		}
	}
	
	/**
	 * run by the task each time when it's going to finish running, either normally or abnormally
	 */
	virtual void onFinished(uint64_t task_id, const std::exception_ptr eptr) noexcept {
		//printf("---------->onFinished(%ld)<-----------\n", task_id);
		/*defer{
			printf("---------->onFinished(%ld) returns<-----------\n", task_id);
		};*/
		
		PhpgoContext* ctx = (PhpgoContext*)TaskLocalStorage::GetSpecific(phpgo_context_key, task_id);
		if(!ctx) return;
		
		/*cannot do free task local storage here as the onSwapOut still need to use it*/
		/*set flag so that onSwapOut can do the task local storage cleanup*/
		ctx->go_routine_finished = true;
	}
};

