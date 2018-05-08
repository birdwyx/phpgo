#pragma once

#ifdef ZTS
#define THREAD_LOCAL thread_local
#else
#define THREAD_LOCAL
#endif

#define PHPGO_MAX_GO_ARGS  (256 + 2)  // goo($option, $callable, $arg1,...,arg256);

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

/*copied from zend_execute.c*/
zend_always_inline zend_vm_stack zend_vm_stack_new_page(size_t size) {
	zend_vm_stack page = (zend_vm_stack)emalloc(size);

	page->top = ZEND_VM_STACK_ELEMENTS(page);
	page->end = (zval*)((char*)page + size);
	page->prev = NULL;
	return page;
}

#if PHP_MAJOR_VERSION >= 7
	#define ALLOC_PERMANENT_ZVAL(z)	\
		do{ \
			z = (zval *)pemalloc(sizeof(zval),1);  \
			bzero(z, sizeof(zval)); \
		}while(0)
	
	#define ALLOC_ZVAL(z)                     do{ z = (zval *)emalloc(sizeof(zval)); bzero(z, sizeof(zval)); }while(0)
	#define INIT_PZVAL(z)                     
	#define ALLOC_INIT_ZVAL(z)                do{ ALLOC_ZVAL(z); INIT_PZVAL(z); }while(0)
	#define MAKE_STD_ZVAL(z)                  ALLOC_INIT_ZVAL(z)
	#define MAKE_STD_ZVAL_IN_STACK(z)         zval _stack_zval_##z; z = &(_stack_zval_##z)
	#define MAKE_COPY_ZVAL(ppz_src, pz_dest)  ZVAL_COPY(pz_dest, *ppz_src)
	#define INIT_PZVAL_COPY(z_dest,z_src)     ZVAL_COPY_VALUE(z_dest,z_src)
									
	#define phpgo_zval_ptr_dtor(ppz)                          zval_ptr_dtor(*ppz)
	#define phpgo_zval_add_ref(ppz)                           zval_add_ref(*ppz)
	#define PHPGO_MAKE_STD_ZVAL(z)                            MAKE_STD_ZVAL(z)   
	#define PHPGO_MAKE_STD_ZVAL_IN_STACK(z)                   MAKE_STD_ZVAL_IN_STACK(z)
	#define PHPGO_ALLOC_INIT_ZVAL(z)                          ALLOC_INIT_ZVAL(z)
	#define PHPGO_SEPARATE_ZVAL(z)                            SEPARATE_ZVAL(z)
	#define PHPGO_MAKE_COPY_ZVAL(ppz_src, pz_dest)            MAKE_COPY_ZVAL(ppz_src, pz_dest)
	#define PHPGO_ZVAL_STRING(z,str,duplicate)                ZVAL_STRING(z, str)
	#define PHPGO_FREE_PZVAL(pz)                              efree(pz)
	#define PHPGO_FREE_PERMENENT_PZVAL(pz)                    pefree(pz, 1)
	#define PHPGO_REPLACE_ZVAL_VALUE(ppzv_dest, pzv_src, copy) \
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
	
	#define PHPGO_ARG_TYPE zval
	#define PHPGO_ARG_TO_PZVAL(zv) (&zv)
	#define PHPGO_HASH_DATA_TO_ARG(data) (*(zval*)data)
	
	#define PHPGO_INVALID_HASH_POSITION  HT_INVALID_IDX
	
	#define EG_VM_STACK (EG(vm_stack))
	
	typedef zend_string* FUNC_NAME_TYPE;
	#define FREE_FUNC_NAME(func_name) zend_string_release(func_name)
	
	#define PHP5_AND_BELOW_ONLY_CC(x)
	#define PHP5_VS_7(x,y) (y)
	#define PHP7_AND_ABOVE(x) x
	#define PHP5_AND_BELOW(x)
	
	#define PHPGO_ZVAL_IS_BOOL(z) (Z_TYPE_P(z)==IS_TRUE || Z_TYPE_P(z)==IS_FALSE)
#else
	#define phpgo_zval_ptr_dtor(ppz)               zval_ptr_dtor(ppz)
	#define phpgo_zval_add_ref(ppz)                zval_add_ref(ppz)
	#define PHPGO_MAKE_STD_ZVAL(z)                 MAKE_STD_ZVAL(z)
	#define PHPGO_ALLOC_INIT_ZVAL(p)               ALLOC_INIT_ZVAL(p)
	#define PHPGO_SEPARATE_ZVAL(p)                 SEPARATE_ZVAL(p)
	#define PHPGO_MAKE_COPY_ZVAL(ppz_src, pz_dest) MAKE_COPY_ZVAL(ppz_src, pz_dest)
	#define PHPGO_ZVAL_STRING(z,str,duplicate)     ZVAL_STRING(z, str, duplicate)
	#define PHPGO_FREE_PZVAL(pz)
    #define PHPGO_FREE_PERMENENT_PZVAL(pz)
	#define PHPGO_REPLACE_ZVAL_VALUE(ppzv_dest, pzv_src, copy) \
	        REPLACE_ZVAL_VALUE(ppzv_dest, pzv_src, copy)
	
	#define PHPGO_ARG_TYPE zval**
	#define PHPGO_ARG_TO_PZVAL(ppz) (*ppz)
	#define PHPGO_HASH_DATA_TO_ARG(data) ((zval**)data)
	
	#define PHPGO_INVALID_HASH_POSITION  NULL
	
	#define EG_VM_STACK (EG(argument_stack))
	
	typedef char* FUNC_NAME_TYPE;
	#define FREE_FUNC_NAME(func_name) efree(func_name)
	
	#define PHP5_AND_BELOW_ONLY_CC(x) ,x
	#define PHP5_VS_7(x,y) (x)
	#define PHP7_AND_ABOVE(x)
	#define PHP5_AND_BELOW(x) x
	
	#define PHPGO_ZVAL_IS_BOOL(z) (Z_TYPE_P(z)==IS_BOOL)
#endif


#if PHP_MAJOR_VERSION >= 7

typedef uint32_t zend_uint;  //php7 has no zend_uint
#define IS_BOOL _IS_BOOL

//php7 uses IS_REFERENCE type of zval to replace the is_ref
//so the unset of is_ref is no relevant any more
#define Z_UNSET_ISREF_P(z)


#define Z_BVAL_P(z)     (Z_TYPE_P(z) == IS_TRUE)
#define Z_BVAL(z)       (Z_TYPE(z) == IS_TRUE)

#define PZVAL_IS_REF(z) Z_ISREF_P(z) 
#define IS_CONSTANT_TYPE_MASK  (-1)


#define phpgo_add_assoc_string(array, key, key_len, value, duplicate)   add_assoc_string(array, key, value)

#endif
