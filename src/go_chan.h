#pragma once
#include <map>
#include <atomic>
#include <unistd.h>
#include "zend.h"
#include "TSRM.h"
#include "zend_API.h"
#include "zend_variables_persist.h"

#define MAX_CHAN_NAME_LEN (32)

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
#endif
	uint64_t from_task_id;
	bool     copy;
	zval*    z;

	ChannelData(zval* zv, bool copy_flag TSRMLS_DC);
	~ChannelData(){
		if(!z) return;
		if(copy){
			zval_persistent_ptr_dtor(&z);
			//php7: the dtor won't free the z for us
			//for a permenent z, we've finished using it: it had been copied to
			//thread local heap in GoChan::Pop()/TryPop()
			//free it here
			//note: PHPGO_FREE_PERMENENT_PZVAL() has no effect in php5
			PHPGO_FREE_PERMENENT_PZVAL(z);
		}else{
			phpgo_zval_ptr_dtor(&z);
			//php7: the dtor won't free the z for us
			//note: PHPGO_FREE_PZVAL() has no effect in php5
			PHPGO_FREE_PZVAL(z);
		}
	}
};

class GoChan{
	static std::map<std::string, ChannelInfo*> map_name_to_chinfo;
	static std::atomic_flag chan_lock;
public:
	enum RCode{
		success           = 0,
		channel_closed    = 1,
		channel_not_ready = 2
	};
	
public:
	static void*  Create(unsigned long capacity, char* name, size_t name_len, bool copy);
	static void   Destroy(void* handle);
	static bool   Close(void* handle);
	static RCode  Push(void* handle, zval* z TSRMLS_DC);
	static zval*  Pop(void* handle);
	static RCode  TryPush(void* handle, zval* z TSRMLS_DC);
	static zval*  TryPop(void* handle);
	
	/*convert chan zval to the channel ChannelInfo*/
	static ChannelInfo* ZvalToChannelInfo(zval* z_chan TSRMLS_DC);
};