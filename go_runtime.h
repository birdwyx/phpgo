/*
* the go runtime library
*/
#pragma once
#include <libgo/scheduler.h>

#ifndef co_sched
#define co_sched g_Scheduler
#endif

uint64_t phpgo_go_runtime_num_goroutine();
void phpgo_go_runtime_gosched();

