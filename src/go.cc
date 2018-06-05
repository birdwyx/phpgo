#include "stdinc.h"
#include "php_phpgo.h"
#include "go.h"

#include <libgo/coroutine.h>
#include <libgo/task.h>
#include "freeable.h"
#include "task_local_storage.h"
#include <iostream>
#include "task_listener.h"

using namespace std;
using namespace co;

/*
believe or not, the following code can stop a segmentation fault 
after the php execution, which can be triggered by the following 
steps:
1. pre-condition: php5/7, zts or non-zts enviroment
2. gcc version 4.8.5 ( which may not matter at all )
3. libgo and phpgo make-install'ed; phpgo extension enbaled
4. #export LD_PRELOAD=liblibgo.so (this does matter)
5. #php -r "echo 123"; (or any other php execution)
6. you'll see output: 
123Segmentation fault

with the following code in place, the segmentation fault is gone.
anyone who can explain this should get a Nobel Prize :)))
*/
class A
{
public:
	A(){
	}
};

template <class T> struct B
{
	static A const e;
};

template <class T> 
A const  B<T>::e = A();

/**it's the function below that stops the segv*/
inline  A func(){
	return B<int>::e;
}
/*<---------*/

#define PARAMETER_ARRAY_POSITION (1)

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

#if PHP_MAJOR_VERSION < 7 

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
#else
	void dump_zval(zval* zv){
		const char* val_types[] = {
			//0..10
			"IS_UNDEF",
			"IS_NULL",
			"IS_FALSE",
			"IS_TRUE",
			"IS_LONG",
			"IS_DOUBLE",
			"IS_STRING",
			"IS_ARRAY",
			"IS_OBJECT",
			"IS_RESOURCE",
			"IS_REFERENCE",

			"IS_CONSTANT",					//11
			"IS_CONSTANT_AST",				//12
			"_IS_BOOL",	                    //13
			"IS_CALLABLE",					//14
			"IS_INDIRECT",                  //15
			"",
			"IS_PTR",                       //17
			"IS_VOID",						//18
			"IS_ITERABLE",					//19
			"_IS_ERROR"					    //20
		};

		php_printf("zval %p------>\n", zv);
		php_printf("u1.v.type:                    %02x(%s)\n", zv->u1.v.type, val_types[zv->u1.v.type]);
		php_printf("u1.v.type_flags:              %02x\n", zv->u1.v.type_flags);
		//php_printf("u1.v.const_flags:             %02x\n", zv->u1.v.const_flags);
		//php_printf("u1.v.reserved:                %02x\n", zv->u1.v.reserved);
		php_printf("u1.v == u1.type_info ==:      %08x\n", zv->u1.type_info);
		php_printf("u2:                           %08x\n", zv->u2.next);
		php_printf("value:                        %016x\n", zv->value.lval);
		
		if( Z_REFCOUNTED_P(zv) ){
			printf("value.counted->\n");
			printf("    gc.refcount:              %08x\n", zv->value.counted->gc.refcount);
			printf("    gc.u.v.type:              %02x\n", zv->value.counted->gc.u.v.type);
			printf("    gc.u.v.flags:             %02x\n", zv->value.counted->gc.u.v.flags);
			printf("    gc.u.v.gc_info:           %04x\n", zv->value.counted->gc.u.v.gc_info);
			printf("    gc.u.type_info(==gc.u.v): %08x\n", zv->value.counted->gc.u.type_info);
		}
		
		switch( Z_TYPE_P(zv) ){
			case IS_STRING:
				printf("    string(%d) = %s\n", zv->value.str->len, zv->value.str->val);
				break;
			case IS_REFERENCE:
				printf("referece zval:\n");
				dump_zval( &zv->value.ref->val);
				break;
		}
	}
#endif

/*
* explode the parameter array (args[1]) and append them to the args[]
*/
bool php_go_explode_array_arg(PHPGO_ARG_TYPE* args, size_t* exploded_size){
	zval* arr    = PHPGO_ARG_TO_PZVAL( args[PARAMETER_ARRAY_POSITION] );
	size_t index = PARAMETER_ARRAY_POSITION + 1;
	
	HashTable* ht        = Z_ARRVAL_P(arr); 
	HashPosition pointer = PHPGO_INVALID_HASH_POSITION; //hash position is pointer prior to PHP7 and is a uint32_t after
	auto n               = zend_hash_num_elements(ht);
	if(!n) return false;
	
	zend_hash_internal_pointer_reset_ex(ht, &pointer); 
	do{
		void* data           = nullptr;
		if( phpgo_zend_hash_get_current_data_ex(ht, (void**)&data, &pointer) != SUCCESS){
			zend_error(E_ERROR, "phpgo: php_go_explode_array_arg(): error getting data of array index %d", index);
			return false;
		};
		
		//php5: data is a zval**; php7: data is a zval*
		//php5: args is array of zval**; php7: args is array of zval ...uhhhh...f..k
		args[index++] = PHP5_VS_7( (zval**)data, *(zval*)data );
		
		zend_hash_move_forward_ex(ht, &pointer);
	} while( pointer != PHPGO_INVALID_HASH_POSITION );
	
	*exploded_size = n;
	return true;
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

bool phpgo_go(
	uint64_t go_routine_options, 
	uint32_t stack_size, 
	zend_uint argc, 
	PHPGO_ARG_TYPE* args
	TSRMLS_DC
){
	#define GR_VM_STACK_PAGE_SIZE (256)           // 256 zval* = 2048 byte
	#define GR_DEFAULT_STACK_SIZE (1024 * 1024)   // 1M
	#define GR_MIN_STACK_SIZE     (32 * 1024)     // at least 32M  

#if PHP_MAJOR_VERSION < 7
	#define VM_STACK_PUSH(stack, v)  do { *((stack)->top++) = (void*)(v); } while(0)
	#define VM_STACK_NUM_ARGS() ( (unsigned long)(*(EG_VM_STACK->top - 1)) )
#else
	#define VM_STACK_PUSH(stack, v)  do { ZVAL_COPY_VALUE((stack)->top++, (v)); } while(0)
	#define VM_STACK_PUSH_LONG(stack, l) \
		do{ \
			zval tmp; \
			ZVAL_LONG(&tmp, l); \
			*((stack)->top++) = (tmp); \
		} while(0)
	#define VM_STACK_NUM_ARGS() ( Z_LVAL(*(EG_VM_STACK->top - 1)) )
#endif
	
	//allocate a new stack for the go routine
	zend_vm_stack go_routine_vm_stack = zend_vm_stack_new_page( argc > GR_VM_STACK_PAGE_SIZE ? argc : GR_VM_STACK_PAGE_SIZE );
	if(!go_routine_vm_stack) return false;

#if PHP_MAJOR_VERSION < 7
	//set a stack start magic 0xdeaddeaddeaddead in stack bottom
	VM_STACK_PUSH(go_routine_vm_stack, 0xdeaddeaddeaddead);
	
	//store the arguments and arg count into the go routine's own stack
	//so that the go routine can then retrieve via zend_get_parameters_array_ex()
	for(zend_uint i = 0; i < argc; i++ ){
		phpgo_zval_add_ref(args[i]);
		VM_STACK_PUSH(go_routine_vm_stack, *args[i]);
	}
	VM_STACK_PUSH(go_routine_vm_stack, (unsigned long)argc);
#else
	VM_STACK_PUSH_LONG(go_routine_vm_stack, 0xdeaddeaddeaddead);
	for(zend_uint i = 0; i < argc; i++ ){
		VM_STACK_PUSH(go_routine_vm_stack, &args[i]);
		zval_add_ref(&args[i]);
	}
	VM_STACK_PUSH_LONG(go_routine_vm_stack, (unsigned long)argc);
#endif
	
	/* setup the http globals for child to inherit*/
	PHP5_AND_BELOW(	zval* parent_http_globals[NUM_TRACK_VARS];	zval* parent_http_request_global; );
	PHP7_AND_ABOVE( zval  parent_http_globals[NUM_TRACK_VARS];	zval  parent_http_request_global; );
	MAKE_HTTP_GLOBALS(parent_http_globals, parent_http_request_global);
	
	if(go_routine_options & GoRoutineOptions::gro_isolate_http_globals) {
		GET_HTTP_GLOBALS(parent_http_globals, parent_http_request_global);
	}
	/**/
	
	/*setup stack size*/
	if( !stack_size ) {
		stack_size = GR_DEFAULT_STACK_SIZE;  /*default to 1M*/
	}else if( stack_size < GR_MIN_STACK_SIZE ){
		stack_size = GR_MIN_STACK_SIZE;      /*at least 32K stack, if size provided*/
	}
	/**/
		
	go_stack(stack_size) [
		go_routine_vm_stack,
		go_routine_options,
		parent_http_globals,
		parent_http_request_global
		TSRMLS_CC
	] ()mutable {
		defer{
			DELREF_HTTP_GLOBALS(parent_http_globals, parent_http_request_global);
			
			if(!EG_VM_STACK){
				//zend_vm_stack_destroy requires EG_VM_STACK be set
				EG_VM_STACK = go_routine_vm_stack; 
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
		
		// load the http globals: inherited from parent,
		if(include_http_globals) {
			SET_HTTP_GLOBALS(parent_http_globals, parent_http_request_global);
		}
		PHPGO_INITIALIZE_RUNNING_ENVIRONMENT();
		
		//set the vm stack to the dedicate stack
		EG_VM_STACK = go_routine_vm_stack;
		PHP7_AND_ABOVE(
			EG(vm_stack_top) = EG(vm_stack)->top;
			EG(vm_stack_end) = EG(vm_stack)->end;
		);


		FUNC_NAME_TYPE func_name         = NULL; 
		char *error                      = NULL;
		zend_fcall_info_cache *fci_cache = NULL;
		zval* return_value               = NULL;
		zend_uint argc                   = VM_STACK_NUM_ARGS();
		PHPGO_ARG_TYPE* args             = NULL;

		defer{
			if(func_name) 	 FREE_FUNC_NAME(func_name);
			if(error)        efree(error);
			if(fci_cache)    efree(fci_cache);
			if(return_value) phpgo_zval_ptr_dtor(&return_value);
			
			if(args){
#if PHP_MAJOR_VERSION < 7
				//we've add-ref'ed to the callback & parameters outside this go routine, so
				//after complete using the callback, decrease the reference to them
				for(zend_uint i = 0; i < argc; i++ ){
					phpgo_zval_ptr_dtor(args[i]);
				}
#else
				for(zend_uint i = 0; i < argc; i++ ){
					zval* z = &args[i];
					phpgo_zval_ptr_dtor(&z);  //phpgo_zval_ptr_dtor requires zval**
				}
#endif
				efree(args);
			}
		};

		args = (PHPGO_ARG_TYPE*)safe_emalloc(PHPGO_MAX_GO_ARGS, sizeof(PHPGO_ARG_TYPE), 0);
		if(!args) return;
		
		//printf("2: argc=%d, args=%p\n", argc, args);

#if PHP_MAJOR_VERSION < 7
		if (zend_get_parameters_array_ex(argc, args) == FAILURE) {
			zend_error(E_ERROR, "phpgo: getting go routine parameters failed");
			return;
		}
#else
	    //vm stack = dead, arg1, arg2, ... argn, argc
		zval* z_arg = EG_VM_STACK->top - argc -1;
		for(auto i=0; i<argc; i++){
			args[i] = *z_arg++;
		}
#endif
		auto param_count = 0;
		if(argc > 1){
			// args is now [$callable, $parameter_arr]
			// explode the $parameter_arr to $arg1,$arg2...$argn
			// and then we will invoke the callable like this: $callable($arg1,$arg2...)
			// args will look like [$callable, $arg1,$arg2,...,$argn] after this operation
			size_t exploaded_size = 0;
			if( !php_go_explode_array_arg(args, &exploaded_size) ) return;
			param_count = exploaded_size; 
		}

		fci_cache = (zend_fcall_info_cache*)emalloc(sizeof(zend_fcall_info_cache));
		if(!fci_cache) return;
		
		zval* callback = PHPGO_ARG_TO_PZVAL(args[0]);
		if(!zend_is_callable_ex(
			callback, 
			NULL, 
			IS_CALLABLE_CHECK_SILENT, 
			&func_name 
			PHP5_AND_BELOW_ONLY_CC(NULL),
			fci_cache, 
			&error TSRMLS_CC)
		){
			if (error) {
				zend_error(E_WARNING, "phpgo: invalid callback %s, %s", func_name, error);
			}
			return;
        }
		
		// if any arg of the callback should be sent by ref, if yes, set the arg to pass-by-ref
		for (auto i = PARAMETER_ARRAY_POSITION + 1; 
		          i < param_count + PARAMETER_ARRAY_POSITION + 1; 
				  i++) {
			if (ARG_SHOULD_BE_SENT_BY_REF(fci_cache->function_handler, i-PARAMETER_ARRAY_POSITION-1)) {
#if PHP_MAJOR_VERSION < 7
				Z_SET_ISREF_PP(args[i]);
#else
				if (UNEXPECTED(!Z_ISREF_P(&args[i]))) {
					ZVAL_NEW_REF(&args[i], &args[i]);
					defer{
						zval* z = &args[i];
						phpgo_zval_ptr_dtor(&z);
					};
					zend_error(E_WARNING, "phpgo: go: parameter %d to go routine expected to be a reference, value given", i-PARAMETER_ARRAY_POSITION-1);
				}
#endif
			}
		}
		
		PHP7_AND_ABOVE( PHPGO_MAKE_STD_ZVAL(return_value) ); 
		if( call_user_function_ex(
			EG(function_table), 
			NULL, 
			callback,                                  // the callback callable
			PHP5_VS_7(&return_value, return_value),    // return value: php5: zval**, php7: zval*
			param_count,                               // the parameter number required by the callback
			param_count ? args + PARAMETER_ARRAY_POSITION + 1 : NULL,  // the parameter list of the callback
			1, 
			NULL TSRMLS_CC
		) != SUCCESS) {
			zend_error(E_ERROR, "phpgo: execution of go routine faild");
			return;
		}
	};
	
	return true;
}

/*
 * go_debug : set the libgo debug flag
*/
void phpgo_go_debug(unsigned long debug_flag){
	co_sched.GetOptions().debug = debug_flag;
}
	
