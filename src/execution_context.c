#include "execution_context.h"

DEFINE_CLASS_FUNC(GrExecutionContext, OnCreated){	
};

DEFINE_CLASS_FUNC(GrExecutionContext, OnStart){
};

DEFINE_CLASS_FUNC(GrExecutionContext, OnSwapIn){
#if PHP_MAJOR_VERSION < 7
	TSRMLS_FIELD;                  
	PHPGO_LOAD_TSRMLS(&__this->ctx0);
#endif

	/* save the current EG  */    
	__this->ctx0.EG_current_execute_data  =  EG(current_execute_data    );

#if PHP_MAJOR_VERSION < 7
	__this->ctx0.EG_argument_stack        =  EG(argument_stack          ); 
	__this->ctx0.EG_scope                 =  EG(scope                   ); 
	__this->ctx0.EG_This                  =  EG(This                    ); 
	__this->ctx0.EG_called_scope          =  EG(called_scope            ); 
	__this->ctx0.EG_active_symbol_table   =  EG(active_symbol_table     ); 
	__this->ctx0.EG_return_value_ptr_ptr  =  EG(return_value_ptr_ptr    ); 
	__this->ctx0.EG_active_op_array       =  EG(active_op_array         ); 
	__this->ctx0.EG_opline_ptr            =  EG(opline_ptr              ); 
	__this->ctx0.EG_error_zval            =  EG(error_zval              ); 
	__this->ctx0.EG_error_zval_ptr        =  EG(error_zval_ptr          ); 
	__this->ctx0.EG_user_error_handler    =  EG(user_error_handler      ); 
#else
	__this->ctx0.EG_vm_stack              =  EG(vm_stack                );
	__this->ctx0.EG_vm_stack_top          =  EG(vm_stack_top            );
	__this->ctx0.EG_vm_stack_end          =  EG(vm_stack_end            );
#endif

	__this->ctx0.EG_bailout               =  EG(bailout                 );
	
	
#if PHP_MAJOR_VERSION < 7
	//TSRMLS_FIELD;
	PHPGO_LOAD_TSRMLS(&__this->ctx1);
#endif
	
	/* load EG from the task specific context*/                            
	EG(current_execute_data )   =  __this->ctx1.EG_current_execute_data;

#if PHP_MAJOR_VERSION < 7
	EG(argument_stack       )   =  __this->ctx1.EG_argument_stack      ; 
	EG(scope                )   =  __this->ctx1.EG_scope               ; 
	EG(This                 )   =  __this->ctx1.EG_This                ; 
	EG(called_scope         )   =  __this->ctx1.EG_called_scope        ; 
	EG(active_symbol_table  )   =  __this->ctx1.EG_active_symbol_table ; 
	EG(return_value_ptr_ptr )   =  __this->ctx1.EG_return_value_ptr_ptr; 
	EG(active_op_array      )   =  __this->ctx1.EG_active_op_array     ; 
	EG(opline_ptr           )   =  __this->ctx1.EG_opline_ptr          ; 
	EG(error_zval           )   =  __this->ctx1.EG_error_zval          ; 
	EG(error_zval_ptr       )   =  __this->ctx1.EG_error_zval_ptr      ; 
	EG(user_error_handler   )   =  __this->ctx1.EG_user_error_handler  ;
#else
	EG(vm_stack             )	=  __this->ctx1.EG_vm_stack            ;
	EG(vm_stack_top         )	=  __this->ctx1.EG_vm_stack_top        ;
	EG(vm_stack_end         )	=  __this->ctx1.EG_vm_stack_end        ;
#endif

	EG(bailout              )   =  __this->ctx1.EG_bailout             ;	
	
};

DEFINE_CLASS_FUNC(GrExecutionContext, OnSwapOut){
#if PHP_MAJOR_VERSION < 7
	TSRMLS_FIELD;                  
	PHPGO_LOAD_TSRMLS(&__this->ctx1);
#endif

	/* save the current EG  */    
	__this->ctx1.EG_current_execute_data  =  EG(current_execute_data    );

#if PHP_MAJOR_VERSION < 7
	__this->ctx1.EG_argument_stack        =  EG(argument_stack          ); 
	__this->ctx1.EG_scope                 =  EG(scope                   ); 
	__this->ctx1.EG_This                  =  EG(This                    ); 
	__this->ctx1.EG_called_scope          =  EG(called_scope            ); 
	__this->ctx1.EG_active_symbol_table   =  EG(active_symbol_table     ); 
	__this->ctx1.EG_return_value_ptr_ptr  =  EG(return_value_ptr_ptr    ); 
	__this->ctx1.EG_active_op_array       =  EG(active_op_array         ); 
	__this->ctx1.EG_opline_ptr            =  EG(opline_ptr              ); 
	__this->ctx1.EG_error_zval            =  EG(error_zval              ); 
	__this->ctx1.EG_error_zval_ptr        =  EG(error_zval_ptr          ); 
	__this->ctx1.EG_user_error_handler    =  EG(user_error_handler      ); 
#else
	__this->ctx1.EG_vm_stack              =  EG(vm_stack                );
	__this->ctx1.EG_vm_stack_top          =  EG(vm_stack_top            );
	__this->ctx1.EG_vm_stack_end          =  EG(vm_stack_end            );
#endif

	__this->ctx1.EG_bailout               =  EG(bailout                 );
	
	
#if PHP_MAJOR_VERSION < 7
	//TSRMLS_FIELD;
	PHPGO_LOAD_TSRMLS(&__this->ctx0);
#endif
	
	/* load EG from the task specific context*/                            
	EG(current_execute_data )   =  __this->ctx0.EG_current_execute_data;

#if PHP_MAJOR_VERSION < 7
	EG(argument_stack       )   =  __this->ctx0.EG_argument_stack      ; 
	EG(scope                )   =  __this->ctx0.EG_scope               ; 
	EG(This                 )   =  __this->ctx0.EG_This                ; 
	EG(called_scope         )   =  __this->ctx0.EG_called_scope        ; 
	EG(active_symbol_table  )   =  __this->ctx0.EG_active_symbol_table ; 
	EG(return_value_ptr_ptr )   =  __this->ctx0.EG_return_value_ptr_ptr; 
	EG(active_op_array      )   =  __this->ctx0.EG_active_op_array     ; 
	EG(opline_ptr           )   =  __this->ctx0.EG_opline_ptr          ; 
	EG(error_zval           )   =  __this->ctx0.EG_error_zval          ; 
	EG(error_zval_ptr       )   =  __this->ctx0.EG_error_zval_ptr      ; 
	EG(user_error_handler   )   =  __this->ctx0.EG_user_error_handler  ;
#else
	EG(vm_stack             )	=  __this->ctx0.EG_vm_stack            ;
	EG(vm_stack_top         )	=  __this->ctx0.EG_vm_stack_top        ;
	EG(vm_stack_end         )	=  __this->ctx0.EG_vm_stack_end        ;
#endif

	EG(bailout              )   =  __this->ctx0.EG_bailout             ;
};

// finished, still in go routine
DEFINE_CLASS_FUNC(GrExecutionContext, OnFinished){
}

// finished, still in go routine, and is about to swap out
DEFINE_CLASS_FUNC(GrExecutionContext, OnEnd){
	
};