/*
   +----------------------------------------------------------------------+
   | Zend Engine                                                          |
   +----------------------------------------------------------------------+
   | Copyright (c) 1998-2015 Zend Technologies Ltd. (http://www.zend.com) |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.00 of the Zend license,     |
   | that is bundled with this package in the file LICENSE, and is        | 
   | available through the world-wide-web at the following url:           |
   | http://www.zend.com/license/2_00.txt.                                |
   | If you did not receive a copy of the Zend license and are unable to  |
   | obtain it through the world-wide-web, please send a note to          |
   | license@zend.com so we can mail you a copy immediately.              |
   +----------------------------------------------------------------------+
   | Authors: Andi Gutmans <andi@zend.com>                                |
   |          Zeev Suraski <zeev@zend.com>                                |
   +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifndef ZEND_PERMENENT_VARIABLES_H
#define ZEND_PERMENENT_VARIABLES_H

#define PHP_5_5_API_NO  220121212
#define PHP_5_6_API_NO  220131226

#define MAX_HASHTABLE_LAYERS  (16)

#if PHP_MAJOR_VERSION < 7
	#define ZVAL_PERSISTENT_PTR_DTOR (void (*)(void *)) zval_persistent_pptr_dtor_wrapper
#else
	#define ZVAL_PERSISTENT_PTR_DTOR (void (*)(zval *)) zval_persistent_ptr_dtor_wrapper
#endif

#define zval_persistent_ptr_dtor(ppz) i_zval_persistent_ptr_dtor(*(ppz) ZEND_FILE_LINE_CC)


ZEND_API void  zval_persistent_copy_ctor(zval* zvalue);
ZEND_API void _zval_persistent_copy_ctor_func(zval *zvalue ZEND_FILE_LINE_DC);

ZEND_API void  zval_persistent_to_local_ptr_ctor(zval** zvalue);
#define        zval_persistent_to_local_copy_ctor(z) _zval_persistent_to_local_copy_ctor_func(z ZEND_FILE_LINE_CC);
ZEND_API void _zval_persistent_to_local_copy_ctor_func(zval *zvalue ZEND_FILE_LINE_DC);

ZEND_API void  zval_persistent_pptr_dtor_wrapper(zval **zval_ptr);
ZEND_API void  zval_persistent_ptr_dtor_wrapper(zval *zval_ptr);
ZEND_API void _zval_persistent_dtor_func(zval *zvalue ZEND_FILE_LINE_DC);
static zend_always_inline void _zval_persistent_dtor(zval *zvalue ZEND_FILE_LINE_DC);
#define zval_persistent_dtor(z) _zval_persistent_dtor((z) ZEND_FILE_LINE_CC)

#if PHP_MAJOR_VERSION < 7
static zend_always_inline void i_zval_persistent_ptr_dtor(zval *zval_ptr ZEND_FILE_LINE_DC)
{
	if (!Z_DELREF_P(zval_ptr)) {
		_zval_persistent_dtor_func(zval_ptr ZEND_FILE_LINE_RELAY_CC);
		pefree_rel(zval_ptr, 1);
	} else {
		//TSRMLS_FETCH();

		if (Z_REFCOUNT_P(zval_ptr) == 1) {
			Z_UNSET_ISREF_P(zval_ptr);
		}
		//todo: to rewrite GC_ZVAL_CHECK_POSSIBLE_ROOT for full 
		//gc functionality on persistent memory
		//GC_ZVAL_CHECK_POSSIBLE_ROOT(zval_ptr);
	}
}

static zend_always_inline void _zval_persistent_dtor(zval *zvalue ZEND_FILE_LINE_DC)
{
	if (Z_TYPE_P(zvalue) <= IS_BOOL) {
		return;
	}
	_zval_persistent_dtor_func(zvalue ZEND_FILE_LINE_RELAY_CC);
}
#else
	
static zend_always_inline void i_zval_persistent_ptr_dtor(zval *zval_ptr ZEND_FILE_LINE_DC)
{
	if (Z_REFCOUNTED_P(zval_ptr)) {
		zend_refcounted *ref = Z_COUNTED_P(zval_ptr);
		if (!Z_DELREF_P(zval_ptr)) {
			_zval_persistent_dtor_func(zval_ptr ZEND_FILE_LINE_RELAY_CC);
		} else {
			//gc_check_possible_root(ref);
		}
	}
}

#endif

#endif


