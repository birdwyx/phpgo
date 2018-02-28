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
#include <stdio.h>
#include <set>
#include "zend.h"
#include "zend_API.h"
#include "zend_globals.h"
#include "zend_constants.h"
#include "zend_list.h"
#include "zend_extensions.h"
#include "zend_variables_persist.h"
#include "zend_gc.h"
#include "defer.h"


void zend_hash_display(const HashTable *ht)
{
	Bucket *p;
	uint i;

	if (UNEXPECTED(ht->nNumOfElements == 0)) {
		printf( "The hash is empty\n");
		return;
	}
	
	printf("hashtable: %p, elements: %ld, table size %ld\n", ht, ht->nNumOfElements, ht->nTableSize );
	
	for (i = 0; i < ht->nTableSize; i++) {
		p = ht->arBuckets[i];
		while (p != NULL) {
			printf("%s <==> 0x%lX, zval**=%p, zval*=%p, zval refcount=%ld\n",
                   p->arKey, p->h, p->pData, *(zval**)p->pData, (**((zval**)p->pData)).refcount__gc);
			//void dump_zval(zval*);
			//dump_zval(*(zval**)p->pData);
			p = p->pNext;
		}
	}
	
	printf("-----\n");

	p = ht->pListTail;
	while (p != NULL) {
		printf("%s <==> 0x%lX, zval**=%p, zval*=%p, zval refcount=%ld\n",
               p->arKey, p->h, p->pData, *(zval**)p->pData, (**((zval**)p->pData)).refcount__gc);
		p = p->pListLast;
	}
}


/*
 persistent zval copy constructor
 dealing with the zvals allocated in shared memory
 (as apposed to the zvals allocated in thread local memory)
*/
ZEND_API void zval_persistent_copy_ctor(zval* zvalue){
	_zval_persistent_copy_ctor_func(zvalue ZEND_FILE_LINE_CC);
}

ZEND_API void zval_persistent_ptr_ctor(zval** zvalue){
	/*Now that the zvalue (zval**) is pointing to persistent memory (i.e, allocated by malloc)
	  however the *zvalue (zval*) is still pointing to the caller's thread local, we need to 
	  allocate a zval and let *zvalue point to it.
	*/
	zval* z;
	ALLOC_PERSISTENT_ZVAL(z); 
	//copy the zval content (**zvalue) to shared memory, and re-initialize the refcount 
	*z = **zvalue; INIT_PZVAL(z);
	
	//change the zval** to point to z
	*zvalue = z;
	
	//call the constructor on zval* to copy all real data into share memory
	_zval_persistent_copy_ctor_func(*zvalue ZEND_FILE_LINE_CC);
}

ZEND_API void _zval_persistent_copy_ctor_func(zval *zvalue ZEND_FILE_LINE_DC)
{
	//printf("_zval_persistent_copy_ctor_func: zval %p, type = %ld\n",  zvalue, Z_TYPE_P(zvalue));
	//extern void dump_zval(zval*);
	//dump_zval(zvalue);
	//printf("_zval_persistent_copy_ctor_func: zval* = %p, ref = %ld, isref= %ld\n", zvalue, zvalue->refcount__gc, zvalue->is_ref__gc);
	
	switch (Z_TYPE_P(zvalue) & IS_CONSTANT_TYPE_MASK) {
		case IS_RESOURCE: {
				zend_error(E_ERROR, "duplication of resource type is not allowed in channel operation" );
				return;
			}
			break;
		case IS_BOOL:
		case IS_LONG:
		case IS_NULL:
			break;
		case IS_CONSTANT:
		case IS_STRING:
			CHECK_ZVAL_STRING_REL(zvalue);
			if (!IS_INTERNED(zvalue->value.str.val)) {
				//zvalue->value.str.val = (char *) estrndup_rel(zvalue->value.str.val, zvalue->value.str.len);
				zvalue->value.str.val = (char *) pestrndup(zvalue->value.str.val, zvalue->value.str.len, 1);
				//printf("gotten a new str at %p, s=%s\n", zvalue->value.str.val, zvalue->value.str.val);
			}
			break;
		case IS_ARRAY:
#if ZEND_EXTENSION_API_NO <= PHP_5_5_API_NO
		case IS_CONSTANT_ARRAY: 
#endif
			{
				typedef std::set<HashTable*> HtSet;
				static thread_local HtSet* htset = new HtSet();
				
				static thread_local int reentries = 0;
				++reentries; defer{ --reentries; };
				if( reentries > MAX_HASHTABLE_LAYERS ){
					zend_error(E_ERROR, "reference loop exists or nested level exceeds %d in array", MAX_HASHTABLE_LAYERS );
					return;
				}
	
				zval *tmp;
				HashTable *original_ht = zvalue->value.ht;
				HashTable *tmp_ht = NULL;
				TSRMLS_FETCH();

				if (zvalue->value.ht == &EG(symbol_table)) {
					return; /* do nothing */
				}
				
				//ALLOC_HASHTABLE_REL(tmp_ht);
				tmp_ht = (HashTable *) pemalloc_rel(sizeof(HashTable), 1);  //allocate persistently
				
				zend_hash_init(tmp_ht, zend_hash_num_elements(original_ht), NULL, ZVAL_PERSISTENT_PTR_DTOR, 1);  //1, init to persistent
				zvalue->value.ht = tmp_ht;
				
				//printf("zend_hash_copy tmp_ht = %p, ht->persistent %d\n", tmp_ht, tmp_ht->persistent);
				//printf("original_ht-->\n");
				//zend_hash_display(original_ht);
				//printf("original_ht<--\n");
				
				if( htset->find(original_ht) != htset->end() ){
					zend_error(E_WARNING, "reference loop exists in array" );
					return;
				}
				
				htset->insert(original_ht);
				defer{
					auto it = htset->find(original_ht);
					if( it!=htset->end() )
						htset->erase(it);
				};
				
				zend_hash_copy(tmp_ht, original_ht, (copy_ctor_func_t)zval_persistent_ptr_ctor, (void *) &tmp, sizeof(zval *));
				tmp_ht->nNextFreeElement = original_ht->nNextFreeElement;
				
				//printf("tmp_ht1-->\n");
				//zend_hash_display(tmp_ht);
				//printf("tmp_ht1<--\n");

			}
			break;
#if ZEND_EXTENSION_API_NO >= PHP_5_6_API_NO
		case IS_CONSTANT_AST:{
				zend_error(E_ERROR, "duplication of AST type is not allowed in channel operation" );
				return;
			}
			break;
#endif
		case IS_OBJECT:
			{
				zend_error(E_ERROR, "duplication of object type is not allowed in channel operation" );
				return;
			}
			break;
	}
}

/*
  zval persistent to local copy contructor
  dealing with the zvals copied from the shared memory to thread local storage
  the main difference to the normal zval_copy_ctor is that we must ensure no 
  any pointer is still pointing to the share memory after the 
  zval_persistent_to_local_ptr_ctor() call
*/

ZEND_API void zval_persistent_to_local_ptr_ctor(zval** zvalue){
	/*Now that the zvalue (zval**) is pointing to local memory (i.e, allocated by emalloc)
	  however the *zvalue (zval*) is still pointing to the share memory, we need to 
	  allocate a zval and let *zvalue point to it.
	*/
	zval* z;
	ALLOC_ZVAL(z); 
	//copy the zval content (**zvalue) to thread local, and re-initialize the refcount 
	*z = **zvalue; INIT_PZVAL(z);
	
	//change the zval** to point to z
	*zvalue = z;
	
	//call the constructor on zval* to copy all real data into thread local
	_zval_persistent_to_local_copy_ctor_func(*zvalue ZEND_FILE_LINE_CC);
}

ZEND_API void _zval_persistent_to_local_copy_ctor_func(zval *zvalue ZEND_FILE_LINE_DC)
{
	switch (Z_TYPE_P(zvalue) & IS_CONSTANT_TYPE_MASK) {
		case IS_RESOURCE: {
				//TSRMLS_FETCH();
				//zend_list_addref(zvalue->value.lval);
				zend_error(E_ERROR, "found resource type in channel operation while this should not happen" );
				return;
			}
			break;
		case IS_BOOL:
		case IS_LONG:
		case IS_NULL:
			break;
		case IS_CONSTANT:
		case IS_STRING:
			CHECK_ZVAL_STRING_REL(zvalue);
			if (!IS_INTERNED(zvalue->value.str.val)) {
				zvalue->value.str.val = (char *) estrndup_rel(zvalue->value.str.val, zvalue->value.str.len);
			}
			break;
		case IS_ARRAY:
#if ZEND_EXTENSION_API_NO <= PHP_5_5_API_NO
		case IS_CONSTANT_ARRAY: 
#endif
			{
				zval *tmp;
				HashTable *original_ht = zvalue->value.ht;
				HashTable *tmp_ht = NULL;
				TSRMLS_FETCH();

				if (zvalue->value.ht == &EG(symbol_table)) {
					return; /* do nothing */
				}
				ALLOC_HASHTABLE_REL(tmp_ht);
				zend_hash_init(tmp_ht, zend_hash_num_elements(original_ht), NULL, ZVAL_PTR_DTOR, 0);
				zvalue->value.ht = tmp_ht;
				zend_hash_copy(tmp_ht, original_ht, (copy_ctor_func_t)zval_persistent_to_local_ptr_ctor, (void *) &tmp, sizeof(zval *));
				tmp_ht->nNextFreeElement = original_ht->nNextFreeElement;
			}
			break;
#if ZEND_EXTENSION_API_NO >= PHP_5_6_API_NO
		case IS_CONSTANT_AST:
			//Z_AST_P(zvalue) = zend_ast_copy(Z_AST_P(zvalue));
			{
				zend_error(E_WARNING, "found AST type in channel operation while this should not happen" );
				return;
			}
			break;
#endif
		case IS_OBJECT:
			{
				//TSRMLS_FETCH();
				//Z_OBJ_HT_P(zvalue)->add_ref(zvalue TSRMLS_CC);
				zend_error(E_WARNING, "found object type in channel operation while this should not happen" );
				return;
			}
			break;
	}
}

/*
  persistent zval destructor
 dealing with the destruction of zvals allocated in shared memory
 (as apposed to the zvals allocated in thread local memory)
*/

ZEND_API void zval_persistent_ptr_dtor_wrapper(zval **zval_ptr)
{
	//printf("zval_persistent_ptr_dtor zval** = %p\n", zval_ptr);
	
	if( zval_ptr && *zval_ptr ){
		i_zval_persistent_ptr_dtor(*zval_ptr ZEND_FILE_LINE_CC);
	}
	
	// set the zval pointer to null to avoid a second dtor call
	*zval_ptr = nullptr;
	
	//printf("zval_persistent_ptr_dtor zval** = %p returns\n", zval_ptr);
}

ZEND_API void _zval_persistent_dtor_func(zval *zvalue ZEND_FILE_LINE_DC)
{
	#define STR_PERMENENT_FREE_REL(ptr)     if (ptr && !IS_INTERNED(ptr)) { pefree_rel(ptr,1); }
	#define FREE_PERMENENT_HASHTABLE(ht)	pefree(ht, 1)

	//printf("_zval_persistent_dtor_func zval* = %p\n", zvalue);
	//extern void dump_zval(zval*);
	//dump_zval(zvalue);
	
	switch (Z_TYPE_P(zvalue) & IS_CONSTANT_TYPE_MASK) {
		case IS_STRING:
		case IS_CONSTANT:
			CHECK_ZVAL_STRING_REL(zvalue);
			//printf("_zval_persistent_dtor_func try to free string: %p, s= %s\n", zvalue->value.str.val, zvalue->value.str.val);
			STR_PERMENENT_FREE_REL(zvalue->value.str.val);
			//printf("after free  string\n");
			break;
		case IS_ARRAY:
#if ZEND_EXTENSION_API_NO <= PHP_5_5_API_NO
		case IS_CONSTANT_ARRAY: 
#endif
			{
				TSRMLS_FETCH();
					//printf("_zval_persistent_dtor_func try to free ht: %p\n", zvalue->value.ht);
					//zend_hash_display(zvalue->value.ht);
					
				if (zvalue->value.ht && (zvalue->value.ht != &EG(symbol_table))) {
					/* break possible cycles */
					Z_TYPE_P(zvalue) = IS_NULL;
					zend_hash_destroy(zvalue->value.ht);

					FREE_PERMENENT_HASHTABLE(zvalue->value.ht);
				}
			}
			break;
#if ZEND_EXTENSION_API_NO >= PHP_5_6_API_NO
		case IS_CONSTANT_AST:{
				zend_error(E_WARNING, "found AST type in channel operation while this should not happen" );
				return;
			}
			break;
#endif
		case IS_OBJECT:
			{
				//TSRMLS_FETCH();

				//Z_OBJ_HT_P(zvalue)->del_ref(zvalue TSRMLS_CC);
				zend_error(E_WARNING, "found object type in channel operation while this should not happen" );
				return;
			}
			break;
		case IS_RESOURCE:
			{
				//TSRMLS_FETCH();

				/* destroy resource */
				//zend_list_delete(zvalue->value.lval);
				zend_error(E_WARNING, "found resource type in channel operation while this should not happen" );
				return;
			}
			break;
		case IS_LONG:
		case IS_DOUBLE:
		case IS_BOOL:
		case IS_NULL:
		default:
			return;
			break;
	}
	
	//printf("_zval_persistent_dtor_func returns\n");
}
