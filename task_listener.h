#pragma once
#include <libgo/freeable.h>
#include <libgo/task_local_storage.h>

using namespace co;

#ifdef ZTS
	#define TSRMLS_FIELD TSRMLS_D
#else
	#define TSRMLS_FIELD
#endif

#ifdef ZTS
    /*save_to_ctx->tsrm_ls = tsrm_ls*/
	#define PHPGO_SAVE_TSRMLS(save_to_ctx) save_to_ctx->TSRMLS_C = TSRMLS_C
	
	/*void*** tsrm_ls = load_from_ctx->trsm_ls*/
	#define PHPGO_LOAD_TSRMLS(load_from_ctx) TSRMLS_C = load_from_ctx->TSRMLS_C
#else
	#define PHPGO_SAVE_TSRMLS(save_to_ctx)
	#define PHPGO_LOAD_TSRMLS(load_from_ctx)
#endif

/*
* save the current running context to "save_to_ctx"
* Note: for ZTS: the save_to_ctx->tsrm_ls must already correctly set before 
* calling the PHPGO_SAVE_CONTEXT()
*/
#define PHPGO_SAVE_CONTEXT(save_to_ctx)                                    \
{                                                                          \
    TSRMLS_FIELD;                                                          \
	/* save the current EG  */                                             \
	PHPGO_LOAD_TSRMLS(save_to_ctx);                                        \
	GET_HTTP_GLOBAL("_GET",    save_to_ctx->PG_http_globals, TRACK_VARS_GET);        \
	GET_HTTP_GLOBAL("_POST",   save_to_ctx->PG_http_globals, TRACK_VARS_POST);       \
	GET_HTTP_GLOBAL("_COOKIE", save_to_ctx->PG_http_globals, TRACK_VARS_COOKIE);     \
	GET_HTTP_GLOBAL("_SERVER", save_to_ctx->PG_http_globals, TRACK_VARS_SERVER);     \
	GET_HTTP_GLOBAL("_ENV",    save_to_ctx->PG_http_globals, TRACK_VARS_ENV);        \
	GET_HTTP_GLOBAL("_FILES",  save_to_ctx->PG_http_globals, TRACK_VARS_FILES);      \
	GET_HTTP_REQUEST_GLOBAL(   save_to_ctx->http_request_global); /*get $_REQUEST*/  \

	save_to_ctx->EG_current_execute_data  =  EG(current_execute_data    ); \
	save_to_ctx->EG_argument_stack        =  EG(argument_stack          ); \
	save_to_ctx->EG_scope                 =  EG(scope                   ); \
	save_to_ctx->EG_This                  =  EG(This                    ); \
	save_to_ctx->EG_called_scope          =  EG(called_scope            ); \
	save_to_ctx->EG_active_symbol_table   =  EG(active_symbol_table     ); \
	save_to_ctx->EG_return_value_ptr_ptr  =  EG(return_value_ptr_ptr    ); \
	save_to_ctx->EG_active_op_array       =  EG(active_op_array         ); \
	save_to_ctx->EG_opline_ptr            =  EG(opline_ptr              ); \
	save_to_ctx->EG_error_zval            =  EG(error_zval              ); \
	save_to_ctx->EG_error_zval_ptr        =  EG(error_zval_ptr          ); \
	save_to_ctx->EG_user_error_handler    =  EG(user_error_handler      ); \
}                                                                          \

/*                                                                                   
* load the "load_from_ctx" into running environment                                  
* Note: for ZTS: load_from_ctx->trsm_ls must already correctly set before            
* calling the PHPGO_LOAD_CONTEXT()                                                   
*/ 
#define PHPGO_LOAD_CONTEXT(load_from_ctx)                                  \
{                                                                          \
	TSRMLS_FIELD;                                                          \
	/* load EG from the task specific context*/                            \
	PHPGO_LOAD_TSRMLS(load_from_ctx);                                      \
	EG(current_execute_data )   =  load_from_ctx->EG_current_execute_data; \
	EG(argument_stack       )   =  load_from_ctx->EG_argument_stack      ; \
	EG(scope                )   =  load_from_ctx->EG_scope               ; \
	EG(This                 )   =  load_from_ctx->EG_This                ; \
	EG(called_scope         )   =  load_from_ctx->EG_called_scope        ; \
	EG(active_symbol_table  )   =  load_from_ctx->EG_active_symbol_table ; \
	EG(return_value_ptr_ptr )   =  load_from_ctx->EG_return_value_ptr_ptr; \
	EG(active_op_array      )   =  load_from_ctx->EG_active_op_array     ; \
	EG(opline_ptr           )   =  load_from_ctx->EG_opline_ptr          ; \
	EG(error_zval           )   =  load_from_ctx->EG_error_zval          ; \
	EG(error_zval_ptr       )   =  load_from_ctx->EG_error_zval_ptr      ; \
	EG(user_error_handler   )   =  load_from_ctx->EG_user_error_handler  ; \
	memcpy( PG(http_globals), load_from_ctx->PG_http_globals, sizeof(PG(http_globals)) ); \
	                                                                                      \
	SET_HTTP_GLOBAL("_GET",   load_from_ctx->PG_http_globals, TRACK_VARS_GET);            \
	SET_HTTP_GLOBAL("_POST",  load_from_ctx->PG_http_globals, TRACK_VARS_POST);           \
	SET_HTTP_GLOBAL("_COOKIE",load_from_ctx->PG_http_globals, TRACK_VARS_COOKIE);         \
	SET_HTTP_GLOBAL("_SERVER",load_from_ctx->PG_http_globals, TRACK_VARS_SERVER);         \
	SET_HTTP_GLOBAL("_ENV",   load_from_ctx->PG_http_globals, TRACK_VARS_ENV);            \
	SET_HTTP_GLOBAL("_FILES", load_from_ctx->PG_http_globals, TRACK_VARS_FILES);          \
	SET_HTTP_REQUEST_GLOBAL(  load_from_ctx->http_request_global); /*set $_REQUEST*/      \
}                                                                                         \

/*null-out our concerned globals to avoid potential problem*/
#define PHPGO_INITIALIZE_RUNNING_ENVIRONMENT()                     \
{                                                                  \
	EG(current_execute_data )   =  NULL;                           \
	EG(argument_stack       )   =  NULL;                           \
	EG(scope                )   =  NULL;                           \
	EG(This                 )   =  NULL;                           \
	EG(called_scope         )   =  NULL;                           \
	EG(active_symbol_table  )   =  NULL;                           \
	EG(return_value_ptr_ptr )   =  NULL;                           \
	EG(active_op_array      )   =  NULL;                           \
	EG(opline_ptr           )   =  NULL;                           \
	INIT_ZVAL(EG(error_zval));                                     \
	EG(error_zval_ptr       )   =  NULL;                           \
	EG(user_error_handler   )   =  NULL;                           \
	/*do not do the following as we inherit globals from parent*/  \
	/*memset(PG(http_globals), 0, sizeof(PG(http_globals)) );*/    \
}

#define GET_HTTP_GLOBAL(name, http_globals, offset) \
do{ \
	zval** ppz_arr = nullptr; \
	zend_hash_find(&EG(symbol_table), name, sizeof(name), (void**)&ppz_arr); \
	if(ppz_arr) { \
		http_globals[offset] = *ppz_arr; \
		Z_ADDREF_P(*ppz_arr); \
	}else{ \
		http_globals[offset] = nullptr; \
	} \
}while(0)

#define GET_HTTP_REQUEST_GLOBAL(http_request_global) \
do{ \
	zval** ppz_arr = nullptr; \
	zend_hash_find(&EG(symbol_table), "_REQUEST", sizeof("_REQUEST"), (void**)&ppz_arr); \
	if(ppz_arr){ \
		http_request_global = *ppz_arr; \
		Z_ADDREF_P(*ppz_arr); \
	}else{ \
		http_request_global = nullptr; \
	} \
}while(0)

#define SET_HTTP_GLOBAL(name, http_globals, offset) \
do{ \
	if( http_globals[offset] ) {\
		zend_hash_update(&EG(symbol_table), name, sizeof(name), http_globals[offset], sizeof(zval *), NULL); \
	} \
}while(0)

#define SET_HTTP_REQUEST_GLOBAL(http_request_global) \
do{ \
	if( http_request_global ) {\
		zend_hash_update(&EG(symbol_table), "_REQUEST", sizeof("_REQUEST"), http_request_global, sizeof(zval *), NULL); \
	} \
}while(0)

struct PhpgoBaseContext{
	TSRMLS_FIELD;     /*ZTS: void ***tsrm_ls;*/         
	struct _zend_execute_data* EG_current_execute_data; 
	zend_vm_stack 			   EG_argument_stack;       
	zend_class_entry*		   EG_scope;                
	zval*					   EG_This;                 
	zend_class_entry*		   EG_called_scope;         
	HashTable*				   EG_active_symbol_table;  
	zval**					   EG_return_value_ptr_ptr; 
	zend_op_array*			   EG_active_op_array;      
	zend_op**				   EG_opline_ptr;           
	zval                       EG_error_zval;           
	zval*                      EG_error_zval_ptr;       
	zval*                      EG_user_error_handler;   
	zval*                      PG_http_globals[NUM_TRACK_VARS];
	zval*                      http_request_global;
};

struct PhpgoContext : public PhpgoBaseContext, public FreeableImpl{
public:
	PhpgoContext(TSRMLS_D){
		TSRMLS_SET_CTX(this->TSRMLS_C);
	}
};

// the Scheduler Context is essentially the same as the Task's context,
// but since Scheduler Context  is thread local and thread locals cannot
// have virtual members, we have to remove the FreeableImpl from the
// Scheduler Context...
struct PhpgoSchedulerContext : public PhpgoBaseContext{
public:
	PhpgoSchedulerContext(){
		TSRMLS_FETCH();                     // void ***tsrm_ls = (void ***) ts_resource_ex(0, NULL)
		TSRMLS_SET_CTX(this->TSRMLS_C);     // this->tsrm_ls = (void ***) tsrm_ls
	}
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

		PhpgoSchedulerContext* sched_ctx = &scheduler_ctx; /*scheduler_ctx is thread local*/
		PhpgoContext* ctx = (PhpgoContext*)TaskLocalStorage::GetSpecific(phpgo_context_key);

		// first time swap into a task:
		// return - not to do the PHPGO_SAVE/LOAD_CONTEXT since they will be done in the
		// phpgo_go()
		if(!ctx) return;

		// running -> sched_ctx and ctx -> running
		PHPGO_SAVE_CONTEXT(sched_ctx);
		PHPGO_LOAD_CONTEXT(ctx);

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
		PHPGO_SAVE_CONTEXT(ctx);
		PHPGO_LOAD_CONTEXT(sched_ctx);
		
		//printf("---------->onSwapOut(%ld) returns<-----------\n", task_id);
	}
};

