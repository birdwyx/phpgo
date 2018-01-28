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
	/*pure functions*/
	bool     phpgo_initialize();
	void*    phpgo_go(zend_uint argc, zval*** args TSRMLS_DC);
	void     phpgo_go_debug(unsigned long debug_flag);
	zval*    phpgo_select(GO_SELECT_CASE* case_array, long case_count TSRMLS_DC);
	
	/*underlying functions for classes in namespace "go"*/
	void     phpgo_go_scheduler_run_once(void);
	void     phpgo_go_scheduler_run_join_all(void);
	void     phpgo_go_scheduler_run_forever(void);
#ifdef ZTS
	void     phpgo_go_scheduler_run_forever_multi_threaded(uint64_t);
#endif
   
	void*    phpgo_go_chan_create(unsigned long capacity);
	void*    phpgo_go_chan_push(void* handle, zval* z);
	zval*    phpgo_go_chan_pop(void* handle);
	bool     phpgo_go_chan_try_push(void* handle, zval* z);
	zval*    phpgo_go_chan_try_pop(void* handle);
	void     phpgo_go_chan_close(void* handle);
	void     phpgo_go_chan_destroy(void* handle);
	         

	
	void*    phpgo_go_mutex_create(bool signaled);
	void     phpgo_go_mutex_lock(void* mutex);
	void     phpgo_go_mutex_unlock(void* mutex);
	bool     phpgo_go_mutex_try_lock(void* mutex);
	bool     phpgo_go_mutex_is_lock(void* mutex);
	void     phpgo_go_mutex_destroy(void* mutex);
	
	void*    phpgo_go_wait_group_create();
	int64_t  phpgo_go_wait_group_add(void* wg, int64_t delta);
	int64_t  phpgo_go_wait_group_done(void* wg);
	int64_t  phpgo_go_wait_group_count(void* wg);
	void     phpgo_go_wait_group_wait(void* wg);
	bool     phpgo_go_wait_group_destruct(void* wg);
	
	void     phpgo_go_timer_tick(zval* z_chan, void* h_chan, uint64_t micro_seconds);
	void     phpgo_go_timer_after(zval* z_chan, void* h_chan, uint64_t micro_seconds);
	
	uint64_t phpgo_go_runtime_num_goroutine();
	void     phpgo_go_runtime_gosched();
}

#endif