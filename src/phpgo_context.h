#pragma once
#include <libgo/scheduler.h>
#include "freeable.h"
#include "task_local_storage.h"

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

#define NUM_TRACK_VARS	6

#define GET_HTTP_GLOBAL(name, http_globals, offset) \
do{ \
	void*  data = nullptr; \
	phpgo_zend_hash_find(&EG(symbol_table), name, sizeof(name), (void**)&data); \
	zval*  pz_arr  = PHP5_VS_7(*(zval**)data, (zval*)data); \
\
	PHP5_AND_BELOW( \
		if((http_globals)[offset]) zval_ptr_dtor( &(http_globals)[offset] ); \
		if(pz_arr) { \
			(http_globals)[offset] = pz_arr; \
			Z_ADDREF_P(pz_arr); \
		}else{ \
			(http_globals)[offset] = nullptr;  \
		} \
	) \
	PHP7_AND_ABOVE( \
		zval_ptr_dtor( &(http_globals)[offset] ); \
		if(pz_arr) { \
			ZVAL_COPY_VALUE( &(http_globals)[offset], pz_arr); \
			Z_ADDREF_P(pz_arr); \
		}else{ \
			ZVAL_NULL( &(http_globals)[offset] ); \
		} \
	) \
}while(0)

#define GET_HTTP_REQUEST_GLOBAL(http_request_global) \
do{ \
	void* data = nullptr; \
	phpgo_zend_hash_find(&EG(symbol_table), "_REQUEST", sizeof("_REQUEST"), (void**)&data); \
	zval* pz_arr  = PHP5_VS_7(*(zval**)data, (zval*)data); \
\
	PHP5_AND_BELOW( \
		if(http_request_global) zval_ptr_dtor( &(http_request_global) ); \
		if(pz_arr){ \
			(http_request_global) = pz_arr; \
			Z_ADDREF_P(pz_arr); \
		}else{ \
			(http_request_global) = nullptr; \
		} \
	) \
	PHP7_AND_ABOVE( \
		zval_ptr_dtor( &(http_request_global) ); \
		if(pz_arr){ \
			ZVAL_COPY_VALUE( &(http_request_global), pz_arr ); \
			if( Z_TYPE_P(pz_arr) != IS_NULL ) \
				Z_ADDREF_P(pz_arr); \
		}else{ \
			ZVAL_NULL( &(http_request_global) ); \
		} \
	) \
}while(0)

#define SET_HTTP_GLOBAL(name, http_globals, offset) \
do{ \
	PHP5_AND_BELOW( \
		if( (http_globals)[offset] ) {\
			phpgo_zend_hash_update( \
				&EG(symbol_table), name, sizeof(name), \
				&(http_globals)[offset], sizeof(zval *), NULL \
			); \
			Z_ADDREF_P( (http_globals)[offset] ); \
		} \
	) \
	PHP7_AND_ABOVE(\
		zval* tmp = &(http_globals)[offset]; \
		phpgo_zend_hash_update( \
			&EG(symbol_table), name, sizeof(name), \
			&tmp, sizeof(zval *), NULL \
		); \
		Z_ADDREF_P( &(http_globals)[offset] ); \
	) \
}while(0)

#define SET_HTTP_REQUEST_GLOBAL(http_request_global) \
do{ \
	PHP5_AND_BELOW( \
		if( http_request_global ) {\
			phpgo_zend_hash_update( \
			    &EG(symbol_table), "_REQUEST", sizeof("_REQUEST"),\
				&(http_request_global), sizeof(zval *), NULL \
			); \
			Z_ADDREF_P( http_request_global ); \
		} \
	) \
	PHP7_AND_ABOVE(\
		zval* tmp = &(http_request_global); \
		phpgo_zend_hash_update( \
			&EG(symbol_table), "_REQUEST", sizeof("_REQUEST"), \
			&tmp, sizeof(zval *), NULL \
		); \
		if( Z_TYPE_P(&(http_request_global)) != IS_NULL ) \
			Z_ADDREF_P( &(http_request_global) ); \
	) \
}while(0)
	
#define SET_HTTP_GLOBALS(http_globals, http_request_global) \
do{ \
	SET_HTTP_GLOBAL("_GET",    http_globals, TRACK_VARS_GET);  \
	SET_HTTP_GLOBAL("_POST",   http_globals, TRACK_VARS_POST); \
	SET_HTTP_GLOBAL("_COOKIE", http_globals, TRACK_VARS_COOKIE); \
	SET_HTTP_GLOBAL("_SERVER", http_globals, TRACK_VARS_SERVER); \
	SET_HTTP_GLOBAL("_ENV",    http_globals, TRACK_VARS_ENV); \
	SET_HTTP_GLOBAL("_FILES",  http_globals, TRACK_VARS_FILES); \
	SET_HTTP_REQUEST_GLOBAL(   http_request_global ); /*get $_REQUEST*/ \
}while(0)
	
#define GET_HTTP_GLOBALS(http_globals, http_request_global) \
do{ \
	GET_HTTP_GLOBAL("_GET",    http_globals, TRACK_VARS_GET);  \
	GET_HTTP_GLOBAL("_POST",   http_globals, TRACK_VARS_POST);  \
	GET_HTTP_GLOBAL("_COOKIE", http_globals, TRACK_VARS_COOKIE); \
	GET_HTTP_GLOBAL("_SERVER", http_globals, TRACK_VARS_SERVER); \
	GET_HTTP_GLOBAL("_ENV",    http_globals, TRACK_VARS_ENV); \
	GET_HTTP_GLOBAL("_FILES",  http_globals, TRACK_VARS_FILES); \
	GET_HTTP_REQUEST_GLOBAL(   http_request_global ); /*get $_REQUEST*/ \
}while(0)
	
#define DELREF_HTTP_GLOBALS(http_globals, http_request_global) \
do{ \
	for(int i=0; i<NUM_TRACK_VARS; i++){ \
		PHP5_AND_BELOW( \
		if((http_globals)[i]) )\
			zval_ptr_dtor( &(http_globals)[i] ); \
	} \
	PHP5_AND_BELOW( \
	if(http_request_global) ) \
		zval_ptr_dtor( &(http_request_global) ); \
}while(0)

#define REPLACE_PG_HTTP_GLOBALS_WITH(__http_globals) \
do{ \
	for(int i=0; i<NUM_TRACK_VARS; i++){ \
		zval_ptr_dtor( &PG(http_globals)[i] ); \
	} \
	memcpy( PG(http_globals), (__http_globals), sizeof(PG(http_globals)) ); \
	for(int i=0; i<NUM_TRACK_VARS; i++){ \
		PHP5_VS_7( \
			Z_ADDREF_P( (__http_globals)[i] ), \
			Z_ADDREF_P( &(__http_globals)[i] ) \
		); \
	} \
}while(0)
	
#define MAKE_HTTP_GLOBALS(http_globals, http_request_global) \
do{ \
	for(int i=0; i<NUM_TRACK_VARS; i++){ \
		PHP5_AND_BELOW(	\
			MAKE_STD_ZVAL( (http_globals)[i] ); \
			array_init( (http_globals)[i] ); \
		); \
		PHP7_AND_ABOVE( array_init( &(http_globals)[i] ) ); \
	} \
	PHP5_AND_BELOW( \
		MAKE_STD_ZVAL( http_request_global ); \
		array_init( http_request_global ); \
	); \
	PHP7_AND_ABOVE( array_init( &(http_request_global) ) ); \
}while(0)

/*null-out our concerned globals to avoid potential problem*/
#if PHP_MAJOR_VERSION < 7
#define PHPGO_INITIALIZE_RUNNING_ENVIRONMENT()           \
{                                                                  \
	EG(current_execute_data )   =  NULL;                           \
	EG(bailout )                =  NULL;                           \
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
}
#else
#define PHPGO_INITIALIZE_RUNNING_ENVIRONMENT()           \
{                                                                  \
	EG(current_execute_data )   =  NULL;                           \
	EG(bailout              )   =  NULL;                           \
	EG(vm_stack             )   =  NULL;                           \
	EG(vm_stack_top         )   =  NULL;                           \
	EG(vm_stack_end         )   =  NULL;                           \
}
#endif

struct PhpgoBaseContext{
	uint64_t                   guard__[4];
	uint64_t                   task_id;
	bool                       http_globals_cleanup_required;

	struct _zend_execute_data* EG_current_execute_data;

#if PHP_MAJOR_VERSION < 7
	/*go routine running environment*/
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
	
	TSRMLS_FIELD;  /*ZTS: void ***tsrm_ls;*/
#else 
	/* php7 */
	zend_vm_stack			   EG_vm_stack;
    zval*                      EG_vm_stack_top;
    zval*                      EG_vm_stack_end;
	
	zval                       PG_http_globals[NUM_TRACK_VARS];
	zval                       http_request_global;
#endif

	JMP_BUF*                   EG_bailout;

	uint64_t                   __guard[4];
	/**/
	PhpgoBaseContext(){
		bzero(this, sizeof(*this));
		memset(guard__, 0xcc, sizeof(guard__));
		memset(__guard, 0xcc, sizeof(__guard));
		
		PHP7_AND_ABOVE(
			for(int i=0; i< NUM_TRACK_VARS; i++)
				ZVAL_NULL( &PG_http_globals[i] );
			ZVAL_NULL( &http_request_global );
		);
	}
	
protected:
	inline void SwapOut(bool include_http_globals){
#if PHP_MAJOR_VERSION < 7
		TSRMLS_FIELD;                  
		PHPGO_LOAD_TSRMLS(this);
#endif
		
		if(include_http_globals){
			GET_HTTP_GLOBALS(this->PG_http_globals, this->http_request_global);
			http_globals_cleanup_required = true;
		}

		/* save the current EG  */    
		this->EG_current_execute_data  =  EG(current_execute_data    );

#if PHP_MAJOR_VERSION < 7
		this->EG_argument_stack        =  EG(argument_stack          ); 
		this->EG_scope                 =  EG(scope                   ); 
		this->EG_This                  =  EG(This                    ); 
		this->EG_called_scope          =  EG(called_scope            ); 
		this->EG_active_symbol_table   =  EG(active_symbol_table     ); 
		this->EG_return_value_ptr_ptr  =  EG(return_value_ptr_ptr    ); 
		this->EG_active_op_array       =  EG(active_op_array         ); 
		this->EG_opline_ptr            =  EG(opline_ptr              ); 
		this->EG_error_zval            =  EG(error_zval              ); 
		this->EG_error_zval_ptr        =  EG(error_zval_ptr          ); 
		this->EG_user_error_handler    =  EG(user_error_handler      ); 
#else
		this->EG_vm_stack              =  EG(vm_stack                );
        this->EG_vm_stack_top          =  EG(vm_stack_top            );
		this->EG_vm_stack_end          =  EG(vm_stack_end            );
#endif

		this->EG_bailout               =  EG(bailout                 );
	}

	inline void SwapIn(bool include_http_globals){
#if PHP_MAJOR_VERSION < 7
		TSRMLS_FIELD;
		PHPGO_LOAD_TSRMLS(this);
#endif
		
		/* load EG from the task specific context*/                            
		EG(current_execute_data )   =  this->EG_current_execute_data;

#if PHP_MAJOR_VERSION < 7
		EG(argument_stack       )   =  this->EG_argument_stack      ; 
		EG(scope                )   =  this->EG_scope               ; 
		EG(This                 )   =  this->EG_This                ; 
		EG(called_scope         )   =  this->EG_called_scope        ; 
		EG(active_symbol_table  )   =  this->EG_active_symbol_table ; 
		EG(return_value_ptr_ptr )   =  this->EG_return_value_ptr_ptr; 
		EG(active_op_array      )   =  this->EG_active_op_array     ; 
		EG(opline_ptr           )   =  this->EG_opline_ptr          ; 
		EG(error_zval           )   =  this->EG_error_zval          ; 
		EG(error_zval_ptr       )   =  this->EG_error_zval_ptr      ; 
		EG(user_error_handler   )   =  this->EG_user_error_handler  ;
#else
		EG(vm_stack             )	=  this->EG_vm_stack            ;
	    EG(vm_stack_top         )	=  this->EG_vm_stack_top        ;
	    EG(vm_stack_end         )	=  this->EG_vm_stack_end        ;
#endif

		EG(bailout              )   =  this->EG_bailout             ;
		
		if(include_http_globals){			
			REPLACE_PG_HTTP_GLOBALS_WITH(this->PG_http_globals);
			SET_HTTP_GLOBALS(this->PG_http_globals, this->http_request_global);  		
		}
	}

public:
	inline void Cleanup(){
		if (this->http_globals_cleanup_required) {
			DELREF_HTTP_GLOBALS(this->PG_http_globals, this->http_request_global);
		}
	}
};

struct PhpgoContext : public PhpgoBaseContext, public FreeableImpl{
public:
	uint64_t                   go_routine_options;
	bool                       go_routine_finished;
	
public:
	PhpgoContext(uint64_t options TSRMLS_DC){
#if PHP_MAJOR_VERSION < 7
		TSRMLS_SET_CTX(this->TSRMLS_C);
#else
#endif
		this->go_routine_options  = options;
		this->go_routine_finished = false;
		this->task_id             = g_Scheduler.GetCurrentTaskID();
	}
	
	inline void SwapOut(){
		PhpgoBaseContext::SwapOut(this->go_routine_options & GoRoutineOptions::gro_isolate_http_globals);
		
		// the coroutine ( not the scheduler ) is finished 
		// free the task local storage (including this context itself)
		if(this->go_routine_finished) {
			this->Cleanup();
		}
	}
		
	inline void SwapIn(){
		PhpgoBaseContext::SwapIn(this->go_routine_options & GoRoutineOptions::gro_isolate_http_globals);
	}
	
	inline void SetFinished(bool finished){
		this->go_routine_finished = finished;
	}
	
	inline void Cleanup(){
		PhpgoBaseContext::Cleanup();	
		// should be last sentence as this will free the context itself
		TaskLocalStorage::FreeSpecifics(this->task_id);
	}
};

// the Scheduler Context is essentially the same as the Task's context,
// but since Scheduler Context  is thread local and thread locals cannot
// have virtual members, we have to remove the FreeableImpl from the
// Scheduler Context...
struct PhpgoSchedulerContext : public PhpgoBaseContext{
public:	
	PhpgoSchedulerContext(){
		this->task_id = 0;
		
#if PHP_MAJOR_VERSION < 7
		TSRMLS_FETCH();                     // void ***tsrm_ls = (void ***) ts_resource_ex(0, NULL)
		TSRMLS_SET_CTX(this->TSRMLS_C);     // this->tsrm_ls = (void ***) tsrm_ls
#endif
	}
	
	inline void SwapOut(bool include_http_globals){
		PhpgoBaseContext::SwapOut(include_http_globals);
	}
		
	inline void SwapIn(bool include_http_globals){
		PhpgoBaseContext::SwapIn(include_http_globals);
	}
};

// the scheduler may be executed in multiple thread: 
// use thread local variable to store the scheduler EG's	
extern thread_local PhpgoSchedulerContext scheduler_ctx;