/*
   +----------------------------------------------------------------------+
   | PHP Version 5                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997-2014 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors: Andi Gutmans <andi@zend.com>                                |
   |          Zeev Suraski <zeev@zend.com>                                |
   +----------------------------------------------------------------------+
 */

/* $Id$ */

#if PHP_MAJOR_VERSION < 7
#include "stdinc.h"
#include "ext/standard/basic_functions.h"

static int user_shutdown_function_call(php_shutdown_function_entry *shutdown_function_entry TSRMLS_DC) /* {{{ */
{
	zval retval;
	char *function_name;

	if (!zend_is_callable(shutdown_function_entry->arguments[0], 0, &function_name TSRMLS_CC)) {
		php_error(E_WARNING, "(Registered shutdown functions) Unable to call %s() - function does not exist", function_name);
		if (function_name) {
			efree(function_name);
		}
		return 0;
	}
	if (function_name) {
		efree(function_name);
	}

	if (call_user_function(EG(function_table), NULL,
				shutdown_function_entry->arguments[0],
				&retval,
				shutdown_function_entry->arg_count - 1,
				shutdown_function_entry->arguments + 1
				TSRMLS_CC ) == SUCCESS)
	{
		zval_dtor(&retval);
	}
	return 0;
}
/* }}} */

void php_call_shutdown_functions(TSRMLS_D) /* {{{ */
{
	if (BG(user_shutdown_function_names)) {
		zend_try {
			zend_hash_apply(BG(user_shutdown_function_names), (apply_func_t) user_shutdown_function_call TSRMLS_CC);
		}
		zend_end_try();
		php_free_shutdown_functions(TSRMLS_C);
	}
}
/* }}} */

void php_free_shutdown_functions(TSRMLS_D) /* {{{ */
{
	if (BG(user_shutdown_function_names))
		zend_try {
			zend_hash_destroy(BG(user_shutdown_function_names));
			FREE_HASHTABLE(BG(user_shutdown_function_names));
			BG(user_shutdown_function_names) = NULL;
		} zend_catch {
			/* maybe shutdown method call exit, we just ignore it */
			FREE_HASHTABLE(BG(user_shutdown_function_names));
			BG(user_shutdown_function_names) = NULL;
		} zend_end_try();
}
/* }}} */

#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: fdm=marker
 * vim: noet sw=4 ts=4
 */
