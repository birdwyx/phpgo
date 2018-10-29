#pragma once

/*http globals*/
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

