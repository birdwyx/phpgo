#include <libgo/coroutine.h>
#include <libgo/task.h>
#include <libgo/freeable.h>
#include <libgo/task_local_storage.h>
#include <iostream>
#include <unistd.h>
#include <atomic>
#include <boost/thread.hpp>

using namespace std;
using namespace co;

#include "stdinc.h"
#include "go.h"
#include "mutex.h"
#include "wait_group.h"
#include "defer.h"

extern zend_class_entry* ce_go_chan_ptr;

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
		//printf("---------->onSwapIn(%ld)<-----------\n", task_id);'
		
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

bool phpgo_initialize(){
	PhpgoTaskListener* listener = new PhpgoTaskListener( co_sched.GetTaskListener() );
	co_sched.SetTaskListener(listener);
	co_sched.GetOptions().enable_work_steal = false;
	return true;
}


/*
unsigned int sleep(unsigned int seconds){
	typedef unsigned int(*sleep_t)(unsigned int seconds);
	sleep_f = (sleep_t)dlsym(RTLD_NEXT, "sleep");
	
	php_printf("sleep_f: %p, sleep: %p\n", sleep_f, sleep);
	//if(sleep_f){
	//	sleep_f(seconds);
	//}
	
	int timeout_ms = seconds * 1000;
    g_Scheduler.SleepSwitch(timeout_ms);
}*/

void dump(void* buff, size_t n){
	int i = 0 ; 
	unsigned char* b = (unsigned char*)buff;
	while( i< (int)n ){
		if(i%16 == 0 && i > 0) {
			int j = i - 16;
			if(j >= 0){
				while(j < i){
					unsigned char c =  isprint(b[j])? b[j] : '.';
					php_printf("%c", c);
					j++;
				}
			}
			php_printf("\n");
		}
		php_printf("%02x ", b[i]);
		i++;
	}
	php_printf("\n");
}

void dump_zval(zval* zv){
	php_printf("zval %p------>\n", zv);
	php_printf("type: %d\n", zv->type);
	php_printf("refcount__gc: %d\n", zv->refcount__gc);
	php_printf("is_ref__gc: %d\n", zv->is_ref__gc);
	php_printf("value: \n");
	dump(&(zv->value), sizeof(zv->value));
	php_printf("handle: %x\n", zv->value.obj.handle);
	php_printf("handlers: %x\n", zv->value.obj.handlers);
	php_printf("<------\n");
	//zv->refcount__gc = 3;
}

static void copy_closure_static_var(zval **var TSRMLS_DC, int num_args, va_list args, zend_hash_key *key) /* {{{ */
{
	HashTable *target = va_arg(args, HashTable *);

	SEPARATE_ZVAL_TO_MAKE_IS_REF(var);
	Z_ADDREF_PP(var);
	zend_hash_quick_update(target, key->arKey, key->nKeyLength, key->h, var, sizeof(zval *), NULL);
}

void* phpgo_go(zend_uint argc, zval ***args TSRMLS_DC){
	
	#define GR_VM_STACK_PAGE_SIZE (256)   // 256 zval* = 2048 byte
	#define VM_STACK_PUSH(stack, v)  do { *((stack)->top++) = (void*)(v); } while(0)
	#define VM_STACK_NUM_ARGS(stack) ( (unsigned long)(*(EG(argument_stack)->top - 1)) )
	
	// allocate a new stack for the go routine
	zend_vm_stack go_routine_vm_stack = zend_vm_stack_new_page( argc > GR_VM_STACK_PAGE_SIZE ? argc : GR_VM_STACK_PAGE_SIZE );
	VM_STACK_PUSH(go_routine_vm_stack, NULL);
	
	/*
	char *func_name                  = NULL; 
	char *error                      = NULL;
	zend_fcall_info_cache *fci_cache = (zend_fcall_info_cache*)emalloc(sizeof(zend_fcall_info_cache));
	
	if(!fci_cache){
		efree(go_routine_vm_stack);
		return nullptr;
	}
	
	defer{
		if(func_name) efree(func_name);
		if(error) efree(error);
		efree(fci_cache);
	};
		
	if (!zend_is_callable_ex(*args[0], NULL, IS_CALLABLE_CHECK_SILENT, &func_name, NULL, fci_cache, &error TSRMLS_CC)) {
		if (error) {
			zend_error(E_WARNING, "phpgo: invalid callback %s, %s", func_name, error);
		}
		return nullptr;
	}
	
	auto op_array = &fci_cache->function_handler->op_array;
	(op_array->refcount)++;
	
	*/
	
	/* Create a clone of closure, because it may be destroyed */
	/*
	if (op_array->fn_flags & ZEND_ACC_CLOSURE) {
		zend_op_array *op_array_copy = (zend_op_array*)emalloc(sizeof(zend_op_array));
		*op_array_copy = *op_array;

		(*op_array->refcount)++;
		op_array->run_time_cache = NULL;
		if (op_array->static_variables) {
			ALLOC_HASHTABLE(op_array_copy->static_variables);
			zend_hash_init(
				op_array_copy->static_variables, 
				zend_hash_num_elements(op_array->static_variables),
				NULL, ZVAL_PTR_DTOR, 0
			);
			zend_hash_apply_with_arguments(
				op_array->static_variables TSRMLS_CC,
				(apply_func_args_t) copy_closure_static_var,
				1, op_array_copy->static_variables
			);
		}

		op_array = op_array_copy;
	}*/
	
	//store the parameters into the go routine's own stack
	//so that the go routine can then retrieve via zend_get_parameters_array_ex()
	for(zend_uint i = 0; i < argc; i++ ){
		zval_add_ref(args[i]);
		VM_STACK_PUSH(go_routine_vm_stack, *args[i]);
	}
	VM_STACK_PUSH(go_routine_vm_stack, (unsigned long)argc);

	co::Task* tk = go_stack(32*1024) [go_routine_vm_stack] {
		// the thread to run this go routine may not be the same as the thread calling the
		// phpgo_go(), thus we need to fetch explictly the TSRMLS for current thread
		// 
	#ifdef ZTS
		TSRMLS_FETCH_FROM_CTX(scheduler_ctx.TSRMLS_C); //void ***tsrm_ls = (void ***)scheduler_ctx.tsrm_ls
	#endif
		
		EG(current_execute_data) = NULL;
		
		//set the argument stack to the dedicate stack
		EG(argument_stack) = go_routine_vm_stack;
		
		char *func_name                  = NULL; 
		char *error                      = NULL;
		zend_fcall_info_cache *fci_cache = (zend_fcall_info_cache*)emalloc(sizeof(zend_fcall_info_cache));
		zend_uint argc                   = VM_STACK_NUM_ARGS();
		zval*** args                     = (zval ***)safe_emalloc(argc, sizeof(zval **), 0);
		
		if(!fci_cache) return;
		if(!args) return;
		
		zval* return_value = NULL;
		MAKE_STD_ZVAL(return_value);
		
		defer{
			if(func_name) 	efree(func_name);
			if(error)       efree(error);
			
			efree(fci_cache);
			
			//we've add-ref'ed to the callback & parameters outside this go routine, so
			//after complete using the callback, decrease the reference to them
			for(zend_uint i = 0; i < argc; i++ ){
				zval_ptr_dtor(args[i]);
			}
			efree(args);
			
			if(return_value) zval_ptr_dtor(&return_value);
			
			//free the vm stack since this go routine is going to destroy
			zend_vm_stack_destroy(TSRMLS_C);
		};
		
		if (zend_get_parameters_array_ex(argc, args) == FAILURE) {
			zend_error(E_ERROR, "phpgo: getting go routine parameters faild");
			return;
		}
		
		/* go (callable $callback, callback_arg1, callback_arg2...), then
		/* args[0] == $callback, args[1..n] = callback_arg1, ...callback_argn*/

		if (!zend_is_callable_ex(*args[0], NULL, IS_CALLABLE_CHECK_SILENT, &func_name, NULL, fci_cache, &error TSRMLS_CC)) {
			if (error) {
				zend_error(E_WARNING, "phpgo: invalid callback %s, %s", func_name, error);
			}
			return;
		}
		
		// if any arg of the callback should be sent by ref, if yes, set the arg to pass-by-ref
		for (auto i = 1; i < argc; i++) {
			if (ARG_SHOULD_BE_SENT_BY_REF(fci_cache->function_handler, i)) {
				Z_SET_ISREF_PP(args[i]);
			}
		}
		
		if( call_user_function_ex(
			EG(function_table), 
			NULL, 
			*args[0],                   // the callback callable
			&return_value,              // zval** to receive return value
			argc - 1,                   // the parameter number required by the callback
			argc > 1 ? args + 1 : NULL, // the parameter list of the callback
			1, 
			NULL TSRMLS_CC
		) != SUCCESS) {
			zend_error(E_ERROR, "phpgo: execution of go routine faild");
			return;
		}
	};
	
	//tk->SetSwapHooks(swap_in_hook, swap_out_hook, swap_in_error_hook, swap_out_error_hook);
	
	return tk;
}

void phpgo_go_scheduler_run_once(){
	if (co_sched.IsCoroutine()) {
		zend_error(E_ERROR, "phpgo: error: go_schedule_once must be called outside a go routine\n");
		return;
	}
	co_sched.Run();
}

void phpgo_go_scheduler_run_join_all(){
	if (co_sched.IsCoroutine()) {
		zend_error(E_ERROR, "phpgo: error: go_schedule_all must be called outside a go routine\n");
		return;
	}	
	co_sched.RunUntilNoTask();
}

void phpgo_go_scheduler_run_forever(){
	if (co_sched.IsCoroutine()) {
		zend_error(E_ERROR, "phpgo: error: go_schedule_forever must be called outside a go routine\n");
		return;
	}
	co_sched.RunLoop();
}

#ifdef ZTS
void phpgo_go_scheduler_run_forever_multi_threaded(uint64_t threads){
	if (co_sched.IsCoroutine()) {
		zend_error(E_ERROR, "phpgo: error: go_schedule_forever_multi_threaded must be called outside a go routine\n");
		return;
	}
	
	boost::thread_group tg;
    for (int i = 0; i < threads; ++i){
        tg.create_thread([] {
			for(;;){
				co_sched.Run();
			}
        });
	}
}
#endif

void phpgo_go_debug(unsigned long debug_flag){
	co_sched.GetOptions().debug = debug_flag;
}


void* phpgo_go_chan_create(unsigned long capacity){
	
	CHANNEL_INFO* chinfo = new CHANNEL_INFO;
	chinfo->closed = false;
	chinfo->chan = new co_chan<zval*>(capacity);	
	return chinfo;
}

void  phpgo_go_chan_destroy(void* handle){
	
	//assert(handle);
	
	CHANNEL_INFO* chinfo = (CHANNEL_INFO*)handle;
	
	if(chinfo->chan){
		delete (Channel<zval*>*)(chinfo->chan);
	}
		
	delete chinfo;
}

void  phpgo_go_chan_close(void* handle){
	
	//assert(handle);
	
	CHANNEL_INFO* chinfo = (CHANNEL_INFO*)handle;
	chinfo->closed = true;
}

void* phpgo_go_chan_push(void* handle, zval* z){
	
	CHANNEL_INFO* chinfo = (CHANNEL_INFO*)handle;
	
	// cannot write to a closed channel
	if(chinfo->closed)
		return handle;
	
	zval_add_ref(&z);
	*( (co_chan<zval*>*)chinfo->chan ) << z;
	
	return handle;
}

zval* phpgo_go_chan_pop(void* handle){
	
	zval* z = nullptr;
	
	CHANNEL_INFO* chinfo = (CHANNEL_INFO*)handle;
	
	// to conform to the google go channel:
	// if a chan is closed, return immediately
	// this will allow a master routine to close a channel which is read by multiple 
	// slave routines, thus allow the master to broadcast a signal (typically treated 
	// as close signal) to all slave routines
	if(chinfo->closed){
		MAKE_STD_ZVAL(z);
		ZVAL_NULL(z);
		return z;
	}
	
	co_chan<zval*>* ch = (co_chan<zval*>*)chinfo->chan;
	
	// if it's not in any go routine,
	// run the scheduler (so that the go routines will run and may
	// push to the channel) until successfully read from the channel 
	if (!co_sched.IsCoroutine()) {
		while( !ch->TryPop(z) ) {
			co_sched.Run();
		}
		return z;
	}

	// in a go routine, it's safe for a blocking read
	*ch >> z;
	
	return z;
}

/*convert chan zval to the channel ptr*/
Channel<zval*>* _phpgo_go_chan_z2p(zval* z_chan TSRMLS_DC){
	
	auto z_handler = zend_read_property(ce_go_chan_ptr, z_chan, "handle",   sizeof("handle"), true TSRMLS_CC);
	if( !z_handler || Z_TYPE_P(z_handler) == IS_NULL ){
		return nullptr;
	}
	
	auto chinfo = (CHANNEL_INFO*) Z_LVAL_P(z_handler);
	if(!chinfo){
		return nullptr;
	}
	
	return (Channel<zval*>*)chinfo->chan;
}

/*convert chan zval to the channel CHANNEL_INFO*/
CHANNEL_INFO* _phpgo_go_chan_z2i(zval* z_chan TSRMLS_DC){
	
	auto z_handler = zend_read_property(ce_go_chan_ptr, z_chan, "handle",   sizeof("handle"), true TSRMLS_CC);
	if( !z_handler || Z_TYPE_P(z_handler) == IS_NULL ){
		return nullptr;
	}
	
	auto chinfo = (CHANNEL_INFO*) Z_LVAL_P(z_handler);
	
	return chinfo;
}

bool phpgo_go_chan_try_push(void* handle, zval* z){
	
	CHANNEL_INFO* chinfo = (CHANNEL_INFO*)handle;
	
	// cannot write to a closed channel
	if(chinfo->closed)
		return false;
	
	auto ch = (co_chan<zval*>*)chinfo->chan;
	if( ch->TryPush(z) ){
		zval_add_ref(&z);
		return true;
	}
	
	return false;
}

zval* phpgo_go_chan_try_pop(void* handle){
	
	zval* z = nullptr;
	CHANNEL_INFO* chinfo = (CHANNEL_INFO*)handle;
	
	if(chinfo->closed){
		// return ZVAL_NULL means channel close
		MAKE_STD_ZVAL(z);
		ZVAL_NULL(z);
		return z;
	}
	
	auto ch = (co_chan<zval*>*)chinfo->chan;
	
	// return nullptr means channel is not ready to read
	return ch->TryPop(z) ? z : nullptr;
}
	
/*
 * mutex
 */
 
void* phpgo_go_mutex_create(bool signaled){
	return new ::co::CoRecursiveMutex(signaled);
}

void phpgo_go_mutex_lock(void* mutex){
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

void phpgo_go_mutex_unlock(void* mutex){
	auto mutex_obj = (::co::CoRecursiveMutex*)mutex;
	mutex_obj->unlock();
}

bool phpgo_go_mutex_try_lock(void* mutex){
	auto mutex_obj = (::co::CoRecursiveMutex*)mutex;
	return mutex_obj->try_lock();
}

bool phpgo_go_mutex_is_lock(void* mutex){
	auto mutex_obj = (::co::CoRecursiveMutex*)mutex;
	return mutex_obj->is_lock();
}

void phpgo_go_mutex_destroy(void* mutex){
	auto mutex_obj = (::co::CoRecursiveMutex*)mutex;
	delete mutex_obj;
}

/*
* waitgroup
*/
void* phpgo_go_wait_group_create(){
	return new ::co::CoWaitGroup();
}

int64_t  phpgo_go_wait_group_add(void* wg, int64_t delta){
	auto wg_obj = (::co::CoWaitGroup*)wg;
	return wg_obj->Add(delta);
}

int64_t  phpgo_go_wait_group_done(void* wg){
	auto wg_obj = (::co::CoWaitGroup*)wg;
	return wg_obj->Done();
}

int64_t  phpgo_go_wait_group_count(void* wg){
	auto wg_obj = (::co::CoWaitGroup*)wg;
	return wg_obj->Count();
}

void  phpgo_go_wait_group_wait(void* wg){
	auto wg_obj = (::co::CoWaitGroup*)wg;
	wg_obj->Wait();
}

bool  phpgo_go_wait_group_destruct(void* wg){
	auto wg_obj = (::co::CoWaitGroup*)wg;
	delete wg_obj;
}

uint64_t phpgo_go_runtime_num_goroutine(){
	return co_sched.TaskCount();
}

void phpgo_go_runtime_gosched(){
	if(co_sched.IsCoroutine()){
		co_sched.CoYield();
	}else{
		co_sched.Run();
	}
}
	
/*
* select - case
*/
zval*  phpgo_select(GO_SELECT_CASE* case_array, long case_count TSRMLS_DC){
	
	//printf("phpgo_select\n");
	
	if(!case_count) return nullptr;
	
	srand(time(NULL));
	auto r = rand();
	auto start = r % case_count;
	
	GO_SELECT_CASE* selected_case = nullptr;
	bool any_case_ready =  false;
	
	zval* z_chan = nullptr;
	zval* z_handler = nullptr;
	co::Channel<zval*>* chanptr = nullptr;
	CHANNEL_INFO* chinfo = nullptr;
	
	auto i = start;
	do{
		switch(case_array[i].case_type){
		case GO_CASE_TYPE_CASE:
			z_chan = case_array[i].chan;
			chinfo = _phpgo_go_chan_z2i(z_chan TSRMLS_CC);
			if( !chinfo ){
				zend_error(E_ERROR, "phpgo: phpgo_select: null channel");
				return nullptr;
			}
			if(case_array[i].op == GO_CASE_OP_READ){
				
				// phpgo_go_chan_try_pop:
				// if data not ready to read, return nullptr
				// if channel is closed, return ZVAL_NULL
				// otherwise return the read zval
				zval* data = phpgo_go_chan_try_pop(chinfo);
				if(data) {
					//printf("receive: \n");
					//dump_zval(data);
					
					zval_dtor(case_array[i].value);
					INIT_PZVAL_COPY(case_array[i].value, data);
					
					//ZVAL_COPY_VALUE(case_array[i].value, data);
					//auto ref = case_array[i].value->refcount__gc;
					//*case_array[i].value = *data;
					//case_array[i].value->refcount__gc = ref;
					//zval_copy_ctor(case_array[i].value);
					//case_array[i].value->refcount__gc = 1;
					zval_ptr_dtor(&data);
					
					//printf("receive: case_array[i].value\n");
					//dump_zval(case_array[i].value);
					
					//printf("go select casetype CASE, ch %ld, op %ld, value %p, callback %p\n", 
					// case_array[i].chan, case_array[i].op, case_array[i].value, case_array[i].callback  );
					
					selected_case = &case_array[i];
					goto exit_while;
				}
			}else if(case_array[i].op == GO_CASE_OP_WRITE){
				auto ready = phpgo_go_chan_try_push(chinfo, case_array[i].value);				
				if(ready){
					zval_add_ref(&case_array[i].value);
					selected_case = &case_array[i];
					goto exit_while;
				}
				
			}else{
				//error
			}
			break;
		case GO_CASE_TYPE_DEFAULT:
			selected_case = &case_array[i];
			break;
		default:
			break;
		}  // switch(case_array[i].case_type)
		
		i = (i+1) % case_count;
	}while (i != start );
	
exit_while:
	zval*   return_value = nullptr; 
	if( selected_case ){
		//printf("call default callback\n");
		
		
		zval*** args         = nullptr;
		auto    argc         = 0;
		if(selected_case->case_type != GO_CASE_TYPE_DEFAULT){
			args    = (zval***)safe_emalloc(1, sizeof(zval **), 0);
			args[0] = &selected_case->value;
			argc    = 1;
		}
		
		//printf("phpgo_select ... about to call_user_function_ex\n");
		
		zval_add_ref(&selected_case->callback);
		if( call_user_function_ex(
			EG(function_table), 
			NULL, 
			selected_case->callback,    // the callback callable
			&return_value,              // zval** to receive return value
			argc,                       // the parameter number required by the callback
			args,                       // the parameter list of the callback
			1, 
			NULL TSRMLS_CC
		) != SUCCESS) {
			zend_error(E_ERROR, "phpgo: execution of go routine faild");
			//goto cleanup;
		}
		
		//printf("phpgo_select ... after call_user_function_ex\n");
		zval_ptr_dtor(&selected_case->callback);
		//zval_ptr_dtor(&return_value);
		
		if(args) efree(args);
	}
	
	//printf("phpgo_select ... before CoYield\n");
	
	g_Scheduler.CoYield();
	
	//printf("phpgo_select ... after CoYield\n");
	
	return return_value;
}

void phpgo_go_timer_tick(zval* z_chan, void* h_chan, uint64_t micro_seconds){
	
	// must add ref the z_chan so that it won't be release before the go routine returns
	zval_add_ref(&z_chan);
	go_stack(4*1024) [=] ()mutable {
		while(true){
			zval* z;
			MAKE_STD_ZVAL(z);
			ZVAL_LONG(z, 1);
			
			usleep(micro_seconds);
			phpgo_go_chan_push(h_chan, z);
		}
		//zval_ptr_dtor(&z_chan);
	};
}

void phpgo_go_timer_after(zval* z_chan, void* h_chan, uint64_t micro_seconds){
	
	// must add ref the z_chan so that it won't be release before the go routine returns
	zval_add_ref(&z_chan);
	go_stack(4*1024) [=] ()mutable {
		zval* z;
		MAKE_STD_ZVAL(z);
		ZVAL_LONG(z, 1);
		
		usleep(micro_seconds);
		phpgo_go_chan_push(h_chan, z);
		zval_ptr_dtor(&z_chan);
	};
}

