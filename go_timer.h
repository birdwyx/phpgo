#pragma once
#include <stdint.h>
#include "zend.h"
#include "TSRM.h"

class GoTimer{
public:
	static void Tick(zval* z_chan, void* h_chan, uint64_t micro_seconds TSRMLS_DC);
	static void After(zval* z_chan, void* h_chan, uint64_t micro_seconds TSRMLS_DC);
};