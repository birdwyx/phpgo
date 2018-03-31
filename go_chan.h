#pragma once
#include <map>
#include <atomic>
#include <unistd.h>
#include "zend.h"
#include "TSRM.h"
#include "zend_API.h"
#include "zend_variables_persist.h"

#define MAX_CHAN_NAME_LEN (32)

#ifdef ZTS
/* {{{ pthread_self wrapper */
static inline THREAD_T get_current_thread_id() {
    #ifdef _WIN32
	    return (THREAD_T) GetCurrentThreadId();
    #else
	    return (THREAD_T) pthread_self();
    #endif
} /* }}} */
#endif

typedef struct{
	size_t ref_count;
	char   name[ MAX_CHAN_NAME_LEN + sizeof(uint32_t)];  // 4byte padding multi-byte string ending
	size_t name_len;
	bool   closed;
	bool   copy;   //whether or not we copy the zval data from thread local 
	               //into shared memory on channel push; copy is required
				   //when sending data to a different thread
	void*  chan;
} ChannelInfo;


struct ChannelData{
#ifdef ZTS
	THREAD_T from_thread_id;
	void***  from_tsrm_ls;
#endif
	uint64_t from_task_id;
	bool     copy;
	zval*    z;

	ChannelData(zval* zv, bool copy_flag TSRMLS_DC);
	~ChannelData(){
		if(!z) return;
		if(copy){
			zval_persistent_ptr_dtor(&z);
		}else{
			zval_ptr_dtor(&z);
		}
	}
};

class GoChan{
	static std::map<std::string, ChannelInfo*> map_name_to_chinfo;
	static std::atomic_flag chan_lock;

public:
	static void* Create(unsigned long capacity, char* name, size_t name_len, bool copy);
	static void  Destroy(void* handle);
	static void  Close(void* handle);
	static void* Push(void* handle, zval* z TSRMLS_DC);
	static zval* Pop(void* handle);
	static bool  TryPush(void* handle, zval* z TSRMLS_DC);
	static zval* TryPop(void* handle);
	
	/*convert chan zval to the channel ChannelInfo*/
	static ChannelInfo* ZvalToChannelInfo(zval* z_chan TSRMLS_DC);
};