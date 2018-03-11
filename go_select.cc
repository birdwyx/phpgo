#include "go_select.h"
#include "go_chan.h"
#include <libgo/coroutine.h>
/*
* select - case
*/
zval*  phpgo_select(GO_SELECT_CASE* case_array, long case_count TSRMLS_DC){
	if(!case_count) return nullptr;
	
	//calling srand everytime will cuase the result less randomized,remove
	//srand(time(NULL)); 
	auto r = rand();
	auto start = r % case_count;
	
	GO_SELECT_CASE* selected_case = nullptr;
	bool any_case_ready =  false;
	
	zval* z_chan = nullptr;
	zval* z_handler = nullptr;
	co::Channel<zval*>* chanptr = nullptr;
	ChannelInfo* chinfo = nullptr;
	
	auto i = start;
	do{
		switch(case_array[i].case_type){
		case GO_CASE_TYPE_CASE:
			z_chan = case_array[i].chan;
			chinfo = GoChan::ZvalToChannelInfo(z_chan TSRMLS_CC);
			if( !chinfo ){
				zend_error(E_ERROR, "phpgo: phpgo_select: null channel");
				return nullptr;
			}
			if(case_array[i].op == GO_CASE_OP_READ){
				// GoChan::TryPop:
				// if data not ready to read, return nullptr
				// if channel is closed, return ZVAL_NULL
				// otherwise return the read zval
				zval* data = GoChan::TryPop(chinfo);
				if(data) {
					//zval_dtor(case_array[i].value);
					//INIT_PZVAL_COPY(case_array[i].value, data);
					//ZVAL_COPY_VALUE(case_array[i].value, data); 
					REPLACE_ZVAL_VALUE(&case_array[i].value, data, 0);
					zval_ptr_dtor(&data);					
					selected_case = &case_array[i];
					goto exit_while;
				}
			}else if(case_array[i].op == GO_CASE_OP_WRITE){
				auto ready = GoChan::TryPush(chinfo, case_array[i].value TSRMLS_CC);				
				if(ready){
					zval_add_ref(&case_array[i].value);
					selected_case = &case_array[i];
					goto exit_while;
				}
				
			}else{
				//error
			}
			break;
		case GO_CASE_TYPE_DEFAULT:
			selected_case = &case_array[i];
			break;
		default:
			break;
		}  // switch(case_array[i].case_type)
		
		i = (i+1) % case_count;
	}while (i != start );
	
exit_while:
	zval*   return_value = nullptr; 
	if( selected_case ){
		zval*** args         = nullptr;
		auto    argc         = 0;
		if(selected_case->case_type != GO_CASE_TYPE_DEFAULT){
			args    = (zval***)safe_emalloc(1, sizeof(zval **), 0);
			args[0] = &selected_case->value;
			argc    = 1;
		}
		
		zval_add_ref(&selected_case->callback);
		if( call_user_function_ex(
			EG(function_table), 
			NULL, 
			selected_case->callback,    // the callback callable
			&return_value,              // zval** to receive return value
			argc,                       // the parameter number required by the callback
			args,                       // the parameter list of the callback
			1, 
			NULL TSRMLS_CC
		) != SUCCESS) {
			zend_error(E_ERROR, "phpgo: execution of go routine faild");
			//goto cleanup;
		}

		zval_ptr_dtor(&selected_case->callback);
		if(args) efree(args);
	}
	
	if( g_Scheduler.IsCoroutine() ){
		g_Scheduler.CoYield();
	}
	
	return return_value;
}
