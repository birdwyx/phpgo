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
#include "php_phpgo.h"

#include "go.h"
#include "mutex.h"
#include "zend_interfaces.h"

#include <signal.h>       /* for signal */  
#include <execinfo.h>     /* for backtrace() */  
  
#define BACKTRACE_SIZE   16  
  
void dump(void)  
{  
    int j, nptrs;  
    void *buffer[BACKTRACE_SIZE];  
    char **strings;  
      
    nptrs = backtrace(buffer, BACKTRACE_SIZE);  
      
    printf("backtrace() returned %d addresses\n", nptrs);  
  
    strings = backtrace_symbols(buffer, nptrs);  
    if (strings == NULL) {  
        perror("backtrace_symbols");  
        exit(EXIT_FAILURE);  
    }  
  
    for (j = 0; j < nptrs; j++)  
        printf("  [%02d] %s\n", j, strings[j]);  
  
    free(strings);  
}  




/* If you declare any globals in php_phpgo.h uncomment this:
ZEND_DECLARE_MODULE_GLOBALS(phpgo)
*/

/* True global resources - no need for thread safety here */
static int le_phpgo;
static bool phpgo_initialized = false;

static int le_go_channel;
static int le_go_mutex;

#define le_go_channel_name "go channel"
#define le_go_mutex_name "go mutex"

zend_class_entry  ce_go_chan,      *ce_go_chan_ptr;
zend_class_entry  ce_go_mutex,     *ce_go_mutex_ptr;
zend_class_entry  ce_go_waitgroup, *ce_go_waitgroup_ptr;
zend_class_entry  ce_go_scheduler, *ce_go_scheduler_ptr;
zend_class_entry  ce_go_selector,  *ce_go_selector_ptr;
zend_class_entry  ce_go_timer,     *ce_go_timer_ptr;

/* {{{ phpgo_functions[]
 *
 * Every user visible function must have an entry in phpgo_functions[].
 */
const zend_function_entry phpgo_functions[] = {
	//PHP_FE(confirm_phpgo_compiled,	NULL)		/* For testing, remove later. */
	PHP_FE(go, NULL)
	PHP_FE(go_await, NULL)
	PHP_FE(go_schedule_once, NULL)
	PHP_FE(go_schedule_all, NULL)
	PHP_FE(go_schedule_forever, NULL)

	PHPGO_NS_FE(go, go_debug, NULL)
	//ZEND_NS_NAMED_FE(PHPGO_NS, go_debug, ZEND_FN(go_go_debug), NULL)
	
	PHP_FE(go_chan_create, NULL)
	PHP_FE(go_chan_close, NULL)
	PHP_FE(go_chan_push, NULL)
	PHP_FE(go_chan_pop, NULL)
	PHP_FE(select, NULL)
	PHP_FE(_case, NULL)
	PHP_FE(_default, NULL)
	PHP_FE(go_mutex_create, NULL)
	PHP_FE(go_mutex_destroy, NULL)
	PHP_FE(go_mutex_lock, NULL)
	PHP_FE(go_mutex_unlock, NULL)
	PHP_FE(go_mutex_try_lock, NULL)
	PHP_FE(go_mutex_is_lock, NULL)
	PHP_FE_END	/* Must be the last line in phpgo_functions[] */
};
/* }}} */

/* {{{ arginfo_go_chan_push[]
 *
 */
//ZEND_BEGIN_ARG_INFO_EX(arginfo_go_chan_push, pass_rest_by_reference, return_reference, required_num_args)
ZEND_BEGIN_ARG_INFO_EX(arginfo_go_chan_push, 0, 0, 1)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_go_waitgroup_add, 0, 0, 1)
	ZEND_ARG_INFO(0, delta)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_go_selector_ctor, 0, 0, 1)
	ZEND_ARG_INFO(0, handle)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_go_timer_tick, 0, 0, 1)
	ZEND_ARG_INFO(0, microseconds)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_go_timer_after, 0, 0, 1)
	ZEND_ARG_INFO(0, microseconds)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_go_selector_loop, 0, 0, 1)
	ZEND_ARG_INFO(0, done_chan)
ZEND_END_ARG_INFO()
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

const zend_function_entry go_waitgroup_methods[] = {
	PHP_ME(Waitgroup,      __construct,  NULL,                      ZEND_ACC_PUBLIC|ZEND_ACC_CTOR  )
	PHP_ME(Waitgroup,      Add,          arginfo_go_waitgroup_add,  ZEND_ACC_PUBLIC                )
	PHP_ME(Waitgroup,      Done,         NULL,                      ZEND_ACC_PUBLIC                )
	PHP_ME(Waitgroup,      Wait,         NULL,                      ZEND_ACC_PUBLIC                )
	PHP_ME(Waitgroup,      __destruct,   NULL,                      ZEND_ACC_PUBLIC|ZEND_ACC_DTOR  )
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
/* Uncomment this function if you have INI entries
static void php_phpgo_init_globals(zend_phpgo_globals *phpgo_globals)
{
	phpgo_globals->global_value = 0;
	phpgo_globals->global_string = NULL;
}
*/
/* }}} */

static void phpho_destructor_go_channel(zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
	//printf("phpho_destructor_go_channel\n");
	if(rsrc->ptr)
		php_api_go_chan_close((void*)rsrc->ptr);
}

static void  phpho_destructor_go_mutex(zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
	//printf("phpho_destructor_go_mutex\n");
	if(rsrc->ptr)
		delete ((::co::CoRecursiveMutex*)rsrc->ptr);
}
/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(phpgo)
{
	/* If you have INI entries, uncomment these lines 
	REGISTER_INI_ENTRIES();
	*/
	
	le_go_channel = zend_register_list_destructors_ex(phpho_destructor_go_channel, NULL, le_go_channel_name, module_number);
	le_go_mutex   = zend_register_list_destructors_ex(phpho_destructor_go_mutex,   NULL, le_go_mutex_name,   module_number);
	
	INIT_NS_CLASS_ENTRY(ce_go_chan,      PHPGO_NS, "Chan",      go_chan_methods);  // 类名为 go\Chan
	INIT_NS_CLASS_ENTRY(ce_go_mutex,     PHPGO_NS, "Mutex",     go_mutex_methods); 
	INIT_NS_CLASS_ENTRY(ce_go_waitgroup, PHPGO_NS, "Waitgroup", go_waitgroup_methods); 
	INIT_NS_CLASS_ENTRY(ce_go_scheduler, PHPGO_NS, "Scheduler", go_scheduler_methods); 
	INIT_NS_CLASS_ENTRY(ce_go_selector,  PHPGO_NS, "Selector",  go_selector_methods); 
	INIT_NS_CLASS_ENTRY(ce_go_timer,     PHPGO_NS, "Timer",     go_timer_methods); 
	
	ce_go_chan_ptr      = zend_register_internal_class(&ce_go_chan TSRMLS_CC);
	ce_go_mutex_ptr     = zend_register_internal_class(&ce_go_mutex TSRMLS_CC);
	ce_go_waitgroup_ptr = zend_register_internal_class(&ce_go_waitgroup TSRMLS_CC);
	ce_go_scheduler_ptr = zend_register_internal_class(&ce_go_scheduler TSRMLS_CC);
	ce_go_selector_ptr  = zend_register_internal_class(&ce_go_selector TSRMLS_CC);
	ce_go_timer_ptr     = zend_register_internal_class(&ce_go_timer TSRMLS_CC);
    
	//zend_declare_property_long(ce_go_chan_ptr,"handle",  strlen("handle"),  -1, ZEND_ACC_PUBLIC TSRMLS_CC);
	//zend_declare_property_long(ce_go_chan_ptr,"capacity",strlen("capacity"), 0, ZEND_ACC_PUBLIC TSRMLS_CC);

	if( !phpgo_initialized ){
		if(phpgo_initialize()){
			phpgo_initialized = true;
			return SUCCESS;
		}else{
			return FAILURE;
		}
	}
	
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
	 
	printf("Chan::__construct\n");
	
	long capacity = 0;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &capacity) == FAILURE) {
		zend_error(E_ERROR, "phpgo: Chan::__construct: getting parameter failure");
		return;
	}

	void* chan = php_api_go_chan_create(capacity);
	
	return_value = getThis();
	zend_update_property_long(ce_go_chan_ptr, return_value, "handle",   sizeof("handle"),   (long)chan TSRMLS_CC);
	zend_update_property_long(ce_go_chan_ptr, return_value, "capacity", sizeof("capacity"), capacity   TSRMLS_CC);
	
	/*
	char *fname = NULL, *mime = NULL, *postname = NULL;
	int fname_len, mime_len, postname_len;
	zval *cf = return_value;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|ss", &fname, &fname_len, &mime, &mime_len, &postname, &postname_len) == FAILURE) {
		return;
	}

	if (fname) {
		zend_update_property_string(curl_CURLFile_class, cf, "name", sizeof("name")-1, fname TSRMLS_CC);
	}

	if (mime) {
		zend_update_property_string(curl_CURLFile_class, cf, "mime", sizeof("mime")-1, mime TSRMLS_CC);
	}

	if (postname) {
		zend_update_property_string(curl_CURLFile_class, cf, "postname", sizeof("postname")-1, postname TSRMLS_CC);
	}*/
	
 }
 /* }}} */
 
  /* {{{ proto Chan::__construct
  * Create a go channel object
  */
 PHP_METHOD(Chan,Push){
	 
	printf("Chan::Push\n");
	
	zval* chan     = NULL;
	zval* z        = NULL;
	
	if( zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &z) == FAILURE ){
        zend_error(E_ERROR, "phpgo: Chan::Push: getting parameter failure");
		RETURN_FALSE;
    }
	
	auto self = getThis();
	chan = zend_read_property(ce_go_chan_ptr, self, "handle",   sizeof("handle"), true TSRMLS_CC);

	if(!chan)
		RETURN_FALSE;
	
	auto lchan = Z_LVAL_P(chan);
	void* ch = php_api_go_chan_push( (void*)lchan, z );
	
	printf("chan::push %p\n", ch);
	
	RETURN_TRUE;
	
 }
 /* }}} */
 
   /* {{{ proto Chan::__construct
  * Create a go channel object
  */
 PHP_METHOD(Chan,TryPush){
	 
	printf("Chan::TryPush\n");
	
	zval* chan     = NULL;
	zval* z        = NULL;
	
	if( zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &z) == FAILURE ){
        zend_error(E_ERROR, "phpgo: Chan::Push: getting parameter failure");
		RETURN_FALSE;
    }
	
	auto self = getThis();
	chan = zend_read_property(ce_go_chan_ptr, self, "handle",   sizeof("handle"), true TSRMLS_CC);

	if(!chan)
		RETURN_FALSE;
	
	auto lchan = Z_LVAL_P(chan);
	auto ok = php_api_go_chan_try_push( (void*)lchan, z );
	
	RETURN_BOOL(ok);
	
 }
 /* }}} */
 
 
 /* {{{ proto Chan::__construct
  * Create a go channel object
  */
 PHP_METHOD(Chan,Pop){
	 
	printf("Chan::Pop\n");
	
	zval* self = getThis();
	zval* chan = zend_read_property(ce_go_chan_ptr, self, "handle",   sizeof("handle"), true TSRMLS_CC);

	if(!chan)
		RETURN_FALSE;
	
	auto lchan = Z_LVAL_P(chan);	
	zval* z = php_api_go_chan_pop( (void*)lchan );
	
	if(!z)
		RETURN_NULL();
	
	RETURN_ZVAL(z, 1, 1);
 }
 /* }}} */
 
  /* {{{ proto Chan::__construct
  * Create a go channel object
  */
 PHP_METHOD(Chan,TryPop){
	 
	printf("Chan::TryPop\n");
	
	zval* self = getThis();
	zval* chan = zend_read_property(ce_go_chan_ptr, self, "handle",   sizeof("handle"), true TSRMLS_CC);

	if(!chan)
		RETURN_FALSE;
	
	auto lchan = Z_LVAL_P(chan);	
	zval* z = php_api_go_chan_try_pop( (void*)lchan );
	
	// php_api_go_chan_try_pop will
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
	 
	printf("Chan::Close\n");
	
	zval* self = getThis();
	zval* chan = zend_read_property(ce_go_chan_ptr, self, "handle",   sizeof("handle"), true TSRMLS_CC);

	if(!chan)
		RETURN_FALSE;
	
	auto lchan = Z_LVAL_P(chan);	
	php_api_go_chan_close( (void*)lchan );
	
 }
 /* }}} */
 
 
 /* {{{ proto Chan::__destruct
  * Create a go channel object
  */
 PHP_METHOD(Chan,__destruct){
	 
	printf("Chan::__destruct\n");
	
	zval* self = getThis();
	zval* chan = zend_read_property(ce_go_chan_ptr, self, "handle",   sizeof("handle"), true TSRMLS_CC);
	
	if(!chan)
		RETURN_FALSE;
	
	auto lchan = Z_LVAL_P(chan);
	if(lchan){
		php_api_go_chan_destroy((void*)lchan);
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
	printf("Selector::__construct selector=%ld, %p, %ld\n", sel, sel->case_array, sel->case_count);
	//printf("%d, %d, %d, %p, %p\n", sel->case_array[0].case_type, sel->case_array[0].chan, 
	//	       sel->case_array[0].op, sel->case_array[0].value, sel->case_array[0].callback);
	
    //extern void dump_zval(zval*);	
	//dump_zval(sel->case_array[0].callback);
	
	//printf("\n");
	
	return_value = getThis();
	zend_update_property_long(ce_go_selector_ptr, return_value, "handle",   sizeof("handle"), selector TSRMLS_CC);
 }
 /* }}} */
 
 
    /* {{{ proto Selector::__destruct
  * Create a go channel object
  */
 PHP_METHOD(Selector, Select){
	zval* self = return_value = getThis();
	zval* z_selector = zend_read_property(ce_go_selector_ptr, self, "handle",   sizeof("handle"), true TSRMLS_CC);
	
	if(!z_selector || Z_TYPE_P(z_selector) == IS_NULL){
		zend_error(E_ERROR, "phpgo: Selector::Select(): error reading object handle");
		return;
	}
	
	auto selector = (GO_SELECTOR*)Z_LVAL_P(z_selector);
	if( !selector ){
		zend_error(E_ERROR, "phpgo: Selector::Select(): null object handle");
		return;
	}
	
	php_api_go_select(selector->case_array, selector->case_count);
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
	
	zval* chan = zend_read_property(ce_go_chan_ptr, z_chan, "handle",   sizeof("handle"), true TSRMLS_CC);

	if(!chan || Z_TYPE_P(chan) == IS_NULL ){
		zend_error(E_ERROR, "phpgo: Selector::Loop(): null channel handle");
		RETURN_FALSE;
	}
	
	auto lchan = Z_LVAL_P(chan);
	
	//--
	zval* self = getThis();
	zval* z_selector = zend_read_property(ce_go_selector_ptr, self, "handle",   sizeof("handle"), true TSRMLS_CC);
	
	if(!z_selector || Z_TYPE_P(z_selector) == IS_NULL){
		zend_error(E_ERROR, "phpgo: Selector::Loop(): error reading object handle");
		RETURN_FALSE;
	}
	
	auto selector = (GO_SELECTOR*)Z_LVAL_P(z_selector);
	if( !selector ){
		zend_error(E_ERROR, "phpgo: Selector::Loop(): null object handle");
		RETURN_FALSE;
	}
	
	// php_api_go_chan_try_pop will
	// return nullptr if not ready
	// return ZVAL_NULL if closed
	// otherwise return data read
	
	zval* z = nullptr;
	while( !( z = php_api_go_chan_try_pop( (void*)lchan ) ) ){
		php_api_go_select(selector->case_array, selector->case_count);
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
	zval* selector = zend_read_property(ce_go_selector_ptr, self, "handle",   sizeof("handle"), true TSRMLS_CC);
	
	if(!selector)
		RETURN_FALSE;
	
	auto lselector = Z_LVAL_P(selector);
	
	printf("Selector::__destruct selector = %ld\n", lselector);
	if(lselector){
		GO_SELECTOR* sel = (GO_SELECTOR*)lselector;
		
		auto case_count = sel->case_count;
		auto case_array = sel->case_array;
		
		//printf("########C: %ld------A: %p-----------\n", case_count, case_array);
		for(auto i = 0; i < case_count; i++){
			
			//printf("the %d case:\n",i);
			//void dump_zval(zval*);
			//dump_zval(case_array[i].value);
			//dump_zval(case_array[i].callback);
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
		//printf("##########\n");
		
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
	
	void* co = php_api_go( argc, args TSRMLS_CC);
	
	efree(args);
	RETURN_LONG( (long)co );
}
/* }}} */


/* {{{ proto int go_await( callable $func )
   run the $func as go routine in the current thread context, and 
   wait for the function complete
   Return whatever $func returns */
PHP_FUNCTION(go_await)
{
	//    php_printf("this is on");
    zval *callback;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &callback) == FAILURE)
    {
        php_printf("go: getting parameter failure");
    }

    //判断callback是不是回调方法
    char *func_name = NULL;
    if (!zend_is_callable(callback, 0, &func_name TSRMLS_CC))
    {
        php_error_docref(NULL TSRMLS_CC,E_ERROR, "function '%s' is not callable", func_name);
        efree(func_name);
        RETURN_FALSE;
    }
    efree(func_name);
	
	zval* cb2;
	MAKE_STD_ZVAL(cb2);
    *cb2 = *callback;
	zval_copy_ctor(cb2);
	php_api_go_await(cb2, return_value);
	
	//php_api_go_await(callback, return_value);

}
/* }}} */

/* {{{ proto void go_schedule_once(void)
   run the secheduler for a one pass*/
PHP_FUNCTION(go_schedule_once)
{
	php_api_go_schedule_once();
}
/* }}} */

/* {{{ proto void go_schedule_all(void)
   run the secheduler until all go routines completed*/
PHP_FUNCTION(go_schedule_all)
{
	try{
		php_api_go_schedule_all();
	}catch (...)  
    {  
        php_printf( "exception\n" );  
    }  
	
	//php_printf("go_schedule_all,returns\n");
	//dump();
}
/* }}} */

/* {{{ proto void go_schedule_forever(void)
   loop running the secheduler forever*/
PHP_FUNCTION(go_schedule_forever)
{
	php_api_go_schedule_forever();
}
/* }}} */


/* {{{ proto void go_schedule_forever(void)
   loop running the secheduler forever*/
PHP_FUNCTION( go_go_debug )
{
	zval* arg;
	zval* callback;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz", &arg, &callback) == FAILURE)
    {
        php_printf("go: getting parameter failure");
		return;
    }
	
	void dump_zval(zval* zv);
	dump_zval(arg);
	dump_zval(callback);
	
	arg->is_ref__gc = 1;
	
	
	//zval* return_value;
	
	//ZVAL_LONG(debug_flag, 1111);
	
	zval** params[] = { &arg };
	if( call_user_function_ex(
		EG(function_table), 
		NULL, 
		callback,                   // the callback callable
		&return_value,              // zval** to receive return value
		1,                   // the parameter number required by the callback
		params,                // the parameter list of the callback
		1, 
		NULL TSRMLS_CC
	) != SUCCESS) {
		zend_error(E_ERROR, "phpgo: execution of go routine faild");
		return;
	}
		
	return;
/*	
	char *mystr;
    zval *mysubarray;
    array_init(return_value);

    add_index_long(return_value, 42, 123);
    add_next_index_string(return_value, "I should now be found at index 43", 1);
    add_next_index_stringl(return_value, "I'm at 44!", 10, 1);

    mystr = estrdup("Forty Five");
    add_next_index_string(return_value, mystr, 0);

    add_assoc_double(return_value, "pi", 3.1415926535);

    ALLOC_INIT_ZVAL(mysubarray);
    array_init(mysubarray);
    add_next_index_string(mysubarray, "hello", 1);
    add_assoc_zval(return_value, "subarray", mysubarray);
	
	
	zval *arr, **data;
    HashTable *arr_hash;
    HashPosition pointer;
    int array_count;
	
	arr_hash = Z_ARRVAL_P(array);
    array_count = zend_hash_num_elements(arr_hash);
    php_printf("The array passed contains %d elements ", array_count);

    for(zend_hash_internal_pointer_reset_ex(arr_hash, &pointer); 
		zend_hash_get_current_data_ex(arr_hash, (void**) &data, &pointer) == SUCCESS; 
		zend_hash_move_forward_ex(arr_hash, &pointer)
	) {
        if (Z_TYPE_PP(data) == IS_STRING) {

            PHPWRITE(Z_STRVAL_PP(data), Z_STRLEN_PP(data));

            php_printf(" ");

        }
    }
	
	php_printf("\n");
	
	
	int i = 0;
	for(zend_hash_internal_pointer_reset_ex(arr_hash, &pointer); 
		zend_hash_get_current_data_ex(arr_hash, (void**) &data, &pointer) == SUCCESS; 
		zend_hash_move_forward_ex(arr_hash, &pointer)) {
        zval temp;
        temp = **data;
        zval_copy_ctor(&temp);
        //convert_to_string(&temp);
        //PHPWRITE(Z_STRVAL(temp), Z_STRLEN(temp));
		
		zval *element;
		ALLOC_INIT_ZVAL(element);
		*element = **data;
		zval_copy_ctor(element);
		zval_add_ref(&element);
		add_index_zval(return_value, i++, element);
		//add_assoc_zval(return_value, "abc", element);
		
		
        php_printf(" ");
        zval_dtor(&temp);
	}
	php_printf("\n");
	
	
	for(zend_hash_internal_pointer_reset_ex(arr_hash, &pointer); 
		zend_hash_get_current_data_ex(arr_hash, (void**) &data, &pointer) == SUCCESS; 
		zend_hash_move_forward_ex(arr_hash, &pointer)) {

		zval temp;
		char *key;
		unsigned int key_len;
		unsigned long index;

		if (zend_hash_get_current_key_ex(arr_hash, &key, &key_len, &index, 0, &pointer) == HASH_KEY_IS_STRING) {
			PHPWRITE(key, key_len);
		} else {
			php_printf("%ld", index);
		}

		php_printf(" => ");

		temp = **data;
		zval_copy_ctor(&temp);
		convert_to_string(&temp);
		PHPWRITE(Z_STRVAL(temp), Z_STRLEN(temp));
		php_printf(" ");
		zval_dtor(&temp);
	}*/
	
	php_printf("\n");
	
	

	
	//php_api_go_debug(debug_flag);
}
/* }}} */

/* {{{ proto void go_schedule_forever(void)
   loop running the secheduler forever*/
PHP_FUNCTION(go_chan_create)
{
	unsigned long capacity = 0;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &capacity) == FAILURE)
    {
        php_printf("phpgo: getting parameter failure");
    }
	
	void* chan = php_api_go_chan_create(capacity);
	
	ZEND_REGISTER_RESOURCE(return_value, chan, le_go_channel);
}
/* }}} */

/* {{{ proto void go_schedule_forever(void)
   loop running the secheduler forever*/
PHP_FUNCTION(go_chan_push)
{
	zval* chan     = NULL;
	zval* z        = NULL;
	void* chan_obj = NULL;
	
	if( zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rz", &chan, &z) == FAILURE ){
        zend_error(E_ERROR, "phpgo: go_chan_push: getting parameter failure");
		RETURN_NULL();
    }
	
	if(!chan)
		RETURN_NULL();
	
	ZEND_FETCH_RESOURCE(chan_obj, void*, &chan, -1, le_go_channel_name, le_go_channel);
	
	void* ch = php_api_go_chan_push(chan_obj, z);
	
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto void go_schedule_forever(void)
   loop running the secheduler forever*/
PHP_FUNCTION(go_chan_pop)
{
	zval* chan     = NULL;
	void* chan_obj = NULL;
	
	if( zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &chan) == FAILURE ){
        zend_error(E_ERROR, "phpgo: go_chan_push: getting parameter failure");
		RETURN_NULL();
    }
	
	if(!chan) {
		RETURN_NULL();
	}
	
	ZEND_FETCH_RESOURCE(chan_obj, void*, &chan, -1, le_go_channel_name, le_go_channel);
	
	zval* z = php_api_go_chan_pop(chan_obj);
	
	if(!z)
		RETURN_NULL();
	
	RETURN_ZVAL(z, 1, 0);
}
/* }}} */

/* {{{ proto void go_chan_close($chan)
   close a channel and null-out the variable*/
PHP_FUNCTION(go_chan_close)
{
	zval* chan;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &chan) == FAILURE)
    {
        zend_error(E_ERROR, "phpgo: getting parameter failure");
		RETURN_FALSE;
    }

	// do not do the following as the destructor (phpho_destructor_go_channel) will do this
	// php_api_go_chan_close(chan_obj);
	
	// just remove the resource from the resource list
	// zend_list_delete may trigger phpho_destructor_go_channel() 
	
	bool ret = zend_list_delete(Z_RESVAL_P(chan)) == SUCCESS;
	RETURN_BOOL( ret );

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

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zszz", &chan, &op, &op_len, &value, &callback ) == FAILURE)
    {
        zend_error(E_ERROR, "phpgo: getting parameter failure");
		goto error_return;
    }
	
	//printf( "chan: %ld op: %s callback: %p\n" , chan, op, callback);
	
	if( strcmp(op, "->") ==0 ){
		op_i = GO_CASE_OP_READ;
	}else if( strcmp(op, "<-") ==0 ){
		op_i = GO_CASE_OP_WRITE;
	}
	else{
		zend_error(E_ERROR, "phpgo: invalid channel operation %s", op);
		goto error_return;
	}
	
    if (!zend_is_callable(callback, 0, &func_name TSRMLS_CC)){
        zend_error(E_ERROR, "phpgo: function '%s' is not callable", func_name);
        goto error_return;
    }
	
	//long lchan;
	//ZEND_FETCH_RESOURCE(lchan, long, &chan, -1, le_go_channel_name, le_go_channel);
	
	array_init(return_value);
	add_index_long(return_value, 0, GO_CASE_TYPE_CASE);
	
	zval_add_ref(&chan);
	add_next_index_zval(return_value, chan);
    
	add_next_index_long(return_value, op_i);
	
	zval_add_ref(&value);
	add_next_index_zval(return_value, value);
	
	//extern void dump_zval(zval*);
	//printf("-------_case: callback=\n");
	//dump_zval(callback);
	
	zval_add_ref(&callback);
	//printf("------go_case2: callback=\n");
	//dump_zval(callback);
	add_next_index_zval(return_value, callback);
	
	//printf("casetype CASE, ch %ld, op %s, value %p, callback %p\n", chan, op, value, callback  );
	
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
	
	zval_add_ref(&callback);
	add_next_index_zval(return_value, callback);
	
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
		php_api_go_select(case_array, case_count);
	
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

/* {{{ proto void go_schedule_forever(void)
   loop running the secheduler forever*/
PHP_FUNCTION(go_mutex_create)
{
	::co::CoRecursiveMutex *mutex = new ::co::CoRecursiveMutex();
	ZEND_REGISTER_RESOURCE(return_value, mutex, le_go_mutex);
}
/* }}} */

/* {{{ proto void go_schedule_forever(void)
   loop running the secheduler forever*/
PHP_FUNCTION(go_mutex_destroy)
{
	zval* mutex;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &mutex) == FAILURE){
        zend_error(E_ERROR, "phpgo: getting parameter failure");
		RETURN_FALSE;
    }
	
	// just remove the resource from the resource list
	// zend_list_delete may trigger phpho_destructor_go_mutex() 
	bool ret = zend_list_delete(Z_RESVAL_P(mutex)) == SUCCESS;
	RETURN_BOOL( ret );
}
/* }}} */

/* {{{ proto void go_schedule_forever(void)
   loop running the secheduler forever*/
PHP_FUNCTION(go_mutex_lock)
{
	zval* mutex; ::co::CoRecursiveMutex *mutex_obj;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &mutex) == FAILURE){
        zend_error(E_ERROR, "phpgo: getting parameter failure");
		RETURN_FALSE;
    }
	
	ZEND_FETCH_RESOURCE(mutex_obj, ::co::CoRecursiveMutex*, &mutex, -1, le_go_mutex_name, le_go_mutex);
	
	mutex_obj->lock();
	
}
/* }}} */

/* {{{ proto void go_schedule_forever(void)
   loop running the secheduler forever*/
PHP_FUNCTION(go_mutex_unlock)
{
	zval* mutex; ::co::CoRecursiveMutex *mutex_obj;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &mutex) == FAILURE){
        zend_error(E_ERROR, "phpgo: getting parameter failure");
		RETURN_FALSE;
    }
	
	ZEND_FETCH_RESOURCE(mutex_obj, ::co::CoRecursiveMutex*, &mutex, -1, le_go_mutex_name, le_go_mutex);
	
	mutex_obj->unlock();
}
/* }}} */

/* {{{ proto void go_schedule_forever(void)
   loop running the secheduler forever*/
PHP_FUNCTION(go_mutex_try_lock)
{
	zval* mutex; ::co::CoRecursiveMutex *mutex_obj;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &mutex) == FAILURE){
        zend_error(E_ERROR, "phpgo: getting parameter failure");
		RETURN_FALSE;
    }
	
	ZEND_FETCH_RESOURCE(mutex_obj, ::co::CoRecursiveMutex*, &mutex, -1, le_go_mutex_name, le_go_mutex);
	
	RETURN_BOOL( mutex_obj->try_lock() );
}
/* }}} */

/* {{{ proto void go_schedule_forever(void)
   loop running the secheduler forever*/
PHP_FUNCTION(go_mutex_is_lock)
{
	zval* mutex; ::co::CoRecursiveMutex *mutex_obj;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &mutex) == FAILURE){
        zend_error(E_ERROR, "phpgo: getting parameter failure");
		RETURN_FALSE;
    }
	
	ZEND_FETCH_RESOURCE(mutex_obj, ::co::CoRecursiveMutex*, &mutex, -1, le_go_mutex_name, le_go_mutex);
	
	RETURN_BOOL( mutex_obj->is_lock() );
}
/* }}} */

  /* {{{ proto Mutex::__construct
  * Create a mutex
  */
PHP_METHOD(Mutex,__construct){
	printf("Mutex::__construct\n");
	
	bool signaled = true; 
	if( ZEND_NUM_ARGS() ){
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "b", &signaled) == FAILURE) {
			zend_error(E_ERROR, "phpgo: Chan::__construct: getting parameter failure");
			return;
		}
	}
	
	void* mutex = php_api_go_mutex_create(signaled);
	return_value = getThis();
	zend_update_property_long(ce_go_mutex_ptr, return_value, "handle",   sizeof("handle"),   (long)mutex TSRMLS_CC);	
 }
 /* }}} */
 
  /* {{{ proto Mutex::Lock
  * Lock mutex. 
  * If currently not in a coroutine and the lock cannot be obtained, the scheduler 
  * will be executed in 10ms interval until the lock is released by the go-routines
  * or other threads
  */
 PHP_METHOD(Mutex,Lock){
	 
	printf("Mutex::Lock\n");
	
	zval* mutex     = NULL;
	zval* z         = NULL;
	
	auto self = getThis();
	mutex = zend_read_property(ce_go_mutex_ptr, self, "handle", sizeof("handle"), true TSRMLS_CC);

	if(!mutex)
		RETURN_FALSE;
	
	auto lmutex = Z_LVAL_P(mutex);
	php_api_go_mutex_lock((void*)lmutex);
	
	RETURN_TRUE;
 }
 /* }}} */
 
 /* {{{ proto Mutex::Unlock
  * Unlock mutex
  */
 PHP_METHOD(Mutex,Unlock){
	 
	printf("Mutex::Unlock\n");
	
	zval* mutex     = NULL;
	zval* z        = NULL;
	
	auto self = getThis();
	mutex = zend_read_property(ce_go_mutex_ptr, self, "handle",   sizeof("handle"), true TSRMLS_CC);

	if(!mutex)
		RETURN_FALSE;
	
	auto lmutex = Z_LVAL_P(mutex);
	php_api_go_mutex_unlock((void*)lmutex);
	
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
	mutex = zend_read_property(ce_go_mutex_ptr, self, "handle",   sizeof("handle"), true TSRMLS_CC);

	if(!mutex)
		RETURN_FALSE;
	
	auto lmutex = Z_LVAL_P(mutex);
	
	printf("Mutex::TryLock %p\n", lmutex);
	RETURN_BOOL( php_api_go_mutex_try_lock((void*)lmutex) );
 }
 /* }}} */
 
  /* {{{ proto Mutex::Unlock
  * Unlock mutex
  */
 PHP_METHOD(Mutex,IsLock){
	 
	printf("Mutex::IsLock\n");
	
	zval* mutex     = NULL;
	zval* z         = NULL;
	
	auto self = getThis();
	mutex = zend_read_property(ce_go_mutex_ptr, self, "handle",   sizeof("handle"), true TSRMLS_CC);

	if(!mutex)
		RETURN_FALSE;
	
	auto lmutex = Z_LVAL_P(mutex);
	RETURN_BOOL( php_api_go_mutex_is_lock((void*)lmutex) );
 }
 /* }}} */
 
 
 /* {{{ proto Mutex::__destruct
  * Destruct the mutex
  */
 PHP_METHOD(Mutex,__destruct){
	 
	printf("Mutex::__destruct\n");
	
	zval* self = getThis();
	zval* mutex = zend_read_property(ce_go_mutex_ptr, self, "handle", sizeof("handle"), true TSRMLS_CC);
	
	if(!mutex)
		RETURN_FALSE;
	
	auto lmutex = Z_LVAL_P(mutex);
	php_api_go_mutex_destroy( (void*)lmutex );
 }
 /* }}} */


  /* {{{ proto Waitgroup::__construct
  * Create a waitgroup
  */
PHP_METHOD(Waitgroup,__construct){
	printf("Waitgroup::__construct\n");
	void* wg = php_api_go_waitgroup_create();
	return_value = getThis();
	zend_update_property_long(ce_go_waitgroup_ptr, return_value, "handle",   sizeof("handle"),   (long)wg TSRMLS_CC);	
 }
 /* }}} */
 
  /* {{{ proto Waitgroup::__construct
  * Create a waitgroup
  */
 PHP_METHOD(Waitgroup,Add){
 	printf("Waitgroup::Add\n");
	
	zval* wg       = NULL;
	int64_t delta  = 1;
	
	if( zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", (long*)&delta) == FAILURE ){
        zend_error(E_ERROR, "phpgo: Waitgroup::Add: getting parameter failure");
		RETURN_FALSE;
    }
	
	auto self = getThis();
	wg = zend_read_property(ce_go_waitgroup_ptr, self, "handle",   sizeof("handle"), true TSRMLS_CC);

	if(!wg)
		RETURN_FALSE;
	
	auto lwg = Z_LVAL_P(wg);
	int64_t count = php_api_go_waitgroup_add( (void*)lwg, delta );
	
	RETURN_LONG(count);
 }
  /* }}} */
 
  /* {{{ proto Waitgroup::__construct
  * Create a waitgroup
  */
 PHP_METHOD(Waitgroup,Done){
 	printf("Waitgroup::Done\n");
	
	zval* wg       = NULL;

	auto self = getThis();
	wg = zend_read_property(ce_go_waitgroup_ptr, self, "handle",   sizeof("handle"), true TSRMLS_CC);

	if(!wg)
		RETURN_FALSE;
	
	auto lwg = Z_LVAL_P(wg);
	int64_t count = php_api_go_waitgroup_done( (void*)lwg );
	
	RETURN_LONG(count);
 }
  /* }}} */ 
  
  /* {{{ proto Waitgroup::__construct
  * Create a waitgroup
  */
 PHP_METHOD(Waitgroup,Count){
 	printf("Waitgroup::Count\n");
	
	zval* wg       = NULL;

	auto self = getThis();
	wg = zend_read_property(ce_go_waitgroup_ptr, self, "handle",   sizeof("handle"), true TSRMLS_CC);

	if(!wg)
		RETURN_FALSE;
	
	auto lwg = Z_LVAL_P(wg);
	int64_t count = php_api_go_waitgroup_count( (void*)lwg );
	
	RETURN_LONG(count);
 }
  /* }}} */ 
  
 /* {{{ proto Waitgroup::__construct
  * Create a waitgroup
  */
 PHP_METHOD(Waitgroup,Wait){
 	printf("Waitgroup::Wait\n");
	
	zval* wg       = NULL;

	auto self = getThis();
	wg = zend_read_property(ce_go_waitgroup_ptr, self, "handle",   sizeof("handle"), true TSRMLS_CC);

	if(!wg)
		RETURN_FALSE;
	
	auto lwg = Z_LVAL_P(wg);
	php_api_go_waitgroup_wait( (void*)lwg );
	
	RETURN_TRUE;
 }
  /* }}} */ 
  
  /* {{{ proto Waitgroup::__destruct
  * Destruct the waitgroup
  */
 PHP_METHOD(Waitgroup,__destruct){
	 
	printf("Waitgroup::__destruct\n");
	
	zval* self = getThis();
	zval* wg = zend_read_property(ce_go_waitgroup_ptr, self, "handle", sizeof("handle"), true TSRMLS_CC);
	
	if(!wg)
		RETURN_FALSE;
	
	auto lwg = Z_LVAL_P(wg);
	php_api_go_waitgroup_destroy( (void*)lwg );
 }
 /* }}} */
 
 /* {{{ proto void go_schedule_once(void)
   run the secheduler for a one pass*/
PHP_METHOD(Scheduler, RunOnce)
{
	php_api_go_schedule_once();
}
/* }}} */

/* {{{ proto void go_schedule_all(void)
   run the secheduler until all go routines completed*/
PHP_METHOD(Scheduler, RunJoinAll)
{
	php_api_go_schedule_all();
}
/* }}} */

/* {{{ proto void go_schedule_forever(void)
   loop running the secheduler forever*/
PHP_METHOD(Scheduler, RunForever)
{
	php_api_go_schedule_forever();
}
/* }}} */



/* {{{ proto void go_schedule_forever(void)
   loop running the secheduler forever*/
PHP_METHOD(Timer, Tick)
{
	printf("Timer::Tick\n");
	
	uint64_t microseconds  = 0;
	if( zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", (long*)&microseconds) == FAILURE ){
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
	
	auto z_handler = zend_read_property(ce_go_chan_ptr, z_chan, "handle",   sizeof("handle"), true TSRMLS_CC);
	
	if(!z_handler)
		RETURN_NULL();
	
	void* chan = (void*)Z_LVAL_P(z_handler);
	if(!chan)
		RETURN_NULL();
	
	php_api_go_timer_tick(z_chan, chan, microseconds);
	
	RETURN_ZVAL(z_chan, 1, 1);
}
/* }}} */


/* {{{ proto void go_schedule_forever(void)
   loop running the secheduler forever*/
PHP_METHOD(Timer, After)
{
	printf("Timer::After\n");
	
	uint64_t microseconds  = 0;
	if( zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", (long*)&microseconds) == FAILURE ){
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
	
	auto z_handler = zend_read_property(ce_go_chan_ptr, z_chan, "handle",   sizeof("handle"), true TSRMLS_CC);
	
	if(!z_handler)
		RETURN_NULL();
	
	void* chan = (void*)Z_LVAL_P(z_handler);
	if(!chan)
		RETURN_NULL();
	
	php_api_go_timer_after(z_chan, chan, microseconds);
	
	RETURN_ZVAL(z_chan, 1, 1);
}
/* }}} */
