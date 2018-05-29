#pragma once
#include "co_wait_group.h"

/*
* waitgroup
*/
class GoWaitGroup{
public:
	static void* Create(){
		return new ::co::CoWaitGroup();
	}

	static int64_t Add(void* wg, int64_t delta){
		auto wg_obj = (::co::CoWaitGroup*)wg;
		return wg_obj->Add(delta);
	}

	static int64_t Done(void* wg){
		auto wg_obj = (::co::CoWaitGroup*)wg;
		return wg_obj->Done();
	}

	static int64_t Count(void* wg){
		auto wg_obj = (::co::CoWaitGroup*)wg;
		return wg_obj->Count();
	}

	static void Wait(void* wg){
		auto wg_obj = (::co::CoWaitGroup*)wg;
		wg_obj->Wait();
	}

	static bool Destruct(void* wg){
		auto wg_obj = (::co::CoWaitGroup*)wg;
		delete wg_obj;
	}
};