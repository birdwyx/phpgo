#pragma once
#include "zend.h"

#define GO_CASE_OP_READ      1
#define GO_CASE_OP_WRITE     2

#define GO_CASE_TYPE_CASE    1
#define GO_CASE_TYPE_DEFAULT 2

typedef struct{
	long  case_type;
	zval* chan;
	long  op;
	zval* value;
	zval* callback;
} GO_SELECT_CASE;

typedef struct{
	long case_count;
	GO_SELECT_CASE* case_array;
} GO_SELECTOR;


zval*  phpgo_select(GO_SELECT_CASE* case_array, long case_count TSRMLS_DC);