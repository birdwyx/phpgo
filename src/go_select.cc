#include "stdinc.h"
#include "go_select.h"
#include "go_chan.h"
#include <libgo/coroutine.h>
/*
* select - case
*/
bool  phpgo_select(GO_SELECT_CASE* case_array, long case_count TSRMLS_DC){
    if(!case_count) return false;

    GO_SELECT_CASE* selected_case = nullptr;
    while (!selected_case) {
        //calling srand everytime will cuase the result less randomized,remove
        //srand(time(NULL)); 
        auto r = rand();
        auto start = r % case_count;
        
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
                    return false;
                }
                if(case_array[i].op == GO_CASE_OP_READ){
                    // GoChan::TryPop:
                    // if data not ready to read, return nullptr
                    // if channel is closed, return ZVAL_NULL
                    // otherwise return the read zval
                    zval* data = GoChan::TryPop(chinfo);
                    if(data) {
                        PHPGO_REPLACE_ZVAL_VALUE(&case_array[i].value, data, 1 /*invoke zval_copy_ctor*/);
                        phpgo_zval_ptr_dtor(&data);
                        PHPGO_FREE_PZVAL(data); //efree(data) on php7 and no effect on php5
                        selected_case = &case_array[i];
                        goto exit_do_while;
                    }
                }else if(case_array[i].op == GO_CASE_OP_WRITE){
                    auto rc = GoChan::TryPush(chinfo, case_array[i].value TSRMLS_CC);
                    if( rc==GoChan::RCode::success ){
                        //todo: php7 need to test the write of non-scalar value (array/string etc)
                        //phpgo_zval_add_ref(&case_array[i].value);
                        selected_case = &case_array[i];
                        goto exit_do_while;
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
        
    exit_do_while:
        PHP5_AND_BELOW(
            zval*   return_value = nullptr; 
            defer{ if(return_value) zval_ptr_dtor(&return_value); };
        );
        PHP7_AND_ABOVE( zval return_value; );
        
        if( selected_case ){
            PHPGO_ARG_TYPE* args         = nullptr;
            auto            argc         = 0;
            if(selected_case->case_type != GO_CASE_TYPE_DEFAULT){
                args    = (PHPGO_ARG_TYPE*)safe_emalloc(1, sizeof(PHPGO_ARG_TYPE), 0);
                args[0] = PHP5_VS_7(&selected_case->value, *selected_case->value);
                argc    = 1;
            }
            
            phpgo_zval_add_ref(&selected_case->callback);
            if( call_user_function_ex(
                EG(function_table), 
                NULL, 
                selected_case->callback,    // the callback callable
                &return_value,              // zval**(PHP5)/zval*(PHP7) to receive return value
                argc,                       // the parameter number required by the callback
                args,                       // the parameter list of the callback
                1, 
                NULL TSRMLS_CC
            ) != SUCCESS) {
                zend_error(E_ERROR, "phpgo: execution of go routine faild");
                //goto cleanup;
            }

            phpgo_zval_ptr_dtor(&selected_case->callback);
            if(args) efree(args);
        }else{
            if( co_sched.IsCoroutine() ){
                co_sched.CoYield();
            }else{
                co_sched.Run();
            }
        }
    }
    
    return true;
}
