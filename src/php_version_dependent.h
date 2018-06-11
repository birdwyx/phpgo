#pragma once


#if PHP_MAJOR_VERSION >= 7
	/*dtor and add_ref now takes zval* instead of zval** as parameter*/
	#define phpgo_zval_ptr_dtor(ppz)                          zval_ptr_dtor(*ppz)
	#define phpgo_zval_add_ref(ppz)                           zval_add_ref(*ppz)

	/* the removed zval macros in php7 - but still required by phpgo */
	#define __MAKE_STD_ZVAL_IN_STACK(z)         zval _stack_zval_##z; z = &(_stack_zval_##z)	
	#define __ALLOC_PERMANENT_ZVAL(z)	        do{ z = (zval *)pemalloc(sizeof(zval),1); bzero(z, sizeof(zval)); }while(0)
	#define __ALLOC_ZVAL(z)                     do{ z = (zval *)emalloc(sizeof(zval)); bzero(z, sizeof(zval)); }while(0)
	#define __ALLOC_INIT_ZVAL(z)                do{ __ALLOC_ZVAL(z); __INIT_PZVAL(z); }while(0)
	#define __INIT_PZVAL(z)
	#define __MAKE_COPY_ZVAL(ppz_src, pz_dest)  ZVAL_COPY(pz_dest, *ppz_src)
	#define __INIT_PZVAL_COPY(z_dest,z_src)     ZVAL_COPY_VALUE(z_dest,z_src)
	#define __REPLACE_ZVAL_VALUE(ppzv_dest, pzv_src, copy) \
		do{	\
			zval* dest = *ppzv_dest;                    \
			if( Z_TYPE_P(dest) == IS_REFERENCE ){       \
				dest = Z_REFVAL_P(dest);                \
			}											\
			SEPARATE_ZVAL_IF_NOT_REF(dest);		        \
			zval_dtor(dest);						    \
			ZVAL_COPY_VALUE(dest, pzv_src);	            \
			if (copy) {                                 \
				zval_copy_ctor(dest);				    \
			}else{	                                    \
				Z_ADDREF_P(dest);                       \
			} \
		}while(0)

	#define PHPGO_MAKE_STD_ZVAL(z)                             __MAKE_STD_ZVAL_IN_STACK(z)
	#define PHPGO_ALLOC_PERMANENT_ZVAL(z)                      __ALLOC_PERMANENT_ZVAL(z)
	#define PHPGO_ALLOC_ZVAL(z)                                __ALLOC_ZVAL(z)                 
	#define PHPGO_ALLOC_INIT_ZVAL(z)                           __ALLOC_INIT_ZVAL(z)
	#define PHPGO_INIT_PZVAL(z)                                __INIT_PZVAL(z)
	#define PHPGO_MAKE_COPY_ZVAL(ppz_src, pz_dest)             __MAKE_COPY_ZVAL(ppz_src, pz_dest)
	#define PHPGO_INIT_PZVAL_COPY(z_dest,z_src)                __INIT_PZVAL_COPY(z_dest,z_src)
	#define PHPGO_REPLACE_ZVAL_VALUE(ppzv_dest, pzv_src, copy) __REPLACE_ZVAL_VALUE(ppzv_dest, pzv_src, copy)

	/*free the zval allocated in heap (e.g, by __ALLOC_ZVAL)
	  zval_ptr_dtor() won't free the zval memory itself since 
	  zvals are typically stored in stack for php7 
	*/
	#define PHPGO_FREE_PZVAL(pz)                               efree(pz)
	#define PHPGO_FREE_PERMENENT_PZVAL(pz)                     pefree(pz, 1)

	#define PHPGO_ZVAL_STRING(z,str,duplicate)                 ZVAL_STRING(z, str)
	#define PHPGO_ZVAL_IS_BOOL(z)                              (Z_TYPE_P(z)==IS_TRUE || Z_TYPE_P(z)==IS_FALSE)

	/*call_user_function argument type:	
	from php5 zval** -> php7 zval */
	#define PHPGO_ARG_TYPE                                     zval
	#define PHPGO_ARG_TO_PZVAL(zv)                             (&(zv))

	/*php7, use of zend_string instead of char* */
	typedef zend_string*                                       FUNC_NAME_TYPE;
	#define FREE_FUNC_NAME(func_name)                          zend_string_release(func_name)

	/*hash position where no data pointed*/
	#define PHPGO_INVALID_HASH_POSITION                        HT_INVALID_IDX
	
	/*vm stack address*/
	#define EG_VM_STACK                                        (EG(vm_stack))
	
	/*helpers to deal with the PHP5 and 7 
	zend api differences*/
	#define PHP5_AND_BELOW_ONLY_CC(x)
	#define PHP5_VS_7(x,y)                                     y
	#define PHP7_AND_ABOVE(x)                                  x
	#define PHP5_AND_BELOW(x)
	
	/*php7 changed zend apis*/
	#define phpgo_add_assoc_string(array, key, value, duplicate)   add_assoc_string(array, key, value)
#else
	#define phpgo_zval_ptr_dtor(ppz)                           zval_ptr_dtor(ppz)
	#define phpgo_zval_add_ref(ppz)                            zval_add_ref(ppz)

	#define PHPGO_MAKE_STD_ZVAL(z)                             MAKE_STD_ZVAL(z)
	#define PHPGO_ALLOC_PERMANENT_ZVAL(z)                      ALLOC_PERMANENT_ZVAL(z)
	#define PHPGO_ALLOC_ZVAL(z)                                ALLOC_ZVAL(z)                 
	#define PHPGO_ALLOC_INIT_ZVAL(z)                           ALLOC_INIT_ZVAL(z)
	#define PHPGO_INIT_PZVAL(z)                                INIT_PZVAL(z)
	#define PHPGO_MAKE_COPY_ZVAL(ppz_src, pz_dest)             MAKE_COPY_ZVAL(ppz_src, pz_dest)
	#define PHPGO_INIT_PZVAL_COPY(z_dest,z_src)                INIT_PZVAL_COPY(z_dest,z_src)
	#define PHPGO_REPLACE_ZVAL_VALUE(ppzv_dest, pzv_src, copy) REPLACE_ZVAL_VALUE(ppzv_dest, pzv_src, copy)

	#define PHPGO_FREE_PZVAL(pz)
    #define PHPGO_FREE_PERMENENT_PZVAL(pz)

	#define PHPGO_ZVAL_STRING(z,str,duplicate)                 ZVAL_STRING(z, str, duplicate)
	#define PHPGO_ZVAL_IS_BOOL(z)                              (Z_TYPE_P(z)==IS_BOOL)
	
	#define PHPGO_ARG_TYPE                                     zval**
	#define PHPGO_ARG_TO_PZVAL(ppz)                            (*ppz)

	typedef char*                                              FUNC_NAME_TYPE;
	#define FREE_FUNC_NAME(func_name)                          efree(func_name)

	/*hash position where no data pointed*/
	#define PHPGO_INVALID_HASH_POSITION                        NULL
	
	/*vm stack address*/
	#define EG_VM_STACK                                        (EG(argument_stack))

	/*helpers to deal with the PHP5 and 7 zend api differences*/
	#define PHP5_AND_BELOW_ONLY_CC(x)                          ,x
	#define PHP5_VS_7(x,y)                                     x
	#define PHP7_AND_ABOVE(x)
	#define PHP5_AND_BELOW(x)                                  x

	#define phpgo_add_assoc_string(array, key, value, duplicate)   add_assoc_string(array, key, value, duplicate)
#endif

#if PHP_MAJOR_VERSION >= 7
	typedef uint32_t                                           zend_uint;  //php7 has no zend_uint
	#define IS_BOOL                                            _IS_BOOL

	#define Z_BVAL_P(z)                                        (Z_TYPE_P(z) == IS_TRUE)
	#define Z_BVAL(z)                                          (Z_TYPE(z) == IS_TRUE)

	#define PZVAL_IS_REF(z)                                    Z_ISREF_P(z) 
	#define IS_CONSTANT_TYPE_MASK                              (-1)

#ifndef ZEND_VM_STACK_ELEMENTS
        #define ZEND_VM_STACK_ELEMENTS(stack) \
	(((zval*)(stack)) + ZEND_VM_STACK_HEADER_SLOTS)
#endif
	/*copied from zend_execute.c*/
	zend_always_inline zend_vm_stack zend_vm_stack_new_page(size_t size) {
		zend_vm_stack page = (zend_vm_stack)emalloc(size);

		page->top = ZEND_VM_STACK_ELEMENTS(page);
		page->end = (zval*)((char*)page + size);
		page->prev = NULL;
		return page;
	}
#endif

zend_always_inline ZEND_API zval * phpgo_zend_read_property(
	zend_class_entry *scope, zval *object, const char *name, 
	int name_length, zend_bool silent TSRMLS_DC
){
#if PHP_MAJOR_VERSION >= 7
	zval dummy;
	return zend_read_property(scope, object, name, name_length, silent, &dummy);
#else
	return zend_read_property(scope, object, name, name_length, silent TSRMLS_CC);
#endif
}

zend_always_inline ZEND_API int phpgo_zend_hash_get_current_data_ex(
    HashTable *ht, void **pData, HashPosition *pos
){
#if PHP_MAJOR_VERSION >= 7
	zval* z = zend_hash_get_current_data_ex(ht, pos);
	*pData = z;
	return z ? SUCCESS : FAILURE;
#else
	return zend_hash_get_current_data_ex(ht, pData, pos);
#endif		
}

zend_always_inline ZEND_API int _phpgo_zend_hash_add_or_update(
	HashTable *ht, const char *arKey, uint nKeyLength, void *pData, uint nDataSize, void **pDest, int flag ZEND_FILE_LINE_DC
){
#if PHP_MAJOR_VERSION >= 7
	zend_string* key = zend_string_init(arKey, nKeyLength-1, 0);
	
	//pData is a zval**, while php7 _zend_hash_add_or_update requires zval* as pData
	zval* z = _zend_hash_add_or_update(ht, key, *(zval**)pData, flag ZEND_FILE_LINE_CC);
	zend_string_release(key);
	
	if(pDest) *pDest = z;
	return z ? SUCCESS : FAILURE;
#else
	return _zend_hash_add_or_update(ht, arKey, nKeyLength, pData, nDataSize, pDest, flag ZEND_FILE_LINE_CC);
#endif	
}

#define phpgo_zend_hash_update(ht, arKey, nKeyLength, pData, nDataSize, pDest) \
	_phpgo_zend_hash_add_or_update(ht, arKey, nKeyLength, pData, nDataSize, pDest, HASH_UPDATE ZEND_FILE_LINE_CC)

#define phpgo_zend_hash_add(ht, arKey, nKeyLength, pData, nDataSize, pDest) \
	_phpgo_zend_hash_add_or_update(ht, arKey, nKeyLength, pData, nDataSize, pDest, HASH_ADD ZEND_FILE_LINE_CC)
	
// phpgo_zend_hash_find()
// php5: pData holds a zval** (i.e, pData is zval***) on return
// php7: pData holds a zval* on return
zend_always_inline ZEND_API int phpgo_zend_hash_find(
    const HashTable *ht, const char *arKey, uint nKeyLength, void **pData
){
#if PHP_MAJOR_VERSION >= 7
	zval* z = zend_hash_str_find(ht, arKey, nKeyLength-1);
	*pData = z;
	return z ? SUCCESS : FAILURE;
#else
	return zend_hash_find(ht, arKey, nKeyLength, pData);
#endif		
}
