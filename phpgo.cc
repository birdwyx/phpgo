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

#include "php_phpgo.h"
#include "go.h"
#include "go_scheduler.h"
#include "go_chan.h"
#include "go_mutex.h"
#include "go_runtime.h"
#include "go_wait_group.h"
#include "go_timer.h"
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
zend_class_entry  ce_go_timer,     *ce_go_timer_ptr;
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

ZEND_BEGIN_ARG_INFO_EX(arginfo_go_timer_tick, 0, 0, 1)
	ZEND_ARG_INFO(0, micro_seconds)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_go_timer_after, 0, 0, 1)
	ZEND_ARG_INFO(0, micro_seconds)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_go_selector_loop, 0, 0, 1)
	ZEND_ARG_INFO(0, done_chan)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_go__case, 0, 0, 4)
	ZEND_ARG_INFO(0, chan)
	ZEND_ARG_INFO(0, rw)
	ZEND_ARG_INFO(1, var)
	ZEND_ARG_INFO(0, callback)
ZEND_END_ARG_INFO()
/* }}} */

/* {{{ phpgo_functions[]
 *
 * Every user visible function must have an entry in phpgo_functions[].
 */
const zend_function_entry phpgo_functions[] = {
	//PHP_FE(confirm_phpgo_compiled,	NULL)		/* For testing, remove later. */
	PHP_FE(go, NULL)
	PHP_FE(go_debug, NULL)
	//ZEND_NS_NAMED_FE(PHPGO_NS, go_debug, ZEND_FN(go_go_debug), NULL)
	
	PHP_FE(select, NULL)
	PHP_FE(_case, arginfo_go__case)
	PHP_FE(_default, NULL)
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
	PHP_ME(Scheduler,      RunOnce,      NULL,                      ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Scheduler,      RunJoinAll,   NULL,                      ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Scheduler,      RunForever,   NULL,                      ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_FE_END	/* Must be the last line */
};

const zend_function_entry go_selector_methods[] = {
	PHP_ME(Selector,       __construct,  arginfo_go_selector_ctor,  ZEND_ACC_PUBLIC|ZEND_ACC_CTOR  )
	PHP_ME(Selector,       Select,       NULL,                      ZEND_ACC_PUBLIC                )
	PHP_ME(Selector,       Loop,         arginfo_go_selector_loop,  ZEND_ACC_PUBLIC                )
	PHP_ME(Selector,       __destruct,   NULL,                      ZEND_ACC_PUBLIC|ZEND_ACC_DTOR  )
	PHP_FE_END	/* Must be the last line */
};

const zend_function_entry go_timer_methods[] = {
	PHP_ME(Timer,          Tick,        arginfo_go_timer_tick,      ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Timer,          After,       arginfo_go_timer_after,     ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_FE_END	/* Must be the last line */
};

const zend_function_entry go_runtime_methods[] = {
	PHP_ME(Runtime,        NumGoroutine, NULL,                     ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Runtime,        Gosched,      NULL,                     ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
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
	INIT_NS_CLASS_ENTRY(ce_go_timer,     PHPGO_NS, "Timer",     go_timer_methods); 
	INIT_NS_CLASS_ENTRY(ce_go_runtime,   PHPGO_NS, "Runtime",   go_runtime_methods); 
	
	ce_go_chan_ptr      = zend_register_internal_class(&ce_go_chan TSRMLS_CC);
	ce_go_mutex_ptr     = zend_register_internal_class(&ce_go_mutex TSRMLS_CC);
	ce_go_wait_group_ptr= zend_register_internal_class(&ce_go_wait_group TSRMLS_CC);
	ce_go_scheduler_ptr = zend_register_internal_class(&ce_go_scheduler TSRMLS_CC);
	ce_go_selector_ptr  = zend_register_internal_class(&ce_go_selector TSRMLS_CC);
	ce_go_timer_ptr     = zend_register_internal_class(&ce_go_timer TSRMLS_CC);
	ce_go_runtime_ptr   = zend_register_internal_class(&ce_go_runtime TSRMLS_CC);
    
	//zend_declare_property_long(ce_go_chan_ptr,"handle",  strlen("handle"),  -1, ZEND_ACC_PUBLIC TSRMLS_CC);
	//zend_declare_property_long(ce_go_chan_ptr,"capacity",strlen("capacity"), 0, ZEND_ACC_PUBLIC TSRMLS_CC);
	
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
	
	/*
	if(z3){
		if( !z1 || Z_TYPE_P(z1) != IS_LONG   || 
			!z2 || Z_TYPE_P(z2) != IS_STRING ||
			       Z_TYPE_P(z3) != IS_BOOL 
		){
			zend_error(E_ERROR, "phpgo: Chan($capacity, $name, $copy): the 3 parameters must be of type long, string, bool respectively");
			RETURN_NULL();
		}
		copy     = Z_BVAL_P(z3);
		capacity = Z_LVAL_P(z1);
		name     = Z_STRVAL_P(z2);
		name_len = Z_STRLEN_P(z2);
	}else if(z2){
		if( !z1 || Z_TYPE_P(z1) != IS_LONG || 
		           Z_TYPE_P(z2) != IS_STRING
		){
			zend_error(E_ERROR, "phpgo: Chan($capacity, $name): the 2 parameters must be of type long, string respectively");
			RETURN_NULL();
		}
		capacity = Z_LVAL_P(z1);
		name     = Z_STRVAL_P(z2);
		name_len = Z_STRLEN_P(z2);
		if( name_len <= 0 ){
			zend_error(E_ERROR, "phpgo: Chan($capacity, $name): the channel name string, when provided, cannot be empty");
			RETURN_NULL();
		}
	}else if(z1){
		if( Z_TYPE_P(z1) == IS_LONG ){
			capacity = Z_LVAL_P(z1);
		}else if( Z_TYPE_P(z1) == IS_STRING ){
			name     = Z_STRVAL_P(z1);
			name_len = Z_STRLEN_P(z1);
			if( name_len <= 0 ){
				zend_error(E_ERROR, "phpgo: Chan($capacity | $name): the channel name string, when provided, cannot be empty");
				RETURN_NULL();
			}
		}else{
			zend_error(E_ERROR, "phpgo: Chan($capacity | $name): the parameter must be either long or string");
			RETURN_NULL();
		}
	}else{
		// no parameter, use default
	}*/
	
	if( z1 ){
	    if( Z_TYPE_P(z1) == IS_LONG ){
			capacity = Z_LVAL_P(z1);
		}else if( Z_TYPE_P(z1) == IS_ARRAY && z1->value.ht){
			zval** ppz = NULL;
			if( zend_hash_find(HASH_OF(z1), "name", sizeof("name"), (void**)&ppz) == SUCCESS ){
				if(Z_TYPE_P(*ppz) != IS_STRING){
					zend_error(E_ERROR, "phpgo: Chan( $options ): option \"name\" must be string");
					RETURN_NULL();
				}
				name     = Z_STRVAL_P(*ppz);
				name_len = Z_STRLEN_P(*ppz);
			}
			if( zend_hash_find(HASH_OF(z1), "capacity", sizeof("capacity"), (void**)&ppz) == SUCCESS ){
				if(Z_TYPE_P(*ppz) != IS_LONG){
					zend_error(E_ERROR, "phpgo: Chan( $options ): option \"capacity\" must be long");
					RETURN_NULL();
				}
				capacity     = Z_LVAL_P(*ppz);
			}
			if( zend_hash_find(HASH_OF(z1), "copy", sizeof("copy"), (void**)&ppz) == SUCCESS ){
				if(Z_TYPE_P(*ppz) != IS_BOOL){
					zend_error(E_ERROR, "phpgo: Chan( $options ): option \"copy\" must be bool");
					RETURN_NULL();
				}
				copy     = Z_BVAL_P(*ppz);
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
	zend_update_property_long  (ce_go_chan_ptr, return_value, "handle", sizeof("handle")-1,   (long)chan    TSRMLS_CC);
	zend_update_property_string(ce_go_chan_ptr, return_value, "name",     sizeof("name")-1,     name?name:""  TSRMLS_CC);
	zend_update_property_long  (ce_go_chan_ptr, return_value, "capacity", sizeof("capacity")-1, capacity      TSRMLS_CC);
	zend_update_property_bool  (ce_go_chan_ptr, return_value, "copy",     sizeof("copy")-1,     copy          TSRMLS_CC);
 }
 /* }}} */
 
  /* {{{ proto Chan::__construct
  * Create a go channel object
  */
 PHP_METHOD(Chan,Push){
	 
	//printf("Chan::Push\n");
	
	zval* chan     = NULL;
	zval* z        = NULL;
	
	if( zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &z) == FAILURE ){
        zend_error(E_ERROR, "phpgo: Chan::Push: getting parameter failure");
		RETURN_FALSE;
    }
	
	auto self = getThis();
	chan = zend_read_property(ce_go_chan_ptr, self, "handle", sizeof("handle")-1, true TSRMLS_CC);

	if(!chan)
		RETURN_FALSE;
	
	auto lchan = Z_LVAL_P(chan);
	void* ch = GoChan::Push( (void*)lchan, z TSRMLS_CC);
	
	//printf("chan::push %p\n", ch);
	
	RETURN_TRUE;
	
 }
 /* }}} */
 
   /* {{{ proto Chan::__construct
  * Create a go channel object
  */
 PHP_METHOD(Chan,TryPush){
	 
	//printf("Chan::TryPush\n");
	
	zval* chan     = NULL;
	zval* z        = NULL;
	
	if( zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &z) == FAILURE ){
        zend_error(E_ERROR, "phpgo: Chan::Push: getting parameter failure");
		RETURN_FALSE;
    }
	
	auto self = getThis();
	chan = zend_read_property(ce_go_chan_ptr, self, "handle", sizeof("handle")-1, true TSRMLS_CC);

	if(!chan)
		RETURN_FALSE;
	
	auto lchan = Z_LVAL_P(chan);
	auto ok = GoChan::TryPush( (void*)lchan, z TSRMLS_CC);
	
	RETURN_BOOL(ok);
	
 }
 /* }}} */
 
 
 /* {{{ proto Chan::__construct
  * Create a go channel object
  */
 PHP_METHOD(Chan,Pop){
	 
	//printf("Chan::Pop\n");
	
	zval* self = getThis();
	zval* chan = zend_read_property(ce_go_chan_ptr, self, "handle", sizeof("handle")-1, true TSRMLS_CC);

	if(!chan)
		RETURN_FALSE;
	
	auto lchan = Z_LVAL_P(chan);	
	zval* z = GoChan::Pop( (void*)lchan );
	
	if(!z)
		RETURN_NULL();
	
	RETURN_ZVAL(z, 1, 1);
 }
 /* }}} */
 
  /* {{{ proto Chan::__construct
  * Create a go channel object
  */
 PHP_METHOD(Chan,TryPop){
	 
	//printf("Chan::TryPop\n");
	
	zval* self = getThis();
	zval* chan = zend_read_property(ce_go_chan_ptr, self, "handle", sizeof("handle")-1, true TSRMLS_CC);

	if(!chan)
		RETURN_FALSE;
	
	auto lchan = Z_LVAL_P(chan);	
	zval* z = GoChan::TryPop( (void*)lchan );
	
	// phpgo_go_chan_try_pop will
	// return nullptr if not ready
	// return ZVAL_NULL if closed
	// otherwise return data read
	
	// TryPop return false if channel not ready
	if(!z)
		RETURN_FALSE;
	
	RETURN_ZVAL(z, 1, 1);
 }
 /* }}} */
 
 
  /* {{{ proto Chan::__construct
  * Create a go channel object
  */
 PHP_METHOD(Chan,Close){
	 
	//printf("Chan::Close\n");
	
	zval* self = getThis();
	zval* chan = zend_read_property(ce_go_chan_ptr, self, "handle", sizeof("handle")-1, true TSRMLS_CC);

	if(!chan)
		RETURN_FALSE;
	
	auto lchan = Z_LVAL_P(chan);	
	GoChan::Close( (void*)lchan );
	
 }
 /* }}} */
 
 
 /* {{{ proto Chan::__destruct
  * Create a go channel object
  */
 PHP_METHOD(Chan,__destruct){
	 
	//printf("Chan::__destruct\n");
	
	zval* self = getThis();
	zval* chan = zend_read_property(ce_go_chan_ptr, self, "handle", sizeof("handle")-1, true TSRMLS_CC);
	
	if(!chan)
		RETURN_FALSE;
	
	auto lchan = Z_LVAL_P(chan);
	if(lchan){
		GoChan::Destroy((void*)lchan);
	}
 }
 /* }}} */
 
   /* {{{ proto Selector::__destruct
  * Create a go channel object
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
 
 
 /* {{{ proto Selector::__destruct
  * Create a go channel object
  */
 PHP_METHOD(Selector, Select){
	zval* self = getThis();
	zval* z_selector = zend_read_property(ce_go_selector_ptr, self, "handle", sizeof("handle")-1, true TSRMLS_CC);
	
	if(!z_selector || Z_TYPE_P(z_selector) == IS_NULL){
		zend_error(E_ERROR, "phpgo: Selector::Select(): error reading object handle");
		return;
	}
	
	auto selector = (GO_SELECTOR*)Z_LVAL_P(z_selector);
	if( !selector ){
		zend_error(E_ERROR, "phpgo: Selector::Select(): null object handle");
		return;
	}
	
	phpgo_select(selector->case_array, selector->case_count TSRMLS_CC);
	
	RETURN_ZVAL(self, 1, 0);
 }
 /* }}} */
 
     /* {{{ proto Selector::__destruct
  * Create a go channel object
  */
 PHP_METHOD(Selector, Loop){
	 
	zval* z_chan     = NULL;
	
	if( zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &z_chan) == FAILURE ){
        zend_error(E_ERROR, "phpgo: Selector::Loop(): getting parameter failure");
		RETURN_FALSE;
    }
	
	zval* chan = zend_read_property(ce_go_chan_ptr, z_chan, "handle", sizeof("handle")-1, true TSRMLS_CC);

	if(!chan || Z_TYPE_P(chan) == IS_NULL ){
		zend_error(E_ERROR, "phpgo: Selector::Loop(): null channel handle");
		RETURN_FALSE;
	}
	
	auto lchan = Z_LVAL_P(chan);
	
	//--
	zval* self = getThis();
	zval* z_selector = zend_read_property(ce_go_selector_ptr, self, "handle", sizeof("handle")-1, true TSRMLS_CC);
	
	if(!z_selector || Z_TYPE_P(z_selector) == IS_NULL){
		zend_error(E_ERROR, "phpgo: Selector::Loop(): error reading object handle");
		RETURN_FALSE;
	}
	
	auto selector = (GO_SELECTOR*)Z_LVAL_P(z_selector);
	if( !selector ){
		zend_error(E_ERROR, "phpgo: Selector::Loop(): null object handle");
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
	
	zval_add_ref(&z);
	RETURN_ZVAL(z, 1, 1);
 }
 /* }}} */
 
  /* {{{ proto Selector::__destruct
  * Create a go channel object
  */
 PHP_METHOD(Selector,__destruct){
	
	zval* self = getThis();
	zval* selector = zend_read_property(ce_go_selector_ptr, self, "handle", sizeof("handle")-1, true TSRMLS_CC);
	
	if(!selector)
		RETURN_FALSE;
	
	auto lselector = Z_LVAL_P(selector);

	if(lselector){
		GO_SELECTOR* sel = (GO_SELECTOR*)lselector;
		
		auto case_count = sel->case_count;
		auto case_array = sel->case_array;

		for(auto i = 0; i < case_count; i++){
			if( case_array[i].chan && 
			    Z_TYPE_P(case_array[i].chan) != IS_NULL ) {
				zval_ptr_dtor(&case_array[i].chan);
			}
			
			if( case_array[i].value && 
			    Z_TYPE_P(case_array[i].value) != IS_NULL ) {
				zval_ptr_dtor(&case_array[i].value);
			}

			if( case_array[i].callback && 
			    Z_TYPE_P(case_array[i].callback) != IS_NULL) {
				zval_ptr_dtor(&case_array[i].callback);
			}
		}
		
		efree(sel->case_array);
		efree(sel);
	}
 }
 /* }}} */
 
/* {{{ proto int go( callable $func )
   run the $func as go routine in the current thread context, and 
   do not wait for the function complete
   Return true */
PHP_FUNCTION(go)
{
	int argc = ZEND_NUM_ARGS();
	if(argc < 1){
		zend_error(E_ERROR, "phpgo: go(): callable missing in parameter list");
        RETURN_FALSE;
	}
	
    zval ***args = (zval ***)safe_emalloc(argc, sizeof(zval **), 0);
    if ( zend_get_parameters_array_ex(argc, args) == FAILURE) {
        efree(args);
		zend_error(E_ERROR, "phpgo: go(): error getting parameters");
        RETURN_FALSE;
    }

    char *func_name = NULL;
    if (!zend_is_callable(*args[0], 0, &func_name TSRMLS_CC))
    {
        php_error_docref(NULL TSRMLS_CC,E_ERROR, "phpgo: go(): function '%s' is not callable", func_name);
        efree(func_name);
		efree(args);
        RETURN_FALSE;
    }
    efree(func_name);
	
	void* co = phpgo_go( argc, args TSRMLS_CC);
	
	efree(args);
	RETURN_LONG( (long)co );
}

/* {{{ proto void go_schedule_forever(void)
   loop running the secheduler forever*/
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

/* {{{ proto mixed _case(mixed $ch, string $rw, mixed& $value, callable $callback)
   construct a case for the select()
   $ch:       the channel object obtained by go_chan_create()
   $rw:       to read or write the channel, can be "->" (read) or "<-" (write)
   $value:    the reference of the value to be written to / read from the channel
   $callback: when the $value is is successfully read from/written to the channel, 
              the callback will be invoked with the data read/written, i.e., 
			  $callback($value) called
			  
   A go case must be used with select(), otherwise nothing will happen, i.e.,
   no any data read/write happens and callback won't be called 
*/
PHP_FUNCTION(_case)
{
	#define GO_CASE_FREE_RESOURCE() \
		do { /*if(op) efree(op);*/ \
			 if(func_name) efree(func_name); \
		}while(0)
	
	zval* chan;
	zval* callback;
	zval* value;
	char* op = NULL; 
	int   op_len; 
	char* func_name = NULL; 
	long  op_i = 0;
	
	defer{
		GO_CASE_FREE_RESOURCE();
	};

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zszz", &chan, &op, &op_len, &value, &callback ) == FAILURE)
    {
        zend_error(E_ERROR, "phpgo: getting parameter failure");
		return;
    }
	
	if( strcmp(op, "->") ==0 ){
		op_i = GO_CASE_OP_READ;
	}else if( strcmp(op, "<-") ==0 ){
		op_i = GO_CASE_OP_WRITE;
	}
	else{
		zend_error(E_ERROR, "phpgo: invalid channel operation %s", op);
		return;
	}
	
    if (!zend_is_callable(callback, 0, &func_name TSRMLS_CC)){
        zend_error(E_ERROR, "phpgo: function '%s' is not callable", func_name);
        return;
    }
	
	array_init(return_value);
	add_index_long(return_value, 0, GO_CASE_TYPE_CASE);
	
	zval* ch;
	ALLOC_INIT_ZVAL(ch);
	MAKE_COPY_ZVAL(&chan,ch);
	//zval_add_ref(&chan);
	add_next_index_zval(return_value, ch);
	
	add_next_index_long(return_value, op_i);
	
	if(op_i == GO_CASE_OP_READ){
		zval_add_ref(&value);
		add_next_index_zval(return_value, value);
	}else{
		zval* v;
		ALLOC_INIT_ZVAL(v);
		MAKE_COPY_ZVAL(&value,v);
		add_next_index_zval(return_value, v);
	}
	
	zval* cb;
	ALLOC_INIT_ZVAL(cb);
	MAKE_COPY_ZVAL(&callback,cb);
	add_next_index_zval(return_value, cb);
}
/* }}} */

/* {{{ proto void go_schedule_forever(void)
   loop running the secheduler forever*/
PHP_FUNCTION(_default)
{
	#define GO_CASE_FREE_RESOURCE() \
		do { \
			 if(func_name) efree(func_name); \
		}while(0)

	zval* callback;
	char* func_name = NULL; 

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &callback ) == FAILURE)
    {
        zend_error(E_ERROR, "phpgo: getting parameter failure");
		goto error_return;
    }
	
    if (!zend_is_callable(callback, 0, &func_name TSRMLS_CC)){
        zend_error(E_ERROR, "phpgo: function '%s' is not callable", func_name);
        goto error_return;
    }
	
	array_init(return_value);
	add_index_long(return_value, 0, GO_CASE_TYPE_DEFAULT);
	add_next_index_long(return_value, 0);
    add_next_index_long(return_value, 0);
	
	zval* zval_null;
	ALLOC_INIT_ZVAL(zval_null);
	add_next_index_zval(return_value, zval_null);
	
	zval* cb;
	ALLOC_INIT_ZVAL(cb);
	MAKE_COPY_ZVAL(&callback,cb);
	add_next_index_zval(return_value, cb);
	
	GO_CASE_FREE_RESOURCE();
	return;
	
error_return:
	
	GO_CASE_FREE_RESOURCE();
	RETURN_FALSE;
	//void dump_zval(zval* zv);
	//dump_zval(callback);
}
/* }}} */

/* {{{ proto void go_schedule_forever(void)
   loop running the secheduler forever*/
PHP_FUNCTION(select)
{
	#define GO_SELECT_FREE_RESOURCE() \
	do { \
		if(args) efree(args); \
		if(case_array) efree(case_array); \
	}while(0)
	
	GO_SELECT_CASE* case_array = NULL; 
	GO_SELECTOR* selector      = NULL;
	zend_uchar type            = IS_NULL; 
	bool exec                  = true;
	zval* z_selector           = NULL;
	zval* arg1                 = NULL;
			
	int argc                   = ZEND_NUM_ARGS(); 
	int case_count             = argc;
	
	if( argc < 1 ){
		zend_error(E_ERROR, "phpgo: select(): require at least 1 parameter");
        RETURN_FALSE;
	}
	
    zval ***args = (zval ***)safe_emalloc(argc, sizeof(zval **), 0);
    if ( zend_get_parameters_array_ex(argc, args) == FAILURE) {
		zend_error(E_ERROR, "phpgo: select(): error getting parameters");
        goto error_return;
    }
	
	/*the last argument == false, return the case array rather than execute the select*/
	type =  Z_TYPE_P( *args[argc-1] );
	if( type == IS_BOOL ){
		exec = Z_BVAL_P(*args[argc-1]);
		--case_count;
	}
	
	if(!case_count){
		zend_error(E_ERROR, "phpgo: select(): at least one case or default switch or selector required");
        goto error_return;
	}
	
	case_array = (GO_SELECT_CASE*)safe_emalloc(case_count, sizeof(GO_SELECT_CASE), 0);
	
	for(int i = 0; i < case_count; i++){
		zval**       data;
		HashTable*   ht;
		HashPosition pointer;
		zval*        chan = NULL;
		long         op = 0, case_type = 0; 
		zval*        callback = NULL; 
		zval*        value = NULL;
		
		if( Z_TYPE_P(*args[i]) != IS_ARRAY ){
			zend_error(E_ERROR, "Parameter %d to select() expected to be an array", i+1);
			goto error_return;
		}
			
		ht = Z_ARRVAL_P(*args[i]);
		zend_hash_internal_pointer_reset_ex(ht, &pointer); 
		
		if( zend_hash_get_current_data_ex(ht, (void**) &data, &pointer) != SUCCESS){
			zend_error(E_ERROR, "phpgo: select(): error getting data of parameter %d", i + 1);
			goto error_return;
		};
		case_type = Z_LVAL_P(*data); 
		
		zend_hash_move_forward_ex(ht, &pointer);
		
		if( zend_hash_get_current_data_ex(ht, (void**) &data, &pointer) != SUCCESS){
			zend_error(E_ERROR, "phpgo: select(): error getting parameter %d data", i + 1);
			goto error_return;
		};
		//chan = Z_LVAL_P(*data); 
		chan = *data;
		zend_hash_move_forward_ex(ht, &pointer);
		
		if( zend_hash_get_current_data_ex(ht, (void**) &data, &pointer) != SUCCESS){
			zend_error(E_ERROR, "phpgo: select(): error getting parameter %d data", i + 1);
			goto error_return;
		}; 
		op = Z_LVAL_P(*data); 
		zend_hash_move_forward_ex(ht, &pointer);
		
		if( zend_hash_get_current_data_ex(ht, (void**) &data, &pointer) != SUCCESS){
			zend_error(E_ERROR, "phpgo: select(): error getting parameter %d data", i + 1);
			goto error_return;
		}; 
		value = *data; 
		zend_hash_move_forward_ex(ht, &pointer);
		
		if( zend_hash_get_current_data_ex(ht, (void**) &data, &pointer) != SUCCESS){
			zend_error(E_ERROR, "phpgo: select(): error getting parameter %d data", i + 1);
			goto error_return;
		};
		callback = *data; 
		
		case_array[i].case_type = case_type;
		
		zval_add_ref(&chan);
		case_array[i].chan = chan;
		case_array[i].op = op;
		
		//need to addref the value and callback, since they will be dtor'ed 
		//on the hash table (ht) destruction during this funciton return
		zval_add_ref(&value);
		case_array[i].value = value; 
		zval_add_ref(&callback);
		case_array[i].callback = callback; 
		
		//printf("go select: conver go_cases into c array: callback:\n");
		//extern void dump_zval(zval*);
		//dump_zval(callback);
		
		
		//printf( "select case_type: %ld, chan: %ld op: %d, value %p, callback: %p\n" , case_type, chan, op, value, callback);
		
	}
	
	if( exec )
		phpgo_select(case_array, case_count TSRMLS_CC);
	
	selector = (GO_SELECTOR*)safe_emalloc(1, sizeof(GO_SELECTOR), 0);
	selector->case_count = case_count;
	selector->case_array = case_array;
	
    MAKE_STD_ZVAL(z_selector);
    object_init_ex(z_selector, ce_go_selector_ptr);
	
	MAKE_STD_ZVAL(arg1);
	ZVAL_LONG(arg1, (long)selector);
	
	//printf("select, about to call Selector constructor selector: %p\n", selector);
	zend_call_method_with_1_params(&z_selector, ce_go_selector_ptr, &ce_go_selector_ptr->constructor, "__construct", NULL, arg1);
	zval_ptr_dtor(&arg1);
	
	//extern void dump_zval(zval*);
	//printf("select, after call Selector constructor:\n");
	//dump_zval(selector->case_array[0].callback); 
	
	efree(args);
	
	//copy z_selector value to return value, call copy ctor on return value,
	//call dtor on z_selector
	RETURN_ZVAL(z_selector, 1, 1);
	//RETURN_LONG((long)selector);
	
error_return:	
	GO_SELECT_FREE_RESOURCE();
	RETURN_FALSE;
	
	//RETURN_ZVAL(z, 1, 0);
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
 
  /* {{{ proto Mutex::Lock
  * Lock mutex. 
  * If currently not in a coroutine and the lock cannot be obtained, the scheduler 
  * will be executed in 10ms interval until the lock is released by the go-routines
  * or other threads
  */
 PHP_METHOD(Mutex,Lock){
	 
	//printf("Mutex::Lock\n");
	
	zval* mutex     = NULL;
	zval* z         = NULL;
	
	auto self = getThis();
	mutex = zend_read_property(ce_go_mutex_ptr, self, "handle", sizeof("handle")-1, true TSRMLS_CC);

	if(!mutex)
		RETURN_FALSE;
	
	auto lmutex = Z_LVAL_P(mutex);
	GoMutex::Lock((void*)lmutex);
	
	RETURN_TRUE;
 }
 /* }}} */
 
 /* {{{ proto Mutex::Unlock
  * Unlock mutex
  */
 PHP_METHOD(Mutex,Unlock){
	 
	//printf("Mutex::Unlock\n");
	
	zval* mutex     = NULL;
	zval* z        = NULL;
	
	auto self = getThis();
	mutex = zend_read_property(ce_go_mutex_ptr, self, "handle", sizeof("handle")-1, true TSRMLS_CC);

	if(!mutex)
		RETURN_FALSE;
	
	auto lmutex = Z_LVAL_P(mutex);
	GoMutex::Unlock((void*)lmutex);
	
	RETURN_TRUE;
 }
 /* }}} */
 
  /* {{{ proto Mutex::TryLock
  * Unlock mutex
  */
 PHP_METHOD(Mutex,TryLock){
	zval* mutex     = NULL;
	zval* z         = NULL;
	
	auto self = getThis();
	mutex = zend_read_property(ce_go_mutex_ptr, self, "handle", sizeof("handle")-1, true TSRMLS_CC);

	if(!mutex)
		RETURN_FALSE;
	
	auto lmutex = Z_LVAL_P(mutex);
	
	//printf("Mutex::TryLock %p\n", lmutex);
	RETURN_BOOL( GoMutex::TryLock((void*)lmutex) );
 }
 /* }}} */
 
  /* {{{ proto Mutex::Unlock
  * Unlock mutex
  */
 PHP_METHOD(Mutex,IsLock){
	 
	//printf("Mutex::IsLock\n");
	
	zval* mutex     = NULL;
	zval* z         = NULL;
	
	auto self = getThis();
	mutex = zend_read_property(ce_go_mutex_ptr, self, "handle", sizeof("handle")-1, true TSRMLS_CC);

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
	zval* mutex = zend_read_property(ce_go_mutex_ptr, self, "handle", sizeof("handle")-1, true TSRMLS_CC);
	
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
 
  /* {{{ proto WaitGroup::__construct
  * Create a WaitGroup
  */
 PHP_METHOD(WaitGroup,Add){
 	//printf("WaitGroup::Add\n");
	
	zval* wg       = NULL;
	int64_t delta  = 1;
	
	if( zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", (long*)&delta) == FAILURE ){
        zend_error(E_ERROR, "phpgo: WaitGroup::Add: getting parameter failure");
		RETURN_FALSE;
    }
	
	auto self = getThis();
	wg = zend_read_property(ce_go_wait_group_ptr, self, "handle", sizeof("handle")-1, true TSRMLS_CC);

	if(!wg)
		RETURN_FALSE;
	
	auto l_wg = Z_LVAL_P(wg);
	int64_t count = GoWaitGroup::Add( (void*)l_wg, delta );
	
	RETURN_LONG(count);
 }
  /* }}} */
 
  /* {{{ proto WaitGroup::__construct
  * Create a WaitGroup
  */
 PHP_METHOD(WaitGroup,Done){
 	//printf("WaitGroup::Done\n");
	
	zval* wg       = NULL;

	auto self = getThis();
	wg = zend_read_property(ce_go_wait_group_ptr, self, "handle", sizeof("handle")-1, true TSRMLS_CC);

	if(!wg)
		RETURN_FALSE;
	
	auto l_wg = Z_LVAL_P(wg);
	int64_t count = GoWaitGroup::Done( (void*)l_wg );
	
	RETURN_LONG(count);
 }
  /* }}} */ 
  
  /* {{{ proto WaitGroup::__construct
  * Create a WaitGroup
  */
 PHP_METHOD(WaitGroup,Count){
 	//printf("WaitGroup::Count\n");
	
	zval* wg       = NULL;

	auto self = getThis();
	wg = zend_read_property(ce_go_wait_group_ptr, self, "handle", sizeof("handle")-1, true TSRMLS_CC);

	if(!wg)
		RETURN_FALSE;
	
	auto l_wg = Z_LVAL_P(wg);
	int64_t count = GoWaitGroup::Count( (void*)l_wg );
	
	RETURN_LONG(count);
 }
  /* }}} */ 
  
 /* {{{ proto WaitGroup::__construct
  * Create a WaitGroup
  */
 PHP_METHOD(WaitGroup,Wait){
 	//printf("WaitGroup::Wait\n");
	
	zval* wg       = NULL;

	auto self = getThis();
	wg = zend_read_property(ce_go_wait_group_ptr, self, "handle", sizeof("handle")-1, true TSRMLS_CC);

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
	zval* wg = zend_read_property(ce_go_wait_group_ptr, self, "handle", sizeof("handle")-1, true TSRMLS_CC);
	
	if(!wg)
		RETURN_FALSE;
	
	auto l_wg = Z_LVAL_P(wg);
	GoWaitGroup::Destruct( (void*)l_wg );
 }
 /* }}} */
 
 /* {{{ proto void go_schedule_once(void)
   run the secheduler for a one pass*/
PHP_METHOD(Scheduler, RunOnce)
{
	GoScheduler::RunOnce();
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto void go_schedule_all(void)
   run the secheduler until all go routines completed*/
PHP_METHOD(Scheduler, RunJoinAll)
{
	GoScheduler::RunJoinAll();
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto void go_schedule_forever(void)
   loop running the secheduler forever*/
PHP_METHOD(Scheduler, RunForever)
{
	GoScheduler::RunForever();
	RETURN_TRUE;
}
/* }}} */


/* {{{ proto void go_schedule_forever(void)
   loop running the secheduler forever*/
PHP_METHOD(Runtime, NumGoroutine)
{
	RETURN_LONG( phpgo_go_runtime_num_goroutine() );
}
/* }}} */

/* {{{ proto void go_schedule_forever(void)
   loop running the secheduler forever*/
PHP_METHOD(Runtime, Gosched)
{
	phpgo_go_runtime_gosched();
}
/* }}} */

/* {{{ proto void go_schedule_forever(void)
   loop running the secheduler forever*/
PHP_METHOD(Timer, Tick)
{
	//printf("Timer::Tick\n");
	
	uint64_t micro_seconds  = 0;
	if( zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", (long*)&micro_seconds) == FAILURE ){
        zend_error(E_ERROR, "phpgo: Timer::Tick: getting parameter failure");
		RETURN_NULL();
    }
	
	zval* z_chan = nullptr;
	MAKE_STD_ZVAL(z_chan);
    object_init_ex(z_chan, ce_go_chan_ptr);

	zval *arg1 = nullptr;
	MAKE_STD_ZVAL(arg1);
	ZVAL_LONG(arg1, 1);
	
	//printf("select, about to call Selector constructor selector: %p\n", selector);
	zend_call_method_with_1_params(&z_chan, ce_go_chan_ptr, &ce_go_chan_ptr->constructor, "__construct", NULL, arg1);
	zval_ptr_dtor(&arg1);
	
	auto z_handler = zend_read_property(ce_go_chan_ptr, z_chan, "handle", sizeof("handle")-1, true TSRMLS_CC);
	
	if(!z_handler)
		RETURN_NULL();
	
	void* chan = (void*)Z_LVAL_P(z_handler);
	if(!chan)
		RETURN_NULL();
	
	GoTimer::Tick(z_chan, chan, micro_seconds TSRMLS_CC);
	
	RETURN_ZVAL(z_chan, 1, 1);
}
/* }}} */


/* {{{ proto void go_schedule_forever(void)
   loop running the secheduler forever*/
PHP_METHOD(Timer, After)
{
	//printf("Timer::After\n");
	
	uint64_t micro_seconds  = 0;
	if( zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", (long*)&micro_seconds) == FAILURE ){
        zend_error(E_ERROR, "phpgo: Timer::Tick: getting parameter failure");
		RETURN_NULL();
    }
	
	zval* z_chan = nullptr;
	MAKE_STD_ZVAL(z_chan);
    object_init_ex(z_chan, ce_go_chan_ptr);

	zval *arg1 = nullptr;
	MAKE_STD_ZVAL(arg1);
	ZVAL_LONG(arg1, 1);
	
	//printf("select, about to call Selector constructor selector: %p\n", selector);
	zend_call_method_with_1_params(&z_chan, ce_go_chan_ptr, &ce_go_chan_ptr->constructor, "__construct", NULL, arg1);
	zval_ptr_dtor(&arg1);
	
	auto z_handler = zend_read_property(ce_go_chan_ptr, z_chan, "handle", sizeof("handle")-1, true TSRMLS_CC);
	
	if(!z_handler)
		RETURN_NULL();
	
	void* chan = (void*)Z_LVAL_P(z_handler);
	if(!chan)
		RETURN_NULL();
	
	GoTimer::After(z_chan, chan, micro_seconds TSRMLS_CC);
	
	RETURN_ZVAL(z_chan, 1, 1);
}
/* }}} */
