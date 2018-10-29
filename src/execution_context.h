#pragma once
#include "phpgo_context.h"
#include "class.h"

typedef struct {
	uint64_t                   guard__[8];
	
	struct _zend_execute_data* EG_current_execute_data;

#if PHP_MAJOR_VERSION < 7
	TSRMLS_FIELD;  /*ZTS: void ***tsrm_ls;*/
	
	/*go routine running environment*/
	zend_vm_stack 			   EG_argument_stack;
	zend_class_entry*		   EG_scope;                
	zval*					   EG_This;                 
	zend_class_entry*		   EG_called_scope;         
	HashTable*				   EG_active_symbol_table;  
	zval**					   EG_return_value_ptr_ptr; 
	zend_op_array*			   EG_active_op_array;      
	zend_op**				   EG_opline_ptr;           
	zval                       EG_error_zval;           
	zval*                      EG_error_zval_ptr;       
	zval*                      EG_user_error_handler;
	

#else 
	/* php7 */
	zend_vm_stack			   EG_vm_stack;
    zval*                      EG_vm_stack_top;
    zval*                      EG_vm_stack_end;
	

#endif

	JMP_BUF*                   EG_bailout;
	
	//for ob_xxxx
	zend_stack                 OG_handlers;
	php_output_handler*        OG_active;
	php_output_handler*        OG_running;
	const char*                OG_output_start_filename;
	int                        OG_output_start_lineno;
	int                        OG_flags;
	//

	uint64_t                   __guard[8];
} GrExecutionData;

CLASS(GrExecutionContext){
	BASE_CONTEXT_FIELDS;
	GrExecutionData ctx0;
	GrExecutionData ctx1;
};

DEFINE_EXTERN_CLASS_FUNC(GrExecutionContext, OnCreated);
DEFINE_EXTERN_CLASS_FUNC(GrExecutionContext, OnStart);
DEFINE_EXTERN_CLASS_FUNC(GrExecutionContext, OnSwapIn);
DEFINE_EXTERN_CLASS_FUNC(GrExecutionContext, OnSwapOut);
DEFINE_EXTERN_CLASS_FUNC(GrExecutionContext, OnFinished);
DEFINE_EXTERN_CLASS_FUNC(GrExecutionContext, OnEnd);