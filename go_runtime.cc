/*
* the go runtime library
*/
#include "go_runtime.h"

uint64_t phpgo_go_runtime_num_goroutine(){
	return co_sched.TaskCount();
}

void phpgo_go_runtime_gosched(){
	if(co_sched.IsCoroutine()){
		co_sched.CoYield();
	}else{
		co_sched.Run();
	}
}
