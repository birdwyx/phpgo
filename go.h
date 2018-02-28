#ifndef __GO_H__
#define __GO_H__

/*pure functions*/
bool     phpgo_initialize();
void*    phpgo_go(zend_uint argc, zval*** args TSRMLS_DC);
void     phpgo_go_debug(unsigned long debug_flag);

#endif