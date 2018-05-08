#ifndef __GO_H__
#define __GO_H__

enum GoRoutineOptions{
	// change of http globals in children go routines does not 
	// affect those of parent go routine
	gro_default              = 0x0,
	gro_isolate_http_globals = 0x1
};

/*pure functions*/
bool     phpgo_initialize();
void     phpgo_go_debug(unsigned long debug_flag);
void*    phpgo_go(uint64_t go_routine_options, uint32_t stack_size, zend_uint argc, PHPGO_ARG_TYPE *args TSRMLS_DC);

#endif