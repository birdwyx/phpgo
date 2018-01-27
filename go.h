#ifndef __GO_H__
#define __GO_H__

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

typedef struct{
	bool closed;
	void* chan;
} CHANNEL_INFO;
	
extern "C" {
	bool     phpgo_initialize();
	void     php_api_go_debug(unsigned long debug_flag);
	void*    php_api_go(zend_uint argc, zval*** args TSRMLS_DC);
	
	void     php_api_go_schedule_once(void);
	void     php_api_go_schedule_all(void);
	void     php_api_go_schedule_forever(void);
	         
	void*    php_api_go_chan_create(unsigned long capacity);
	void*    php_api_go_chan_push(void* handle, zval* z);
	zval*    php_api_go_chan_pop(void* handle);
	bool     php_api_go_chan_try_push(void* handle, zval* z);
	zval*    php_api_go_chan_try_pop(void* handle);
	void     php_api_go_chan_close(void* handle);
	void     php_api_go_chan_destroy(void* handle);
	         
	zval*    php_api_go_select(GO_SELECT_CASE* case_array, long case_count TSRMLS_DC);
	
	void*    php_api_go_mutex_create(bool signaled);
	void     php_api_go_mutex_lock(void* mutex);
	void     php_api_go_mutex_unlock(void* mutex);
	bool     php_api_go_mutex_try_lock(void* mutex);
	bool     php_api_go_mutex_is_lock(void* mutex);
	void     php_api_go_mutex_destroy(void* mutex);
	
	void*    php_api_go_waitgroup_create();
	int64_t  php_api_go_waitgroup_add(void* wg, int64_t delta);
	int64_t  php_api_go_waitgroup_done(void* wg);
	int64_t  php_api_go_waitgroup_count(void* wg);
	void     php_api_go_waitgroup_wait(void* wg);
	bool     php_api_go_waitgroup_destroy(void* wg);
	
	void     php_api_go_timer_tick(zval* z_chan, void* h_chan, uint64_t microseconds);
	void     php_api_go_timer_after(zval* z_chan, void* h_chan, uint64_t microseconds);
	
	uint64_t php_api_go_runtime_num_goroutine();
	void     php_api_go_runtime_gosched();
}

#endif