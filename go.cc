#include "stdinc.h"
#include "php_phpgo.h"
#include "go.h"

#include <libgo/coroutine.h>
#include <libgo/task.h>
#include <libgo/freeable.h>
#include <libgo/task_local_storage.h>
#include <iostream>
//#ifdef ZTS
#include <boost/thread.hpp>
//#endif
#include "task_listener.h"

using namespace std;
using namespace co;


ZEND_EXTERN_MODULE_GLOBALS(phpgo)

// the g_Scheduler is made thread local, so the task_listener must be thread local
static thread_local PhpgoTaskListener task_listener(g_Scheduler.GetTaskListener());

bool phpgo_initialize(){
	TSRMLS_FETCH();
	
	if( !PHPGO_G(phpgo_initialized) ){
		//PhpgoTaskListener* listener = new PhpgoTaskListener( co_sched.GetTaskListener() );
		//note: the co_sched is thread local
		if( co_sched.GetTaskListener() != &task_listener )
			co_sched.SetTaskListener( &task_listener );
		
		co_sched.GetOptions().enable_work_steal = false;
		
		PHPGO_G(phpgo_initialized) = true;
	}
	return true;
}

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

/*currently only single thread is working...
  to support multi-thread (i.e, to work with pthreads), at least the following will
  need to be done: 
   1. create clone of the args in the new thread context, including zval, the object, 
      the closure ... everything passed as argument
   2. allocate zend vm stack in the new thread context since the allocation is always
      from thread local 
   3. ...
*/
void* phpgo_go(uint64_t go_routine_options, uint32_t stack_size, zend_uint argc, zval ***args TSRMLS_DC){
	
	#define GR_VM_STACK_PAGE_SIZE (256)   // 256 zval* = 2048 byte
	#define VM_STACK_PUSH(stack, v)  do { *((stack)->top++) = (void*)(v); } while(0)
	#define VM_STACK_NUM_ARGS(stack) ( (unsigned long)(*(EG(argument_stack)->top - 1)) )
	
	//allocate a new stack for the go routine
	zend_vm_stack go_routine_vm_stack = zend_vm_stack_new_page( argc > GR_VM_STACK_PAGE_SIZE ? argc : GR_VM_STACK_PAGE_SIZE );
	if(!go_routine_vm_stack) return nullptr;
	
	//set a stack start magic 0xdeaddeaddeaddead in stack bottom
	VM_STACK_PUSH(go_routine_vm_stack, 0xdeaddeaddeaddead);
	
	//store the arguments and arg count into the go routine's own stack
	//so that the go routine can then retrieve via zend_get_parameters_array_ex()
	for(zend_uint i = 0; i < argc; i++ ){
		zval_add_ref(args[i]);
		VM_STACK_PUSH(go_routine_vm_stack, *args[i]);
	}
	VM_STACK_PUSH(go_routine_vm_stack, (unsigned long)argc);
	
	/*Make a temporary context and save the current running environment into it
	and then pass this context to the new creating go routine, so that the http
	globals can be inherited */
	PhpgoContext* parent_ctx = new PhpgoContext(GoRoutineOptions::gro_default TSRMLS_CC);
	if(!parent_ctx){
		efree(go_routine_vm_stack);
		return nullptr;
	}
	bool include_http_globals = go_routine_options & GoRoutineOptions::gro_isolate_http_globals;
	parent_ctx->SwapOut(include_http_globals);
	
	if( !stack_size ) {
		stack_size = 1024 * 1024;  /*default to 1M*/
	}
	else if( stack_size < 32*1024 ){
		stack_size = 32*1024;      /*at least 32K stack, if size provided*/
	}
		
	auto tk = go_stack(stack_size) [go_routine_vm_stack, go_routine_options, parent_ctx TSRMLS_CC] ()mutable {
		defer{
			delete parent_ctx;
			
			if(!EG(argument_stack)){
				//zend_vm_stack_destroy requires EG(argument_stack) be set
				EG(argument_stack) = go_routine_vm_stack; 
			}
			zend_vm_stack_destroy(TSRMLS_C);
		};

		// set the tsrm_ls to my prarent, i.e., inherit all globals from my parent
		PhpgoContext* ctx = new PhpgoContext(go_routine_options TSRMLS_CC); 
		if( !ctx ) return;
		
		kls_key_t phpgo_context_key = TaskLocalStorage::CreateKey("PhpgoContext");
		if( !phpgo_context_key ) {
			delete ctx;
			return;
		}
		
		if( !TaskLocalStorage::SetSpecific(phpgo_context_key, ctx) ) {
			delete ctx;
			return;
		}

		// we are just brought up by the scheduler, and we are now running under the
		// scheduler's context. Before we are going to run, we need to save the running
		// environemt into the scheduler's and load our own (which is inherited from
		// our parent)
		//
		// get the scheduler context form scheduler_ctx (thread local)
		// and save the current running environment to it
		bool include_http_globals = ctx->go_routine_options & GoRoutineOptions::gro_isolate_http_globals;
		PhpgoSchedulerContext* sched_ctx = &scheduler_ctx;
		sched_ctx->SwapOut(include_http_globals);

		// load the running environment from the parent_ctx (inherited from parent),
		// and then clear the fileds that we don't want:
		parent_ctx->SwapIn(include_http_globals);  
		PHPGO_INITIALIZE_RUNNING_ENVIRONMENT();
		
		//set the argument stack to the dedicate stack
		EG(argument_stack) = go_routine_vm_stack;
		
		char *func_name                  = NULL; 
		char *error                      = NULL;
		zend_fcall_info_cache *fci_cache = NULL;
		zval* return_value               = NULL;
		zend_uint argc                   = VM_STACK_NUM_ARGS();
		zval*** args                     = NULL;
		
		defer{
			if(func_name) 	 efree(func_name);
			if(error)        efree(error);
			if(fci_cache)    efree(fci_cache);
			if(return_value) zval_ptr_dtor(&return_value);
			
			if(args){
				//we've add-ref'ed to the callback & parameters outside this go routine, so
				//after complete using the callback, decrease the reference to them
				for(zend_uint i = 0; i < argc; i++ ){
					zval_ptr_dtor(args[i]);
				}
				efree(args);
			}
		};

		args = (zval ***)safe_emalloc(argc, sizeof(zval **), 0);
		if(!args) return;
		
		if (zend_get_parameters_array_ex(argc, args) == FAILURE) {
			zend_error(E_ERROR, "phpgo: getting go routine parameters failed");
			return;
		}
		
		/* go (callable $callback, callback_arg1, callback_arg2...), then
		/* args[0] == $callback, args[1..n] = callback_arg1, ...callback_argn*/
		
		fci_cache = (zend_fcall_info_cache*)emalloc(sizeof(zend_fcall_info_cache));
		if(!fci_cache) return;
		
		if(!zend_is_callable_ex(*args[0], NULL, IS_CALLABLE_CHECK_SILENT, &func_name, NULL, fci_cache, &error TSRMLS_CC)) {
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
		
		MAKE_STD_ZVAL(return_value);
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
	
	return tk;
}

/*
 * go_debug : set the libgo debug flag
*/
void phpgo_go_debug(unsigned long debug_flag){
	co_sched.GetOptions().debug = debug_flag;
}
	
