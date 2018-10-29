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


#define MAX_CONTEXTS (128)

#ifdef ZTS
	#if PHP_MAJOR_VERSION < 7
		#define SCHEDULER_TSRMLS_D void ***scheduler_tsrm_ls
		#define SCHEDULER_TSRMLS_C scheduler_tsrm_ls
		#define SCHEDULER_TSRMLS_CC ,scheduler_tsrm_ls
		
		#define G_SCHEDULER_TSRMLS_D extern void thread_local ***g_scheduler_tsrm_ls
		#define SCHEDULER_TSRMLS_SET_CTX(ctx) ctx = (void ***)scheduler_tsrm_ls
	#else
		#define SCHEDULER_TSRMLS_D
	    #define SCHEDULER_TSRMLS_C
		#define SCHEDULER_TSRMLS_CC
		
		#define SCHEDULER_TSRMLS_SET_CTX(ctx)
		#define G_SCHEDULER_TSRMLS_D
	#endif
#else
	#define SCHEDULER_TSRMLS_D
	#define SCHEDULER_TSRMLS_C
	#define SCHEDULER_TSRMLS_CC
	#define SCHEDULER_TSRMLS_SET_CTX(ctx)
	#define G_SCHEDULER_TSRMLS_D
#endif

#define ADD_CONTEXT(base_context, context) \
do{ \
	if(base_context->context_count >= MAX_CONTEXTS){ \
		zend_error("adding context failed: limit reached"); \
	}else{ \
		base_context->contexts[base_context->context_count++] = context; \
	} \
} while(0)
	
#define __PHPGO_MAKE_CONTEXT( \
	context, \
	OnCreated, \
	OnStart, \
	OnSwapIn, \
	OnSwapOut, \
	OnFinished, \
	OnEnd, \
	base_context \
	SCHEDULER_TSRMLS_CC \
	TSRMLS_CC \
) \
do{ \
	context->OnCreated = OnCreated; \
	context->OnStart = OnStart; \
	context->OnSwapIn = OnSwapIn; \
	context->OnSwapOut = OnSwapOut; \
	context->OnFinished = OnFinished; \
	context->OnEnd = OnEnd; \
	context->base_context = base_context; \
	bzero(context->ctx0, sizeof(context->ctx0)); \
	bzero(context->ctx1, sizeof(context->ctx1)); \
	SCHEDULER_TSRMLS_SET_CTX(context->ctx0.tsrm_ls); \
	TSRMLS_SET_CTX(context->ctx1.tsrm_ls); \
}while(0)

#define PHPGO_DECLARE_AND_MAKE_CONTEXT(context_type, context, base_context SCHEDULER_TSRMLS_DC TSRMLS_DC) \
CLASS(context_type)* context = NULL; \
do{ \
	context = (context_type *) malloc(sizeof(context_type)); \
	if(!context) { \
		zend_error("failed to alloc context"); \
	}else{ \
		__PHPGO_MAKE_CONTEXT( \
			context, \
			CLASS_FUNC(context_type, OnCreated), \
			CLASS_FUNC(context_type, OnStart), \
			CLASS_FUNC(context_type, OnSwapIn), \
			CLASS_FUNC(context_type, OnSwapOut), \
			CLASS_FUNC(context_type, OnFinished), \
			CLASS_FUNC(context_type, OnEnd), \
			base_context \
			SCHEDULER_TSRMLS_CC \
			TSRMLS_CC \
		); \
	} \
}while(0)

typedef struct {
	uint64_t                   guard__[8];
	TSRMLS_D;
	SCHEDULER_TSRMLS_D;
	uint64_t                   task_id;
	uint64_t                   gr_options;
	uint64_t                   __guard[8];
	Context*                   contexts[MAX_CONTEXTS];
	int                        context_count;
	
	GrData(uint64_t task_id, uint64_t gr_options TSRMLS_DC){
		TSRMLS_SET_CTX(this->TSRMLS_C);             //this->trsm_ls = (void ***) tsrm_ls
		
		{ //avoid tsrm_ls redeclare
			TSRMLS_FETCH();                         // void ***tsrm_ls = (void ***) ts_resource_ex(0, NULL)
			TSRMLS_SET_CTX(SCHEDULER_TSRMLS_C);     // scheduler_tsrm_ls = (void ***) tsrm_ls
		}
		
		this->task_id = task_id;
		this->gr_options = gr_options;
		
		bzero(&guard__, sizeof(guard__));
		bzero(&__guard, sizeof(__guard));
	}
} GrData;





//---------

typedef struct{
	uint64_t                   guard__[8];
#if PHP_MAJOR_VERSION < 7
	zval*                      PG_http_globals[NUM_TRACK_VARS];
	zval*                      http_request_global;
#else
	zval                       PG_http_globals[NUM_TRACK_VARS];
	zval                       http_request_global;
#endif

	uint64_t                   __guard[8];
} PhpgoIsolateHttpGlobalsContext;


CLASS(PhpgoIsolateHttpGlobalscontext){
	DECLARE_CLASS_FUNC(PhpgoIsolateHttpGlobalscontext, OnCreated);
	DECLARE_CLASS_FUNC(PhpgoIsolateHttpGlobalscontext, OnStart);
	DECLARE_CLASS_FUNC(PhpgoIsolateHttpGlobalscontext, OnSwapIn);
	DECLARE_CLASS_FUNC(PhpgoIsolateHttpGlobalscontext, OnSwapOut);
	DECLARE_CLASS_FUNC(PhpgoIsolateHttpGlobalscontext, OnFinished);
	PhpgoBaseContext*    base_context;
	PhpgoIsolateHttpGlobalsContext ctx0;
	PhpgoIsolateHttpGlobalsContext ctx1;
};

DEFINE_EXTERN_CLASS_FUNC(PhpgoIsolateHttpGlobalscontext, OnCreated);
DEFINE_EXTERN_CLASS_FUNC(PhpgoIsolateHttpGlobalscontext, OnStart);
DEFINE_EXTERN_CLASS_FUNC(PhpgoIsolateHttpGlobalscontext, OnSwapIn);
DEFINE_EXTERN_CLASS_FUNC(PhpgoIsolateHttpGlobalscontext, OnSwapOut);
DEFINE_EXTERN_CLASS_FUNC(PhpgoIsolateHttpGlobalscontext, OnFinished);

DEFINE_CLASS_FUNC(PhpgoIsolateHttpGlobalscontext, OnCreated){
	PHP7_AND_ABOVE(
		for(int i=0; i< NUM_TRACK_VARS; i++){
			ZVAL_NULL( &(__this->ctx0.PG_http_globals[i]) );
			ZVAL_NULL( &(__this->ctx1.PG_http_globals[i]) );
		}
		ZVAL_NULL( &(__this->ctx0.http_request_global) );
		ZVAL_NULL( &(__this->ctx1.http_request_global) );
	);
	
	// this function is invoked in the parent context
	// new http globals are made thus will be isolated with the parent
	MAKE_HTTP_GLOBALS(__this->ctx1.PG_http_globals, __this->ctx1.http_request_global);
	GET_HTTP_GLOBALS(__this->ctx1.PG_http_globals, __this->ctx1.http_request_global);
};

DEFINE_CLASS_FUNC(PhpgoIsolateHttpGlobalscontext, OnStart){
};

DEFINE_CLASS_FUNC(PhpgoIsolateHttpGlobalscontext, OnSwapIn){
#if PHP_MAJOR_VERSION < 7
	TSRMLS_FIELD;                  
	PHPGO_LOAD_TSRMLS(&__this->ctx0);
#endif
	GET_HTTP_GLOBALS(__this->ctx0.PG_http_globals, __this->ctx0.http_request_global);

#if PHP_MAJOR_VERSION < 7
	//TSRMLS_FIELD;                  
	PHPGO_LOAD_TSRMLS(&__this->ctx1);
#endif	
	REPLACE_PG_HTTP_GLOBALS_WITH(__this->ctx1.PG_http_globals);
	SET_HTTP_GLOBALS(__this->ctx1.PG_http_globals, __this->ctx1.http_request_global);  
};

DEFINE_CLASS_FUNC(PhpgoIsolateHttpGlobalscontext, OnSwapOut){
#if PHP_MAJOR_VERSION < 7
	TSRMLS_FIELD;                  
	PHPGO_LOAD_TSRMLS(&__this->ctx1);
#endif
	GET_HTTP_GLOBALS(__this->ctx1.PG_http_globals, __this->ctx1.http_request_global);

#if PHP_MAJOR_VERSION < 7
	//TSRMLS_FIELD;                  
	PHPGO_LOAD_TSRMLS(&__this->ctx0);
#endif	
	REPLACE_PG_HTTP_GLOBALS_WITH(__this->ctx0.PG_http_globals);
	SET_HTTP_GLOBALS(__this->ctx0.PG_http_globals, __this->ctx0.http_request_global); 
};

// function done, still in go routine
DEFINE_CLASS_FUNC(PhpgoIsolateHttpGlobalscontext, OnFinished){
}

//everything done, still in go routine, about to swap out
DEFINE_CLASS_FUNC(PhpgoIsolateHttpGlobalscontext, OnEnd){
	DELREF_HTTP_GLOBALS(__this->ctx0.PG_http_globals, __this->ctx0.http_request_global);
	DELREF_HTTP_GLOBALS(__this->ctx1.PG_http_globals, __this->ctx1.http_request_global);
};

//------------

uint64_t thread_local g_task_id;

extern "C" {
	
	uint64_t libgo_go(
		FUNC func,
		void* param,
		uint64_t stack_size,
		uint64_t options
		TSRMLS_DC
	){
		
		if(!func) return 0;
		
		g_task_id_created = 0;
		
		go[=]{
			if( !(base_context = TaskLocalStorage::GetSpecific(phpgo_context_key, base_context)) ) {
				return;
			}
			
			foreach(base_context->contexts as context){
				context->OnStart(base_context);
			}
			
			func(param);
			
			foreach(base_context->contexts as context){
				context->OnFinished(base_context);
			}
		}
		
		//now the PhpgoTaskcontext::OnCreated should have been invoked;
		//the g_task_id_created should have been set to the task just created
		
		kls_key_t phpgo_context_key = TaskLocalStorage::CreateKey("PhpgoBaseContext");
		if( !phpgo_context_key ) {
			return 0;
		}
		
		// set the tsrm_ls to my prarent, i.e., inherit all globals from my parent
		PhpgoBaseContext* base_context = new PhpgoBaseContext(g_task_id_created, options TSRMLS_CC); 
		if( !base_context ) {
			return;
		}
		
		if( !TaskLocalStorage::SetSpecific(phpgo_context_key, base_context, task_id) ) {
			delete base_context;
			return;
		}
		
		/** add contexts **/
		PHPGO_DECLARE_AND_MAKE_CONTEXT(
			PhpgoBasecontext,
			phpgo_base_context,
			base_context
			SCHEDULER_TSRMLS_CC
			TSRMLS_CC
		);
		ADD_CONTEXT(base_context, phpgo_base_context);

		if(options & GoRoutineOptions::isolate_sapi_globals){
			PHPGO_DECLARE_AND_MAKE_CONTEXT(
				PhpgoIsolateSapiGlobalscontext,
				phpgo_isolate_sapi_globals_context,
				base_context
				SCHEDULER_TSRMLS_CC
				TSRMLS_CC
			);			
			ADD_CONTEXT(base_context, phpgo_isolate_sapi_globals_context);
		}
		
		if(options & GoRoutineOptions::isolate_http_globals){
			PHPGO_DECLARE_AND_MAKE_CONTEXT(
				PhpgoIsolateHttpGlobalscontext,
				phpgo_isolate_http_globals_context,
				base_context
				SCHEDULER_TSRMLS_CC
				TSRMLS_CC
			);			
			ADD_CONTEXT(base_context, phpgo_isolate_http_globals_context);
		}
		
		if(options & GoRoutineOptions::inherit_http_globals){
			PHPGO_DECLARE_AND_MAKE_CONTEXT(
				PhpgoInheritHttpGlobalscontext,
				phpgo_inherit_http_globals_context,
				base_context
				SCHEDULER_TSRMLS_CC
				TSRMLS_CC
			);			
			ADD_CONTEXT(base_context, phpgo_inherit_http_globals_context);
		}
		
		if(options & GoRoutineOptions::isolate_ob_globals){
			PHPGO_DECLARE_AND_MAKE_CONTEXT(
				PhpgoObcontext,
				phpgo_ob_context,
				base_context
				SCHEDULER_TSRMLS_CC
				TSRMLS_CC
			);			
			ADD_CONTEXT(base_context, phpgo_ob_context);
		}
		/** end add contexts **/
		
		/* call the OnCreate for all contexts */
		for(int i = 0; i < base_context->context_count; i++)
			((context*)(base_context->contexts)[i])->OnCreated((context*)(base_context->contexts)[i]);
		}
		
		return g_task_id_created;
		
	}
	
}