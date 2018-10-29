#include "isolate_httpg_context.h"

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
} GrIsolateHttpGlobalsData;


CLASS(PhpgoIsolateHttpGlobalscontext){
	BASE_CONTEXT_FIELDS;
	GrIsolateHttpGlobalsData ctx0;
	GrIsolateHttpGlobalsData ctx1;
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
