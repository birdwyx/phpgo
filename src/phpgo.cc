/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2015 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

/*
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"*/


#include "stdinc.h"
#include <signal.h>       /* for signal */  
#include <execinfo.h>     /* for backtrace() */
#include <libgo/coroutine.h>

/*remove the libgo go() included from the coroutine.h since 
we'll also have function go defined later on in this file*/
#undef go  

#include "php_phpgo.h"
#include "go.h"
#include "go_scheduler.h"
#include "go_chan.h"
#include "go_mutex.h"
#include "go_runtime.h"
#include "go_wait_group.h"
#include "go_time.h"
#include "go_select.h"
#include "zend_interfaces.h"
#include "defer.h"


/* If you declare any globals in php_phpgo.h uncomment this:*/
ZEND_DECLARE_MODULE_GLOBALS(phpgo)

/* True global resources - no need for thread safety here */
//static int le_phpgo;

zend_class_entry  ce_go_chan,      *ce_go_chan_ptr;
zend_class_entry  ce_go_mutex,     *ce_go_mutex_ptr;
zend_class_entry  ce_go_wait_group,*ce_go_wait_group_ptr;
zend_class_entry  ce_go_scheduler, *ce_go_scheduler_ptr;
zend_class_entry  ce_go_selector,  *ce_go_selector_ptr;
zend_class_entry  ce_go_time,      *ce_go_time_ptr;
zend_class_entry  ce_go_runtime,   *ce_go_runtime_ptr;

/* {{{ arginfo_go_chan_push[]
 *
 */
//ZEND_BEGIN_ARG_INFO_EX(arginfo_go_chan_push, pass_rest_by_reference, return_reference, required_num_args)
ZEND_BEGIN_ARG_INFO_EX(arginfo_go_chan_push, 0, 0, 1)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_go_wait_group_add, 0, 0, 1)
	ZEND_ARG_INFO(0, delta)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_go_selector_ctor, 0, 0, 1)
	ZEND_ARG_INFO(0, handle)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_go_time_tick, 0, 0, 1)
	ZEND_ARG_INFO(0, nanoseconds)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_go_time_after, 0, 0, 1)
	ZEND_ARG_INFO(0, nanoseconds)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_go_time_sleep, 0, 0, 1)
	ZEND_ARG_INFO(0, nanoseconds)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_go_selector_loop, 0, 0, 1)
	ZEND_ARG_INFO(0, done_chan)
ZEND_END_ARG_INFO()
/* }}} */

/* {{{ phpgo_functions[]
 *
 * Every user visible function must have an entry in phpgo_functions[].
 */
const zend_function_entry phpgo_functions[] = {
	//PHP_FE(confirm_phpgo_compiled,	NULL)		/* For testing, remove later. */
	PHP_FE(go, NULL)
	PHP_FE(goo, NULL)
	PHP_FE(go_debug, NULL)
	//ZEND_NS_NAMED_FE(PHPGO_NS, go_debug, ZEND_FN(go_go_debug), NULL)
	
	PHP_FE(select, NULL)
	PHP_FE_END	/* Must be the last line in phpgo_functions[] */
};
/* }}} */



/* {{{ phpgo_channel_functions[]
 *
 */
const zend_function_entry go_chan_methods[] = {
	PHP_ME(Chan,           __construct,  NULL,                      ZEND_ACC_PUBLIC|ZEND_ACC_CTOR  )
	PHP_ME(Chan,           Push,         arginfo_go_chan_push,      ZEND_ACC_PUBLIC                )
	PHP_ME(Chan,           Pop,          NULL,                      ZEND_ACC_PUBLIC                )
	PHP_ME(Chan,           TryPush,      NULL,                      ZEND_ACC_PUBLIC                )
	PHP_ME(Chan,           TryPop,       NULL,                      ZEND_ACC_PUBLIC                )
	PHP_ME(Chan,           Close,        NULL,                      ZEND_ACC_PUBLIC                )
	PHP_ME(Chan,           __destruct,   NULL,                      ZEND_ACC_PUBLIC|ZEND_ACC_DTOR  )
	PHP_FE_END	/* Must be the last line */
};

const zend_function_entry go_mutex_methods[] = {
	PHP_ME(Mutex,          __construct,  NULL,                      ZEND_ACC_PUBLIC|ZEND_ACC_CTOR  )
	PHP_ME(Mutex,          Lock,         NULL,                      ZEND_ACC_PUBLIC                )
	PHP_ME(Mutex,          Unlock,       NULL,                      ZEND_ACC_PUBLIC                )
	PHP_ME(Mutex,          TryLock,      NULL,                      ZEND_ACC_PUBLIC                )
	PHP_ME(Mutex,          IsLock,       NULL,                      ZEND_ACC_PUBLIC                )
	PHP_ME(Mutex,          __destruct,   NULL,                      ZEND_ACC_PUBLIC|ZEND_ACC_DTOR  )
	PHP_FE_END	/* Must be the last line */
};

const zend_function_entry go_wait_group_methods[] = {
	PHP_ME(WaitGroup,      __construct,  NULL,                      ZEND_ACC_PUBLIC|ZEND_ACC_CTOR  )
	PHP_ME(WaitGroup,      Add,          arginfo_go_wait_group_add, ZEND_ACC_PUBLIC                )
	PHP_ME(WaitGroup,      Done,         NULL,                      ZEND_ACC_PUBLIC                )
	PHP_ME(WaitGroup,      Wait,         NULL,                      ZEND_ACC_PUBLIC                )
	PHP_ME(WaitGroup,      __destruct,   NULL,                      ZEND_ACC_PUBLIC|ZEND_ACC_DTOR  )
	PHP_FE_END	/* Must be the last line */
};

const zend_function_entry go_scheduler_methods[] = {
	PHP_ME(Scheduler,      Run,          NULL,                      ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Scheduler,      Join,         NULL,                      ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Scheduler,      Loop,         NULL,                      ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_FE_END	/* Must be the last line */
};

const zend_function_entry go_selector_methods[] = {
	PHP_ME(Selector,       __construct,  arginfo_go_selector_ctor,  ZEND_ACC_PUBLIC|ZEND_ACC_CTOR  )
	PHP_ME(Selector,       Select,       NULL,                      ZEND_ACC_PUBLIC                )
	PHP_ME(Selector,       Loop,         arginfo_go_selector_loop,  ZEND_ACC_PUBLIC                )
	PHP_ME(Selector,       __destruct,   NULL,                      ZEND_ACC_PUBLIC|ZEND_ACC_DTOR  )
	PHP_FE_END	/* Must be the last line */
};

const zend_function_entry go_time_methods[] = {
	PHP_ME(Time,           Tick,        arginfo_go_time_tick,      ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Time,           After,       arginfo_go_time_after,     ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Time,           Sleep,       arginfo_go_time_sleep,     ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_FE_END	/* Must be the last line */
};

const zend_function_entry go_runtime_methods[] = {
	PHP_ME(Runtime,        NumGoroutine, NULL,                     ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Runtime,        Gosched,      NULL,                     ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Runtime,        Goid,         NULL,                     ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_FE_END	/* Must be the last line */
};
/* }}} */

/* {{{ phpgo_module_entry
 */
zend_module_entry phpgo_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"phpgo",
	phpgo_functions,
	PHP_MINIT(phpgo),
	PHP_MSHUTDOWN(phpgo),
	PHP_RINIT(phpgo),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(phpgo),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(phpgo),
#if ZEND_MODULE_API_NO >= 20010901
	PHP_PHPGO_VERSION,
#endif
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_PHPGO
ZEND_GET_MODULE(phpgo)
#endif

/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("phpgo.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_phpgo_globals, phpgo_globals)
    STD_PHP_INI_ENTRY("phpgo.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_phpgo_globals, phpgo_globals)
PHP_INI_END()
*/
/* }}} */

/* {{{ php_phpgo_init_globals
 */
/* Uncomment this function if you have INI entries*/
static void php_phpgo_init_globals(zend_phpgo_globals *phpgo_globals)
{
	phpgo_globals->phpgo_initialized = false;
	phpgo_globals->running_internal_go_routines = 0;
}

/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(phpgo)
{
	/* If you have INI entries, uncomment these lines 
	REGISTER_INI_ENTRIES();
	*/
	
	ZEND_INIT_MODULE_GLOBALS(phpgo, php_phpgo_init_globals, NULL);
	
	INIT_NS_CLASS_ENTRY(ce_go_chan,      PHPGO_NS, "Chan",      go_chan_methods);  // 类名为 go\Chan
	INIT_NS_CLASS_ENTRY(ce_go_mutex,     PHPGO_NS, "Mutex",     go_mutex_methods); 
	INIT_NS_CLASS_ENTRY(ce_go_wait_group,PHPGO_NS, "WaitGroup", go_wait_group_methods); 
	INIT_NS_CLASS_ENTRY(ce_go_scheduler, PHPGO_NS, "Scheduler", go_scheduler_methods); 
	INIT_NS_CLASS_ENTRY(ce_go_selector,  PHPGO_NS, "Selector",  go_selector_methods); 
	INIT_NS_CLASS_ENTRY(ce_go_time,      PHPGO_NS, "Time",      go_time_methods); 
	INIT_NS_CLASS_ENTRY(ce_go_runtime,   PHPGO_NS, "Runtime",   go_runtime_methods); 
	
	ce_go_chan_ptr      = zend_register_internal_class(&ce_go_chan TSRMLS_CC);
	ce_go_mutex_ptr     = zend_register_internal_class(&ce_go_mutex TSRMLS_CC);
	ce_go_wait_group_ptr= zend_register_internal_class(&ce_go_wait_group TSRMLS_CC);
	ce_go_scheduler_ptr = zend_register_internal_class(&ce_go_scheduler TSRMLS_CC);
	ce_go_selector_ptr  = zend_register_internal_class(&ce_go_selector TSRMLS_CC);
	ce_go_time_ptr      = zend_register_internal_class(&ce_go_time TSRMLS_CC);
	ce_go_runtime_ptr   = zend_register_internal_class(&ce_go_runtime TSRMLS_CC);
    
	//zend_declare_property_long(ce_go_chan_ptr,"handle",  strlen("handle"),  -1, ZEND_ACC_PUBLIC TSRMLS_CC);
	//zend_declare_property_long(ce_go_chan_ptr,"capacity",strlen("capacity"), 0, ZEND_ACC_PUBLIC TSRMLS_CC);
	
	zend_declare_class_constant_long(ce_go_time_ptr, "NANOSECOND",  sizeof("NANOSECOND")-1,  GoTime::Nanosecond   TSRMLS_CC);
	zend_declare_class_constant_long(ce_go_time_ptr, "MICROSECOND", sizeof("MICROSECOND")-1, GoTime::Microsecond  TSRMLS_CC);
	zend_declare_class_constant_long(ce_go_time_ptr, "MILLISECOND", sizeof("MILLISECOND")-1, GoTime::Millisecond  TSRMLS_CC);
	zend_declare_class_constant_long(ce_go_time_ptr, "SECOND",      sizeof("SECOND")-1,      GoTime::Second       TSRMLS_CC);
	zend_declare_class_constant_long(ce_go_time_ptr, "MINUTE",      sizeof("MINUTE")-1,      GoTime::Minute       TSRMLS_CC);
	zend_declare_class_constant_long(ce_go_time_ptr, "HOUR",        sizeof("HOUR")-1,        GoTime::Hour         TSRMLS_CC);
	
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(phpgo)
{
	/* uncomment this line if you have INI entries
	UNREGISTER_INI_ENTRIES();
	*/
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(phpgo)
{
	phpgo_initialize();
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(phpgo)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(phpgo)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "phpgo support", "enabled");
	php_info_print_table_end();

	/* Remove comments if you have entries in php.ini
	DISPLAY_INI_ENTRIES();
	*/
}
/* }}} */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */

 /* {{{ proto Chan::__construct
  * Create a go channel object
  */
 PHP_METHOD(Chan,__construct){
	 
	//printf("Chan::__construct\n");
	
	long   capacity = 0;
	char*  name     = NULL;
	size_t name_len = 0;
	zval*  z1       = NULL;
	zval*  z2       = NULL;
	zval*  z3       = NULL;
	bool   copy     = false;
	
	if( zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|z", &z1) == FAILURE ){
		zend_error(E_ERROR, "phpgo: Chan::__construct: getting parameter failure");
		RETURN_NULL();
	}
	
	if( z1 ){
	    if( Z_TYPE_P(z1) == IS_LONG ){
			capacity = Z_LVAL_P(z1);
		}else if( Z_TYPE_P(z1) == IS_ARRAY /*&& z1->value.ht  not necessary,remove for php7*/){
			
			void** data = NULL;
			#define zvalp_data PHP5_VS_7( *(zval**)data, (zval*)data )
			
			if( phpgo_zend_hash_find(HASH_OF(z1), "name", sizeof("name"), (void**)&data) == SUCCESS ){
				if(Z_TYPE_P( zvalp_data ) != IS_STRING){
					zend_error(E_ERROR, "phpgo: Chan( $options ): option \"name\" must be string");
					RETURN_NULL();
				}
				name     = Z_STRVAL_P( zvalp_data );
				name_len = Z_STRLEN_P( zvalp_data );
			}
			if( phpgo_zend_hash_find(HASH_OF(z1), "capacity", sizeof("capacity"), (void**)&data) == SUCCESS ){
				if(Z_TYPE_P( zvalp_data ) != IS_LONG){
					zend_error(E_ERROR, "phpgo: Chan( $options ): option \"capacity\" must be long");
					RETURN_NULL();
				}
				capacity     = Z_LVAL_P( zvalp_data );
			}
			if( phpgo_zend_hash_find(HASH_OF(z1), "copy", sizeof("copy"), (void**)&data) == SUCCESS ){
				if( !PHPGO_ZVAL_IS_BOOL( zvalp_data ) ){
					zend_error(E_ERROR, "phpgo: Chan( $options ): option \"copy\" must be bool");
					RETURN_NULL();
				}
				copy     = Z_BVAL_P( zvalp_data );
			}
		}else{
			zend_error(E_ERROR, "phpgo: Chan( long $capacity| array $options ): parameter 1 must be long or array");
			RETURN_NULL();
			//invalid argument
		}
	}else{
		//no argument provided
	}
	
	if( capacity < 0 ){
		zend_error(E_ERROR, "phpgo: Chan(): the capacity must be greater than or equal to 0");
		RETURN_NULL();
	}

	void* chan = GoChan::Create(capacity, name, name_len, copy);
	
	return_value = getThis();
	zend_update_property_long  (ce_go_chan_ptr, return_value, "handle",   sizeof("handle")-1,   (long)chan    TSRMLS_CC);
	zend_update_property_string(ce_go_chan_ptr, return_value, "name",     sizeof("name")-1,     name?name:""  TSRMLS_CC);
	zend_update_property_long  (ce_go_chan_ptr, return_value, "capacity", sizeof("capacity")-1, capacity      TSRMLS_CC);
	zend_update_property_bool  (ce_go_chan_ptr, return_value, "copy",     sizeof("copy")-1,     copy          TSRMLS_CC);
 }
 /* }}} */
 
  /* {{{ proto Chan::push
  * Push object to channel, block if channel not available to write
  */
 PHP_METHOD(Chan,Push){
	 
	//printf("Chan::Push\n");
	
	zval* chan     = NULL;
	zval* z        = NULL;
	
	if( zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &z) == FAILURE ){
        zend_error(E_ERROR, "phpgo: Chan::push: getting parameter failure");
		RETURN_FALSE;
    }
	
	auto self = getThis();
	chan = phpgo_zend_read_property(ce_go_chan_ptr, self, "handle", sizeof("handle")-1, true TSRMLS_CC);

	if(!chan)
		RETURN_FALSE;
	
	auto lchan = Z_LVAL_P(chan);
	auto rc    = GoChan::Push( (void*)lchan, z TSRMLS_CC);
	
	if( rc==GoChan::RCode::channel_closed ){
		zend_error(E_WARNING, "phpgo: Chan::push(): try to push to an allready closed channel");
	}
	
	RETURN_BOOL( rc==GoChan::RCode::success );
 }
 /* }}} */
 
 /* {{{ proto Chan::tryPush
  * Try to push object to channel, if writable
  * otherwise return immediately, with channel unwritten
  */
 PHP_METHOD(Chan,TryPush){
	 
	//printf("Chan::TryPush\n");
	
	zval* chan     = NULL;
	zval* z        = NULL;
	
	if( zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &z) == FAILURE ){
        zend_error(E_ERROR, "phpgo: Chan::tryPush: getting parameter failure");
		RETURN_FALSE;
    }
	
	auto self = getThis();
	chan = phpgo_zend_read_property(ce_go_chan_ptr, self, "handle", sizeof("handle")-1, true TSRMLS_CC);

	if(!chan)
		RETURN_FALSE;
	
	auto lchan = Z_LVAL_P(chan);
	auto rc = GoChan::TryPush( (void*)lchan, z TSRMLS_CC);
	
	if( rc==GoChan::RCode::channel_closed ){
		zend_error(E_WARNING, "phpgo: Chan::tryPush(): try to push to an allready closed channel");
	}
	
	RETURN_BOOL( rc==GoChan::RCode::success );
 }
 /* }}} */
 
 
 /* {{{ proto Chan::pop
  * pop an object from channel, block if no data to read
  * return 
  *   data read if available, or
  *   false on error, or
  *   NULL if channel closed and no data ready to read
  */
 PHP_METHOD(Chan,Pop){
	 
	//printf("Chan::Pop\n");
	
	zval* self = getThis();
	zval* chan = phpgo_zend_read_property(ce_go_chan_ptr, self, "handle", sizeof("handle")-1, true TSRMLS_CC);

	if(!chan)
		RETURN_FALSE;
	
	auto lchan = Z_LVAL_P(chan);	
	zval* z = GoChan::Pop( (void*)lchan );
	
	//Pop return false on error
	if(!z)
		RETURN_FALSE;

	// php7: this z was emalloc'ed in ChannelData::ChannelData() 
	// or inside the GoChan::Pop()
	// we'll need to efree it by ourselves since the zval dtor's
	// won't do this for us in php7
	// note: the PHPGO_FREE_PZVAL has no effect in php5
	defer{ PHPGO_FREE_PZVAL(z); };

	//return the zval, or NULL if channel closed and no data ready to read
	RETURN_ZVAL(z, 1, 1);
 }
 /* }}} */
 
 /* {{{ proto Chan::tryPop
  * Try to pop data from channel, if readable
  * return immediately if no data to read
  * return:
  * 	data read if available
  * 	false if channel not ready for reading
  * 	NULL if channel closed and no data ready to read
  */
 PHP_METHOD(Chan,TryPop){
	 
	//printf("Chan::TryPop\n");
	
	zval* self = getThis();
	zval* chan = phpgo_zend_read_property(ce_go_chan_ptr, self, "handle", sizeof("handle")-1, true TSRMLS_CC);

	if(!chan)
		RETURN_FALSE;
	
	auto lchan = Z_LVAL_P(chan);	
	zval* z = GoChan::TryPop( (void*)lchan );
	
	// GoChan::TryPop will
	//     return nullptr if not ready
	//     return ZVAL_NULL if closed and no data ready to read
	//     otherwise return data read
	
	// return false if channel not ready
	if(!z)
		RETURN_FALSE;

	// php7: this z was emalloc'ed in ChannelData::ChannelData() 
	// or inside the GoChan::TryPop()
	// we'll need to efree it by ourselves since the zval dtor's
	// won't do this for us in php7
	// note: the PHPGO_FREE_PZVAL has no effect in php5
	defer{ PHPGO_FREE_PZVAL(z); };
	
	// return the zval, or NULL if channel closed and no data ready to read
	RETURN_ZVAL(z, 1, 1);
 }
 /* }}} */
 
 
  /* {{{ proto Chan::close
  * Close a channel
  */
 PHP_METHOD(Chan,Close){
	 
	//printf("Chan::Close\n");
	
	zval* self = getThis();
	zval* chan = phpgo_zend_read_property(ce_go_chan_ptr, self, "handle", sizeof("handle")-1, true TSRMLS_CC);

	if(!chan)
		RETURN_FALSE;
	
	auto lchan = Z_LVAL_P(chan);	
	bool ok = GoChan::Close( (void*)lchan );
	if(!ok){
		zend_error(E_WARNING, "phpgo: Chan::close(): try to close an allready closed channel");
	}
	
	RETURN_BOOL(ok); 
 }
 /* }}} */
 
 
 /* {{{ proto Chan::__destruct
  * Destroy a go channel object
  */
 PHP_METHOD(Chan,__destruct){
	 
	//printf("Chan::__destruct\n");
	
	zval* self = getThis();
	zval* chan = phpgo_zend_read_property(ce_go_chan_ptr, self, "handle", sizeof("handle")-1, true TSRMLS_CC);
	
	if(!chan)
		RETURN_FALSE;
	
	auto lchan = Z_LVAL_P(chan);
	if(lchan){
		GoChan::Destroy((void*)lchan);
	}
 }
 /* }}} */
 
   /* {{{ proto Selector::__construct
  * Create a selector object, it won't be called by the PHP code
  * it's called and resulting object returned by the select()
  */
 PHP_METHOD(Selector,__construct){
	long selector = 0;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &selector) == FAILURE) {
		zend_error(E_ERROR, "phpgo: Selector::__construct: getting parameter failure");
		return;
	}
	
	GO_SELECTOR* sel = (GO_SELECTOR*)selector;
	
	return_value = getThis();
	zend_update_property_long(ce_go_selector_ptr, return_value, "handle", sizeof("handle")-1, selector TSRMLS_CC);
 }
 /* }}} */
 
 
 /* {{{ proto Selector::select
  * Do select and return a selector object (for a faster call next time)
  */
 PHP_METHOD(Selector, Select){
	zval* self = getThis();
	zval* z_selector = phpgo_zend_read_property(ce_go_selector_ptr, self, "handle", sizeof("handle")-1, true TSRMLS_CC);
	
	if(!z_selector || Z_TYPE_P(z_selector) == IS_NULL){
		zend_error(E_ERROR, "phpgo: Selector::select(): error reading object handle");
		return;
	}
	
	auto selector = (GO_SELECTOR*)Z_LVAL_P(z_selector);
	if( !selector ){
		zend_error(E_ERROR, "phpgo: Selector::select(): null object handle");
		return;
	}
	
	phpgo_select(selector->case_array, selector->case_count TSRMLS_CC);
	
	RETURN_ZVAL(self, 1, 0);
 }
 /* }}} */
 
 /* {{{ proto Selector::loop
  * Do select in a loop until the channel is written to or closed
  */
 PHP_METHOD(Selector, Loop){
	 
	zval* z_chan     = NULL;
	
	if( zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &z_chan) == FAILURE ){
        zend_error(E_ERROR, "phpgo: Selector::loop(): getting parameter failure");
		RETURN_FALSE;
    }
	
	zval* chan = phpgo_zend_read_property(ce_go_chan_ptr, z_chan, "handle", sizeof("handle")-1, true TSRMLS_CC);

	if(!chan || Z_TYPE_P(chan) == IS_NULL ){
		zend_error(E_ERROR, "phpgo: Selector::loop(): null channel handle");
		RETURN_FALSE;
	}
	
	auto lchan = Z_LVAL_P(chan);
	
	//--
	zval* self = getThis();
	zval* z_selector = phpgo_zend_read_property(ce_go_selector_ptr, self, "handle", sizeof("handle")-1, true TSRMLS_CC);
	
	if(!z_selector || Z_TYPE_P(z_selector) == IS_NULL){
		zend_error(E_ERROR, "phpgo: Selector::loop(): error reading object handle");
		RETURN_FALSE;
	}
	
	auto selector = (GO_SELECTOR*)Z_LVAL_P(z_selector);
	if( !selector ){
		zend_error(E_ERROR, "phpgo: Selector::loop(): null object handle");
		RETURN_FALSE;
	}
	
	// GoChan::TryPop will
	// - return nullptr if not ready
	// return ZVAL_NULL if closed
	// otherwise return data read
	
	zval* z = nullptr;
	while( !( z = GoChan::TryPop( (void*)lchan ) ) ){
		phpgo_select(selector->case_array, selector->case_count TSRMLS_CC);
	}
	
	phpgo_zval_add_ref(&z);
	RETURN_ZVAL(z, 1, 1);
 }
 /* }}} */
 
  /* {{{ proto Selector::__destruct
  * Destroy the selector
  */
 PHP_METHOD(Selector,__destruct){
	
	zval* self = getThis();
	zval* selector = phpgo_zend_read_property(ce_go_selector_ptr, self, "handle", sizeof("handle")-1, true TSRMLS_CC);
	
	if(!selector)
		RETURN_FALSE;
	
	auto lselector = Z_LVAL_P(selector);

	if(lselector){
		GO_SELECTOR* sel = (GO_SELECTOR*)lselector;
		
		auto case_count = sel->case_count;
		auto case_array = sel->case_array;

		for(auto i = 0; i < case_count; i++){
			if( case_array[i].chan /*&& 
			    Z_TYPE_P(case_array[i].chan) != IS_NULL*/ ) {
				phpgo_zval_ptr_dtor(&case_array[i].chan);
				PHPGO_FREE_PZVAL(case_array[i].chan);
			}
			
			if( case_array[i].value /*&& 
			    Z_TYPE_P(case_array[i].value) != IS_NULL*/ ) {
				phpgo_zval_ptr_dtor(&case_array[i].value);
				PHPGO_FREE_PZVAL(case_array[i].value);
			}

			if( case_array[i].callback /*&& 
			    Z_TYPE_P(case_array[i].callback) != IS_NULL*/) {
				phpgo_zval_ptr_dtor(&case_array[i].callback);
				PHPGO_FREE_PZVAL(case_array[i].callback);
			}
		}
		
		efree(sel->case_array);
		efree(sel);
	}
 }
 /* }}} */
 
/* {{{ proto int go( callable $func, ...$args )
 * run the $func as in a go routine in the current thread context, 
 * do not wait the function to complete
 * returns:
 * 		true 
 */
PHP_FUNCTION(go)
{
	int argc = ZEND_NUM_ARGS();
	if(argc < 1){
		zend_error(E_ERROR, "phpgo: go(): callable missing in parameter list");
        RETURN_FALSE;
	}

    PHPGO_ARG_TYPE* args = NULL;
	args = (PHPGO_ARG_TYPE*)safe_emalloc(argc, sizeof(PHPGO_ARG_TYPE), 0);
	defer{
		if(args) efree(args);
	};
	
    if( zend_get_parameters_array_ex(argc, args) == FAILURE ){
		zend_error(E_ERROR, "phpgo: go(): error getting parameters");
        RETURN_FALSE;
    }

	FUNC_NAME_TYPE func_name = NULL;
	defer{
		if(func_name) FREE_FUNC_NAME(func_name);
	};
	
    if(!zend_is_callable( PHPGO_ARG_TO_PZVAL(args[0]), 0, &func_name TSRMLS_CC)){
        zend_error(E_ERROR, "phpgo: go(): function '%s' is not callable", func_name);
        RETURN_FALSE;
    }

	if( argc > 1 && Z_TYPE_P( PHPGO_ARG_TO_PZVAL(args[1]) ) != IS_ARRAY ){
		zend_error(E_ERROR, "phpgo: go(): parameter 2 expected to be an array");
        RETURN_FALSE;
	}
	
	bool ok = phpgo_go( GoRoutineOptions::gro_default, 0/*stack_size*/, argc, args TSRMLS_CC);
	
	RETURN_BOOL( ok);
}

/* {{{ proto int go( array $options, callable $func, ...$args )
 * run the $func as in a go routine in the current thread context, 
 * do not wait the function to complete
 * the available options are:
 * 		$options['stack_size']: 
 * 			provide a customized stack size to the go routine
 * 			if the provided stack size is less than 32K bytes,	it will	be round 
 * 			up to 32K bytes. 
 * 			Default is 1024K bytes if not provided
 * 			Note: the stack size provide a max size of the stack can reach, the 
 * 			underlying mechanism increases stack bit by bit thus a large stack 
 * 			size does not necessarily mean a waste of memory
 *		$options['isolate_http_globals']
 *			denotes whether the http "super globals" - $_GET, $_POST ... etc should
 *			be isolated from the go routine's parent
 *			if ture: the super globals will be copied from parent on the go routine
 *			creation, and then both sets of super globals are maintained seperately,
 *			i.e, change of super golbals in go routine does not change those of the 
 *			parent (can be a go routine or the scheduler) ans vise versa
 *			if false: the super globals are shared by the go routine and it's parent
 * returns:
 * 		true 
 */
PHP_FUNCTION(goo)
{
	bool isolate_http_globals = false;
	uint32_t stack_size = 0;
	
	int argc = ZEND_NUM_ARGS();
	if(argc < 2){
		zend_error(E_ERROR, "phpgo: goo(): at least 2 parameters required");
        RETURN_FALSE;
	}
	
    PHPGO_ARG_TYPE* args = NULL;
	args = (PHPGO_ARG_TYPE*)safe_emalloc(argc, sizeof(PHPGO_ARG_TYPE), 0);
	defer{
		if(args) efree(args);
	};
    
    if ( zend_get_parameters_array_ex(argc, args) == FAILURE) {
		zend_error(E_ERROR, "phpgo: goo(): error getting parameters");
        RETURN_FALSE;
    }

	zval* arr_optoins = PHP5_VS_7(*args[0], &args[0]);
	if( Z_TYPE_P(arr_optoins) != IS_ARRAY ){
		zend_error(E_ERROR, "phpgo: goo(): options must be array");
        RETURN_FALSE;
	}
	
	void* data = NULL;
	#define zval_data PHP5_VS_7( **(zval**)data, *(zval*)data )
	if( phpgo_zend_hash_find(HASH_OF(arr_optoins), "stack_size", sizeof("stack_size"), (void**)&data) == SUCCESS ){
		zval var = zval_data;
		zval_copy_ctor(&var);
		convert_to_long(&var);
		stack_size = Z_LVAL(var);
	}
	
	if( phpgo_zend_hash_find(HASH_OF(arr_optoins), "isolate_http_globals", sizeof("isolate_http_globals"), (void**)&data) == SUCCESS ){
		zval var = zval_data;
		zval_copy_ctor(&var);
		convert_to_boolean(&var);
		isolate_http_globals = Z_BVAL(var);
	}

    FUNC_NAME_TYPE func_name = NULL;
	defer{
		if(func_name) FREE_FUNC_NAME(func_name);
	};
	
	zval* callable = PHP5_VS_7(*args[1], &args[1]);
	if (!zend_is_callable(callable, 0, &func_name TSRMLS_CC))
    {
        zend_error(E_ERROR, "phpgo: go(): function '%s' is not callable", func_name);
        RETURN_FALSE;
    }

	uint64_t options = isolate_http_globals ? GoRoutineOptions::gro_isolate_http_globals : GoRoutineOptions::gro_default;
	bool ok = phpgo_go( options, stack_size, argc - 1, &args[1] TSRMLS_CC);
	
	RETURN_BOOL( ok );
}

/* {{{ proto void go_debug($flag)
 *   set debug flag, -1 for all
 */
PHP_FUNCTION( go_debug )
{
	long debug_flag;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &debug_flag) == FAILURE)
    {
        php_printf("go: getting parameter failure");
		return;
    }
	phpgo_go_debug(debug_flag);
}
/* }}} */

/* {{{ proto void select()
 * do go-style select
 */
PHP_FUNCTION(select)
{
	#define zvalp_data PHP5_VS_7( *(zval**)data, (zval*)data )
	
	#define GO_SELECT_FREE_RESOURCE() \
	do { \
		if(args) efree(args); \
		if(case_array) efree(case_array); \
		if(func_name) FREE_FUNC_NAME(func_name); \
	}while(0)
		
	#define ENSURE_SUFFICIENT_PARAMETERS() \
	do{ \
		if( pointer==PHPGO_INVALID_HASH_POSITION ){ \
			zend_error(E_ERROR, "phpgo: select(): case %d: insufficient parameter count", i+1); \
			goto error_return; \
		} \
	}while(0);
	
	//in case where operator (-> or <-) and value are omitted
	//default behaviour is to read the value into a temporary zval
	#define GO_TO_EXTRACT_CALLABLE_IF_EARLY_CALLABLE() \
	do{ \
		FUNC_NAME_TYPE func_name = NULL; \
		defer{ if(func_name) FREE_FUNC_NAME(func_name); }; \
		if( zend_is_callable(zvalp_data, 0, &func_name TSRMLS_CC) ){ \
			op = GO_CASE_OP_READ; \
			PHPGO_ALLOC_INIT_ZVAL(value); \
			goto extract_callable; \
		} \
	}while(0)
	
	GO_SELECT_CASE* case_array = NULL; 
	GO_SELECTOR* selector      = NULL;
	zend_uchar type            = IS_NULL; 
	bool exec                  = true;
	zval* z_selector           = NULL;
	zval* arg1                 = NULL;
	FUNC_NAME_TYPE func_name   = NULL;
	int argc                   = ZEND_NUM_ARGS(); 
	int case_count             = argc;
	
	if( argc < 1 ){
		zend_error(E_ERROR, "phpgo: select(): require at least 1 parameter");
        RETURN_FALSE;
	}
	
    PHPGO_ARG_TYPE *args = NULL;
	args = (PHPGO_ARG_TYPE*)safe_emalloc(argc, sizeof(PHPGO_ARG_TYPE), 0);

    if ( zend_get_parameters_array_ex(argc, args) == FAILURE ){
		zend_error(E_ERROR, "phpgo: select(): error getting parameters");
        goto error_return;
    }

	#define ARGN PHP5_VS_7(*args[argc-1], &args[argc-1])
	/*the last argument == false, return the case array rather than execute the select*/
	if( PHPGO_ZVAL_IS_BOOL(ARGN) ){
		exec = Z_BVAL_P( ARGN );
		--case_count;
	}
	
	if(!case_count){
		zend_error(E_ERROR, "phpgo: select(): at least one case or default switch or selector required");
        goto error_return;
	}
	
	case_array = (GO_SELECT_CASE*)safe_emalloc(case_count, sizeof(GO_SELECT_CASE), 0);
	
	for(int i = 0; i < case_count; i++){
		void*        data = nullptr;
		HashTable*   ht;
		HashPosition pointer = 0; //hash position is pointer prior to PHP7 and is a uint32_t after
		zval*        chan = NULL;
		long         op = 0, case_type = 0; 
		zval*        callback = NULL; 
		zval*        value = NULL;
		zval*        ch = NULL;
		zval*        v = NULL;
		char*        op_str = NULL;
		
		defer{
			if(chan)     phpgo_zval_ptr_dtor(&chan);
			if(value)    phpgo_zval_ptr_dtor(&value);
			if(callback) phpgo_zval_ptr_dtor(&callback);
		};

		zval* z = PHP5_VS_7(*args[i], &args[i]);
		if( Z_TYPE_P(z) != IS_ARRAY ){
			zend_error(E_ERROR, "parameter %d to select() expected to be an array", i+1);
			goto error_return;
		}
			
		ht = Z_ARRVAL_P(z);
		zend_hash_internal_pointer_reset_ex(ht, &pointer); 
		ENSURE_SUFFICIENT_PARAMETERS();
		
		// #1 'case' / 'default'
		if( phpgo_zend_hash_get_current_data_ex(ht, (void**) &data, &pointer) != SUCCESS){
			zend_error(E_ERROR, "phpgo: select(): error getting data of parameter %d", i+1);
			goto error_return;
		};
		
		if( Z_TYPE_P( zvalp_data ) != IS_STRING ){
			zend_error(E_ERROR, "phpgo: select(): case %d: invalid case type, expect 'case' or 'default'", i+1);
			goto error_return;
		}

		auto case_type_str = Z_STRVAL_P( zvalp_data ); 
		if( !strcasecmp( case_type_str, "case" ) ){
			case_type = GO_CASE_TYPE_CASE;
		}else if ( !strcasecmp( case_type_str, "default" ) ){
			case_type = GO_CASE_TYPE_DEFAULT;
		}else{
			zend_error(E_ERROR, "phpgo: select(): case %d: invalid case type %s", i+1, case_type_str);
			goto error_return;
		}
		
		zend_hash_move_forward_ex(ht, &pointer);
		ENSURE_SUFFICIENT_PARAMETERS();

		if( case_type == GO_CASE_TYPE_DEFAULT ){
			op = 0;
			PHPGO_ALLOC_INIT_ZVAL(chan);
			PHPGO_ALLOC_INIT_ZVAL(value);
			goto extract_callable;
		}
		
		//#2 chan
		if( phpgo_zend_hash_get_current_data_ex(ht, (void**) &data, &pointer) != SUCCESS){
			zend_error(E_ERROR, "phpgo: select(): error getting parameter %d data", i+1);
			goto error_return;
		};
		if( Z_TYPE_P( zvalp_data ) != IS_OBJECT ){
			zend_error(E_ERROR, "phpgo: select(): case %d: invalid parameter type, object of go\\Chan expected", i+1);
			goto error_return;
		}
		ch = zvalp_data;
		PHPGO_ALLOC_INIT_ZVAL(chan);
		PHPGO_MAKE_COPY_ZVAL(&ch,chan);

		zend_hash_move_forward_ex(ht, &pointer);
		ENSURE_SUFFICIENT_PARAMETERS();
		
		// #3 op: "->" "<-"
		if( phpgo_zend_hash_get_current_data_ex(ht, (void**) &data, &pointer) != SUCCESS){
			zend_error(E_ERROR, "phpgo: select(): error getting parameter %d data", i+1);
			goto error_return;
		};
		
		if( Z_TYPE_P( zvalp_data ) != IS_STRING ){
			GO_TO_EXTRACT_CALLABLE_IF_EARLY_CALLABLE();
			zend_error(E_ERROR, "phpgo: select(): case %d: invalid operation type, expect '<-' or '->'", i+1);
			goto error_return;
		}
		op_str = Z_STRVAL_P( zvalp_data ); 
		if( !strcmp(op_str, "->") ){
			op = GO_CASE_OP_READ;
		}else if( !strcmp(op_str, "<-") ){
			op = GO_CASE_OP_WRITE; 
		}else{
			zend_error(E_ERROR, "phpgo: select(): case %d: invalid operation type %s", i+1, op_str);
			goto error_return;
		}

		zend_hash_move_forward_ex(ht, &pointer);
		ENSURE_SUFFICIENT_PARAMETERS();
		
		//#4 value
		if( phpgo_zend_hash_get_current_data_ex(ht, (void**) &data, &pointer) != SUCCESS){
			zend_error(E_ERROR, "phpgo: select(): error getting parameter %d data", i+1);
			goto error_return;
		};
		
		v = zvalp_data; 
		if(op == GO_CASE_OP_READ){
			// '->' followed by a callable, the value is ommited
			GO_TO_EXTRACT_CALLABLE_IF_EARLY_CALLABLE();
			
			if( !PZVAL_IS_REF(v) ){
				zend_error(E_ERROR, "phpgo: select(): case %d: variable must be reference if not ommited", i+1);
				goto error_return;
			}
#if PHP_MAJOR_VERSION < 7
			value = v;
#else
			PHPGO_ALLOC_INIT_ZVAL(value);
			PHPGO_MAKE_COPY_ZVAL(&v,value);
#endif
			phpgo_zval_add_ref(&v);
		}else{
			PHPGO_ALLOC_INIT_ZVAL(value);
			PHPGO_MAKE_COPY_ZVAL(&v,value);
		}

		zend_hash_move_forward_ex(ht, &pointer);
		ENSURE_SUFFICIENT_PARAMETERS();

	extract_callable:
		//#5 callable
		if( phpgo_zend_hash_get_current_data_ex(ht, (void**) &data, &pointer) != SUCCESS){
			zend_error(E_ERROR, "phpgo: select(): error getting parameter %d data", i+1);
			goto error_return;
		};
		if(!zend_is_callable( zvalp_data, 0, &func_name TSRMLS_CC)){
			zend_error(E_ERROR, "phpgo: case %d: function '%s' is not callable", i+1, func_name);
			goto error_return;
		}
		zval* cb = zvalp_data; 
		PHPGO_ALLOC_INIT_ZVAL(callback);
		PHPGO_MAKE_COPY_ZVAL(&cb,callback);
		
		zend_hash_move_forward_ex(ht, &pointer);
		if( pointer != PHPGO_INVALID_HASH_POSITION ){
			zend_error(E_WARNING, "phpgo: select(): case %d: unexpected extra parameter detected after the callable, ommited", i+1);
			//goto error_return;
		}
		
		case_array[i].case_type = case_type;
		
		//need to addref the chan, value and callback, since they will be dtor'ed 
		//by the defer{}
		phpgo_zval_add_ref(&chan);
		case_array[i].chan = chan;
		case_array[i].op = op;

		phpgo_zval_add_ref(&value);
		case_array[i].value = value; 

		phpgo_zval_add_ref(&callback);
		case_array[i].callback = callback; 
	}
	
	if( exec )
		phpgo_select(case_array, case_count TSRMLS_CC);
	
	selector = (GO_SELECTOR*)safe_emalloc(1, sizeof(GO_SELECTOR), 0);
	selector->case_count = case_count;
	selector->case_array = case_array;
	
	//todo: php7: there may have mem leak here: 
	//the make std zval allocated memory but not freed...
    PHPGO_MAKE_STD_ZVAL(z_selector);
    object_init_ex(z_selector, ce_go_selector_ptr);
	
    PHPGO_MAKE_STD_ZVAL(arg1);
	ZVAL_LONG(arg1, (long)selector);
	
	zend_call_method_with_1_params(
         PHP5_VS_7(&z_selector,z_selector),
         ce_go_selector_ptr,
		 &ce_go_selector_ptr->constructor,
		 "__construct",
		 NULL,
		 arg1
	);

	phpgo_zval_ptr_dtor(&arg1);
	
	efree(args);
	
	//copy z_selector value to return value, call copy ctor on return value,
	//call dtor on z_selector
	RETURN_ZVAL(z_selector, 1, 1);
	
error_return:	
	GO_SELECT_FREE_RESOURCE();
	RETURN_FALSE;
}
/* }}} */

  /* {{{ proto Mutex::__construct
  * Create a mutex
  */
PHP_METHOD(Mutex,__construct){
	//printf("Mutex::__construct\n");
	
	bool signaled = true; 
	if( ZEND_NUM_ARGS() ){
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "b", &signaled) == FAILURE) {
			zend_error(E_ERROR, "phpgo: Chan::__construct: getting parameter failure");
			return;
		}
	}
	
	void* mutex = GoMutex::Create(signaled);
	return_value = getThis();
	zend_update_property_long(ce_go_mutex_ptr, return_value, "handle", sizeof("handle")-1,   (long)mutex TSRMLS_CC);	
 }
 /* }}} */
 
  /* {{{ proto Mutex::lock
  * lock mutex. 
  * If currently not in a coroutine and the lock cannot be obtained, the scheduler 
  * will be executed in 10ms interval until the lock is released by the go-routines
  * or other threads
  */
 PHP_METHOD(Mutex,Lock){
	 
	//printf("Mutex::Lock\n");
	
	zval* mutex     = NULL;
	zval* z         = NULL;
	
	auto self = getThis();
	mutex = phpgo_zend_read_property(ce_go_mutex_ptr, self, "handle", sizeof("handle")-1, true TSRMLS_CC);

	if(!mutex)
		RETURN_FALSE;
	
	auto lmutex = Z_LVAL_P(mutex);
	GoMutex::Lock((void*)lmutex);
	
	RETURN_TRUE;
 }
 /* }}} */
 
 /* {{{ proto Mutex::unlock
  * unlock mutex
  */
 PHP_METHOD(Mutex,Unlock){
	 
	//printf("Mutex::Unlock\n");
	
	zval* mutex     = NULL;
	zval* z        = NULL;
	
	auto self = getThis();
	mutex = phpgo_zend_read_property(ce_go_mutex_ptr, self, "handle", sizeof("handle")-1, true TSRMLS_CC);

	if(!mutex)
		RETURN_FALSE;
	
	auto lmutex = Z_LVAL_P(mutex);
	GoMutex::Unlock((void*)lmutex);
	
	RETURN_TRUE;
 }
 /* }}} */
 
 /* {{{ proto Mutex::tryLock
  * try to lock a mutex, if lock held by other go routine,
  * return false immediately
  * return true if lock obtained
  */
 PHP_METHOD(Mutex,TryLock){
	zval* mutex     = NULL;
	zval* z         = NULL;
	
	auto self = getThis();
	mutex = phpgo_zend_read_property(ce_go_mutex_ptr, self, "handle", sizeof("handle")-1, true TSRMLS_CC);

	if(!mutex)
		RETURN_FALSE;
	
	auto lmutex = Z_LVAL_P(mutex);
	
	//printf("Mutex::TryLock %p\n", lmutex);
	RETURN_BOOL( GoMutex::TryLock((void*)lmutex) );
 }
 /* }}} */
 
 /* {{{ proto Mutex::isLock
  * check if the mutex is held by other go routines
  */
 PHP_METHOD(Mutex,IsLock){
	 
	//printf("Mutex::IsLock\n");
	
	zval* mutex     = NULL;
	zval* z         = NULL;
	
	auto self = getThis();
	mutex = phpgo_zend_read_property(ce_go_mutex_ptr, self, "handle", sizeof("handle")-1, true TSRMLS_CC);

	if(!mutex)
		RETURN_FALSE;
	
	auto lmutex = Z_LVAL_P(mutex);
	RETURN_BOOL( GoMutex::IsLock((void*)lmutex) );
 }
 /* }}} */
 
 
 /* {{{ proto Mutex::__destruct
  * Destruct the mutex
  */
 PHP_METHOD(Mutex,__destruct){
	 
	//printf("Mutex::__destruct\n");
	
	zval* self = getThis();
	zval* mutex = phpgo_zend_read_property(ce_go_mutex_ptr, self, "handle", sizeof("handle")-1, true TSRMLS_CC);
	
	if(!mutex)
		RETURN_FALSE;
	
	auto lmutex = Z_LVAL_P(mutex);
	GoMutex::Destroy( (void*)lmutex );
 }
 /* }}} */


  /* {{{ proto WaitGroup::__construct
  * Create a WaitGroup
  */
PHP_METHOD(WaitGroup,__construct){
	//printf("WaitGroup::__construct\n");
	void* wg = GoWaitGroup::Create();
	return_value = getThis();
	zend_update_property_long(ce_go_wait_group_ptr, return_value, "handle", sizeof("handle")-1,   (long)wg TSRMLS_CC);	
 }
 /* }}} */
 
  /* {{{ proto WaitGroup::add
  * Add a delta count to wait group counter
  */
 PHP_METHOD(WaitGroup,Add){
 	//printf("WaitGroup::Add\n");
	
	zval* wg       = NULL;
	int64_t delta  = 1;
	
	if( zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", (long*)&delta) == FAILURE ){
        zend_error(E_ERROR, "phpgo: WaitGroup::add: getting parameter failure");
		RETURN_FALSE;
    }
	
	auto self = getThis();
	wg = phpgo_zend_read_property(ce_go_wait_group_ptr, self, "handle", sizeof("handle")-1, true TSRMLS_CC);

	if(!wg)
		RETURN_FALSE;
	
	auto l_wg = Z_LVAL_P(wg);
	int64_t count = GoWaitGroup::Add( (void*)l_wg, delta );
	
	RETURN_LONG(count);
 }
  /* }}} */
 
  /* {{{ proto WaitGroup::done
  * Decrease the wait group counter by 1, alias of add(-1)
  */
 PHP_METHOD(WaitGroup,Done){
 	//printf("WaitGroup::Done\n");
	
	zval* wg       = NULL;

	auto self = getThis();
	wg = phpgo_zend_read_property(ce_go_wait_group_ptr, self, "handle", sizeof("handle")-1, true TSRMLS_CC);

	if(!wg)
		RETURN_FALSE;
	
	auto l_wg = Z_LVAL_P(wg);
	int64_t count = GoWaitGroup::Done( (void*)l_wg );
	
	RETURN_LONG(count);
 }
  /* }}} */ 
  
  /* {{{ proto WaitGroup::count
  * return the current counter of the wait group
  */
 PHP_METHOD(WaitGroup,Count){
 	//printf("WaitGroup::Count\n");
	
	zval* wg       = NULL;

	auto self = getThis();
	wg = phpgo_zend_read_property(ce_go_wait_group_ptr, self, "handle", sizeof("handle")-1, true TSRMLS_CC);

	if(!wg)
		RETURN_FALSE;
	
	auto l_wg = Z_LVAL_P(wg);
	int64_t count = GoWaitGroup::Count( (void*)l_wg );
	
	RETURN_LONG(count);
 }
  /* }}} */ 
  
 /* {{{ proto WaitGroup::wait
  * Wait for the wait group counter to reach 0
  * The wait() function can be used either in or out of a go routine
  */
 PHP_METHOD(WaitGroup,Wait){
 	//printf("WaitGroup::Wait\n");
	
	zval* wg       = NULL;

	auto self = getThis();
	wg = phpgo_zend_read_property(ce_go_wait_group_ptr, self, "handle", sizeof("handle")-1, true TSRMLS_CC);

	if(!wg)
		RETURN_FALSE;
	
	auto l_wg = Z_LVAL_P(wg);
	GoWaitGroup::Wait( (void*)l_wg );
	
	RETURN_TRUE;
 }
  /* }}} */ 
  
  /* {{{ proto WaitGroup::__destruct
  * Destruct the WaitGroup
  */
 PHP_METHOD(WaitGroup,__destruct){
	 
	//printf("WaitGroup::__destruct\n");
	
	zval* self = getThis();
	zval* wg = phpgo_zend_read_property(ce_go_wait_group_ptr, self, "handle", sizeof("handle")-1, true TSRMLS_CC);
	
	if(!wg)
		RETURN_FALSE;
	
	auto l_wg = Z_LVAL_P(wg);
	GoWaitGroup::Destruct( (void*)l_wg );
 }
 /* }}} */
 
/* {{{ proto void Scheduler::run()
 * run the secheduler for one pass
 * return the number of go routines awaiting for the scheduler to schedule
 */
PHP_METHOD(Scheduler, Run)
{
	auto run_task_count = GoScheduler::Run();
	
	RETURN_LONG(run_task_count);
}
/* }}} */

/* {{{ proto void Scheduler::join($tasks_left)
 *   run the secheduler until the un-completed go routine number 
 *   be less than or equal to $tasks_left
 */
PHP_METHOD(Scheduler, Join)
{
	uint64_t tasks_left = 0; 
	if( ZEND_NUM_ARGS() ){
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &tasks_left) == FAILURE) {
			zend_error(E_ERROR, "phpgo: Scheduler::join: getting parameter failure");
			return;
		}
	}
	
	while( phpgo_go_runtime_num_goroutine() > 
	       (uint32_t)tasks_left + PHPGO_G(running_internal_go_routines)
	){
	    GoScheduler::Run();
	}
	
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto void Scheduler::loop(void)
   loop running the secheduler forever*/
PHP_METHOD(Scheduler, Loop)
{
	GoScheduler::Loop();
	RETURN_TRUE;
}
/* }}} */


/* {{{ proto void Runtime::numGoroutine()
 *  return the number of go routines left to run under the secheuler
 */
PHP_METHOD(Runtime, NumGoroutine)
{
	RETURN_LONG( phpgo_go_runtime_num_goroutine() );
}
/* }}} */

/* {{{ proto void Runtime::gosched(void)
 *  run the scheduler explicitly
 *  if invoked in a go routine, the cpu is yield to other go routines
 *  if invoked by the scheduler, it's equivilant to Scheduler::run()
 */
PHP_METHOD(Runtime, Gosched)
{
	phpgo_go_runtime_gosched();
}
/* }}} */

/* {{{ proto void go_schedule_forever(void)
   loop running the secheduler forever*/
PHP_METHOD(Runtime, Goid)
{
	RETURN_LONG( phpgo_go_runtime_goid() );
}
/* }}} */

/* {{{ */
#if PHP_MAJOR_VERSION < 7
	#define __CREATE_CHANNEL_CALL_METHOD \
		zend_call_method_with_1_params(&z_chan, ce_go_chan_ptr, \
		&ce_go_chan_ptr->constructor,"__construct", NULL, arg1); 
#else
	#define __CREATE_CHANNEL_CALL_METHOD \
		zend_call_method_with_1_params(z_chan, ce_go_chan_ptr, \
		&ce_go_chan_ptr->constructor,"__construct", NULL, arg1); 
#endif

#define CREATE_CHANNEL(chan_name, z_chan) \
static THREAD_LOCAL uint32_t sid = 0; \
zval *arg1 = nullptr; \
do{\
	PHPGO_MAKE_STD_ZVAL(z_chan); \
    object_init_ex(z_chan, ce_go_chan_ptr); \
\
	PHPGO_MAKE_STD_ZVAL(arg1); \
	array_init(arg1); \
\
	sprintf(chan_name, "timer_%d_%d", clock(), sid++); \
	phpgo_add_assoc_string(arg1, "name", chan_name, 1); \
\
	add_assoc_long_ex(arg1, "capacity", PHP5_VS_7(sizeof("capacity"),sizeof("capacity")-1), 1 );  \
	add_assoc_bool_ex(arg1, "copy", PHP5_VS_7(sizeof("copy"), sizeof("copy")-1), 1 ); \
\
	__CREATE_CHANNEL_CALL_METHOD \
	phpgo_zval_ptr_dtor(&arg1); \
}while(0)
/* }}} */

/* {{{ proto Chan Time::tick( $nanoseconds )
 * start a periodic timer with interval $nanoseconds micro seconds
 * once timer expires, an integer 1 is written to the channel as returned
 * by this function
 */
PHP_METHOD(Time, Tick)
{
	//printf("Time::Tick\n");
	
	uint64_t nanoseconds  = 0;
	if( zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", (long*)&nanoseconds) == FAILURE ){
        zend_error(E_ERROR, "phpgo: Time::tick: getting parameter failure");
		RETURN_NULL();
    }
	
	char chan_name[32]; zval* z_chan = nullptr;
	CREATE_CHANNEL(chan_name, z_chan);
	
	bool go_creation = false;
	if( !GoTime::CreateTimer(chan_name, nanoseconds, true, go_creation) ){
		phpgo_zval_ptr_dtor(&z_chan);
		RETURN_NULL();
	}
	if( go_creation ){
		++PHPGO_G(running_internal_go_routines);
	}
	
	RETURN_ZVAL(z_chan, 1, 1);
}
/* }}} */

/* {{{ proto Chan Time::after($nanoseconds)
 * start a one-time timer which will expire $nanoseconds later
 * once timer expires, an integer 1 is written to the channel as returned
 * by this function
 */
PHP_METHOD(Time, After)
{
	//printf("Time::After\n");

	uint64_t nanoseconds  = 0;
	if( zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", (long*)&nanoseconds) == FAILURE ){
        zend_error(E_ERROR, "phpgo: Time::After: getting parameter failure");
		RETURN_NULL();
    }

	char chan_name[32]; zval* z_chan = nullptr;
	CREATE_CHANNEL(chan_name, z_chan);
	
	bool go_creation = false;
	if( !GoTime::CreateTimer(chan_name, nanoseconds, false, go_creation) ){
		phpgo_zval_ptr_dtor(&z_chan);
		RETURN_NULL();
	}
	if( go_creation ){
		++PHPGO_G(running_internal_go_routines);
	}
	
	RETURN_ZVAL(z_chan, 1, 1);
}
/* }}} */

/* {{{ proto Chan Time::Sleep($nanoseconds)
 * start a one-time timer which will expire $nanoseconds later
 * once timer expires, an integer 1 is written to the channel as returned
 * by this function
 */
PHP_METHOD(Time, Sleep)
{
	//printf("Time::Sleep\n");

	uint64_t nanoseconds  = 0;
	if( zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", (long*)&nanoseconds) == FAILURE ){
        zend_error(E_ERROR, "phpgo: Time::Sleep: getting parameter failure");
		RETURN_NULL();
    }
	
	GoTime::Sleep(nanoseconds);
	
	RETURN_BOOL(true);
}
/* }}} */
