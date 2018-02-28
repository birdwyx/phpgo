#pragma once
#include <libgo/freeable.h>
#include <libgo/task_local_storage.h>

using namespace co;

#ifdef ZTS
	#define TSRMLS_FIELD TSRMLS_D
#else
	#define TSRMLS_FIELD
#endif

#define __PHPGO_CONTEXT_FIELDS__ 	                    \
	TSRMLS_FIELD;     /*ZTS: void ***tsrm_ls;*/         \
	struct _zend_execute_data* EG_current_execute_data; \
	zend_vm_stack 			   EG_argument_stack;       \
	zend_class_entry*		   EG_scope;                \
	zval*					   EG_This;                 \
	zend_class_entry*		   EG_called_scope;         \
	HashTable*				   EG_active_symbol_table;  \
	zval**					   EG_return_value_ptr_ptr; \
	zend_op_array*			   EG_active_op_array;      \
	zend_op**				   EG_opline_ptr;           \
	zval                       EG_error_zval;           \
	zval*                      EG_error_zval_ptr;       \
	zval*                      EG_user_error_handler;   \

struct PhpgoContext : public FreeableImpl{
	__PHPGO_CONTEXT_FIELDS__
};

// the Scheduler Context is essentially the same as the Task's context,
// but since Scheduler Context  is thread local and thread locals cannot
// have virtual members, we have to remove the FreeableImpl from the
// Scheduler Context...
struct PhpgoSchedulerContext{
	__PHPGO_CONTEXT_FIELDS__
};

// the scheduler may be executed in multiple thread: 
// use thread local variable to store the scheduler EG's	
static thread_local PhpgoSchedulerContext scheduler_ctx;


	
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
		
		PhpgoContext* ctx;
		if( !(ctx = (PhpgoContext*)TaskLocalStorage::GetSpecific(phpgo_context_key)) ){
			ctx = new PhpgoContext();
			TaskLocalStorage::SetSpecific(phpgo_context_key, ctx);
		}
		
		PhpgoSchedulerContext* sched_ctx    =  &scheduler_ctx;
		
	#ifdef ZTS
	
		/*
		* for optimal performance:
		* cache the tsrm_ls for this scheduler thread into this scheduler context
		* so that we don't need to fetch from the operating system tls every time.
		*/
		if( UNEXPECTED(!sched_ctx->TSRMLS_C) ){
			//this is the first time task run, 
			//fetch and store the thread specific tsrm_ls to local context
			TSRMLS_FETCH();                          // void ***tsrm_ls = (void ***) ts_resource_ex(0, NULL)
			TSRMLS_SET_CTX(sched_ctx->TSRMLS_C);     // sched_ctx->tsrm_ls = (void ***) tsrm_ls
		}
		
		/*
		* Fetch the thread specific tsrm_ls from the scheduler context 
		* (which is thread local) this will also work under work-steal since
		* the tsrm_ls fetched is always for the current thread running 
		* the scheduler, thus the EG saved in onSwapIn() and retored in
		* onSwapOut() are always of this scheduler
		*/
		TSRMLS_FETCH_FROM_CTX(sched_ctx->TSRMLS_C);  //	void ***tsrm_ls = (void ***)sched_ctx->tsrm_ls
	#endif
		
		// save the scheduler EGs first
		sched_ctx->EG_current_execute_data  =  EG(current_execute_data    );
		sched_ctx->EG_argument_stack        =  EG(argument_stack          );
		sched_ctx->EG_scope                 =  EG(scope                   );
		sched_ctx->EG_This                  =  EG(This                    );
		sched_ctx->EG_called_scope          =  EG(called_scope            );
		sched_ctx->EG_active_symbol_table   =  EG(active_symbol_table     );
		sched_ctx->EG_return_value_ptr_ptr  =  EG(return_value_ptr_ptr    );
		sched_ctx->EG_active_op_array       =  EG(active_op_array         );
		sched_ctx->EG_opline_ptr            =  EG(opline_ptr              );
		sched_ctx->EG_error_zval            =  EG(error_zval              );
		sched_ctx->EG_error_zval_ptr        =  EG(error_zval_ptr          );
		sched_ctx->EG_user_error_handler    =  EG(user_error_handler      );
		
		// restore EG to the task specific context (will be all-null on the first time run)
		EG(current_execute_data )           =  ctx->EG_current_execute_data;
		EG(argument_stack       )           =  ctx->EG_argument_stack      ;
		EG(scope                )           =  ctx->EG_scope               ;
		EG(This                 )           =  ctx->EG_This                ;
		EG(called_scope         )           =  ctx->EG_called_scope        ;
		EG(active_symbol_table  )           =  ctx->EG_active_symbol_table ;
		EG(return_value_ptr_ptr )           =  ctx->EG_return_value_ptr_ptr;
		EG(active_op_array      )           =  ctx->EG_active_op_array     ;
		EG(opline_ptr           )           =  ctx->EG_opline_ptr          ;
		EG(error_zval           )           =  ctx->EG_error_zval          ;
		EG(error_zval_ptr       )           =  ctx->EG_error_zval_ptr      ;
		EG(user_error_handler   )           =  ctx->EG_user_error_handler  ;
		
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

		PhpgoSchedulerContext* sched_ctx    =  &scheduler_ctx;
		
	#ifdef ZTS
		//assert(sched_ctx->TSRMLS_C);
		TSRMLS_FETCH_FROM_CTX(sched_ctx->TSRMLS_C); //	void ***tsrm_ls = (void ***)sched_ctx->tsrm_ls
	#endif
		
		PhpgoContext* ctx = (PhpgoContext*)TaskLocalStorage::GetSpecific(phpgo_context_key);
		
		// save the go routine's EG in it's task specific context
		ctx->EG_current_execute_data  =  EG(current_execute_data    );
		ctx->EG_argument_stack        =  EG(argument_stack          );
		ctx->EG_scope                 =  EG(scope                   );
		ctx->EG_This                  =  EG(This                    );
		ctx->EG_called_scope          =  EG(called_scope            );
		ctx->EG_active_symbol_table   =  EG(active_symbol_table     );
		ctx->EG_return_value_ptr_ptr  =  EG(return_value_ptr_ptr    );
		ctx->EG_active_op_array       =  EG(active_op_array         );
		ctx->EG_opline_ptr            =  EG(opline_ptr              );
		ctx->EG_error_zval            =  EG(error_zval              );
		ctx->EG_error_zval_ptr        =  EG(error_zval_ptr          );
		ctx->EG_user_error_handler    =  EG(user_error_handler      );
		
		// restore EG to the scheduler EGs
		EG(current_execute_data )     =  sched_ctx->EG_current_execute_data;
		EG(argument_stack       )     =  sched_ctx->EG_argument_stack      ;
		EG(scope                )     =  sched_ctx->EG_scope               ;
		EG(This                 )     =  sched_ctx->EG_This                ;
		EG(called_scope         )     =  sched_ctx->EG_called_scope        ;
		EG(active_symbol_table  )     =  sched_ctx->EG_active_symbol_table ;
		EG(return_value_ptr_ptr )     =  sched_ctx->EG_return_value_ptr_ptr;
		EG(active_op_array      )     =  sched_ctx->EG_active_op_array     ;
		EG(opline_ptr           )     =  sched_ctx->EG_opline_ptr          ;
		EG(error_zval           )     =  sched_ctx->EG_error_zval          ;
		EG(error_zval_ptr       )     =  sched_ctx->EG_error_zval_ptr      ;
		EG(user_error_handler   )     =  sched_ctx->EG_user_error_handler  ;
		
		//printf("---------->onSwapOut(%ld) returns<-----------\n", task_id);
	}
};

