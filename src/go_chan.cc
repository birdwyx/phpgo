#include "stdinc.h"
#include "go_chan.h"
#include "defer.h"
#include <libgo/coroutine.h>

std::atomic_flag GoChan::chan_lock = ATOMIC_FLAG_INIT;
std::map<std::string, ChannelInfo*> GoChan::map_name_to_chinfo;

ChannelData::ChannelData(zval* zv, bool copy_flag TSRMLS_DC){
#ifdef ZTS
	from_thread_id = tsrm_thread_id();
#endif
	from_task_id   = co_sched.GetCurrentTaskID();
	copy           = copy_flag;
	z              = nullptr;
	
	if(copy){
		PHPGO_ALLOC_PERMANENT_ZVAL(z);      // allocate using malloc()
		PHPGO_INIT_PZVAL_COPY(z, zv);         // copy the zval to shared memory
		zval_persistent_copy_ctor(z);
	}else{
#if PHP_MAJOR_VERSION < 7
		z = zv;
		phpgo_zval_add_ref(&zv);
#else
		// php7: the zv pointed to zval in stack and cannot be used 
	    // for sharing, make a new zval in heap
		zval* new_z;
		PHPGO_ALLOC_INIT_ZVAL(new_z);
		PHPGO_MAKE_COPY_ZVAL(&zv, new_z);
		z = new_z;
#endif
	}
}

void* GoChan::Create(unsigned long capacity, char* name, size_t name_len, bool copy){
	ChannelInfo* chinfo = nullptr;
	if(name && name_len){
	#ifdef ZTS
		while (chan_lock.test_and_set(std::memory_order_acquire));
		defer{
			chan_lock.clear(std::memory_order_release); 
		};	
	#endif
		char name_copy[MAX_CHAN_NAME_LEN + sizeof(uint32_t)];
		
		if( name_len > MAX_CHAN_NAME_LEN ){
			memcpy( name_copy, name, MAX_CHAN_NAME_LEN );
			*(uint32_t*)( (char*)name_copy + MAX_CHAN_NAME_LEN ) = 0;
			name = (char*)name_copy;
			name_len = MAX_CHAN_NAME_LEN;
		}
		chinfo = map_name_to_chinfo[name];
		
		if( !chinfo ){
			// new channel with name
			//channels are allocated in shared memory (not thread local) 
			//so that multiple threads can read from the same channel
			chinfo = new ChannelInfo();
			
			//the following won't work since map is a map[char*] and address of 
			//name may be reused causing differnt names mapping to the same chinfo:
			//map_name_to_chinfo[name] = chinfo 
			
			map_name_to_chinfo[name] = chinfo;
			memcpy( chinfo->name, name, name_len );
			*(uint32_t*)( (char*)chinfo->name + name_len ) = 0;
			chinfo->name_len   = name_len;
			chinfo->ref_count  = 1;
			chinfo->closed     = false;
			chinfo->copy       = copy;
			chinfo->chan       = new co_chan<ChannelData*>(capacity);
		}else{
			//channel exists;
			++chinfo->ref_count;
			return chinfo;
		}
	}else{
		// new chan with empty name
		chinfo             = new ChannelInfo();
		memset(chinfo->name, 0, sizeof(chinfo->name));
		chinfo->name_len   = 0;
		chinfo->ref_count  = 1;
		chinfo->closed     = false;
		chinfo->copy       = copy;
		chinfo->chan       = new co_chan<ChannelData*>(capacity);	
	}

	return chinfo;
}

void GoChan::Destroy(void* handle){
	
	//assert(handle);
	
	ChannelInfo* chinfo = (ChannelInfo*)handle;

#ifdef ZTS
	while (chan_lock.test_and_set(std::memory_order_acquire));
	defer{
		chan_lock.clear(std::memory_order_release); 
	};
#endif

	auto count = --chinfo->ref_count;
	if( count == 0 ){
		if( chinfo->name_len ){
			map_name_to_chinfo.erase(chinfo->name);
		}
		if(chinfo->chan){
			// pop all data from channel, and free them
			ChannelData* cd = nullptr;
			while( ( (co_chan<ChannelData*>*)(chinfo->chan) )->TryPop(cd) ){
				if(cd) delete cd;
			}
			
			delete (co_chan<ChannelData*>*)(chinfo->chan);
		}
		delete chinfo;
	}
}

bool GoChan::Close(void* handle){
	//assert(handle);
	
	ChannelInfo* chinfo = (ChannelInfo*)handle;
	if( chinfo->closed ) return false;
	
	chinfo->closed = true;
	return true;
}

GoChan::RCode GoChan::Push(void* handle, zval* z TSRMLS_DC){
	
	ChannelInfo* chinfo = (ChannelInfo*)handle;
	
	// cannot write to a closed channel
	if(chinfo->closed)
		return GoChan::RCode::channel_closed;
	
	auto cd = new ChannelData(z, chinfo->copy TSRMLS_CC);
	*( (co_chan<ChannelData*>*)chinfo->chan ) << cd;
	
	return GoChan::RCode::success;
}

zval* GoChan::Pop(void* handle){
	
	zval* z         = nullptr; 
	ChannelData* cd = nullptr;
	
	defer{
		if(cd) delete cd;
	};
	
	ChannelInfo* chinfo = (ChannelInfo*)handle;
	co_chan<ChannelData*>* ch = (co_chan<ChannelData*>*)chinfo->chan;
	
	// to conform to the google go channel:
	// if a chan is closed, return immediately
	// this will allow a master routine to close a channel which is read by multiple 
	// slave routines, thus allow the master to broadcast a signal (typically treated 
	// as close signal) to all slave routines
	if(chinfo->closed){
		bool ok = false;
		if (!co_sched.IsCoroutine()) {
			auto pop = false;
			go[ch,&cd,&ok,&pop]{
				ok  = ch->TryPop(cd);
				pop = true;
			};
			while(!pop){
				co_sched.Run();
			}
		}else{
			ok = ch->TryPop(cd);
		}
		
		if( !ok || !cd ){
			//channel closed, and no data available in channel
			PHPGO_ALLOC_INIT_ZVAL(z);
			ZVAL_NULL(z);
			return z;
		}else{
			//data is available in this close channel,
			//should return the data
			//fall through...
		}
	}else{
		// if it's not in any go routine,
		// run the scheduler (so that the go routines will run and may
		// push to the channel) until successfully read from the channel 
		if (!co_sched.IsCoroutine()) {
			go[ch, &cd]{
				*ch >> cd;
			};
			while(!cd){
				co_sched.Run();
			}
		}else{
			// in a go routine, it's safe for a blocking read
			*ch >> cd;
		}
		
		if( !cd || !cd->z ){
			PHPGO_ALLOC_INIT_ZVAL(z);
			ZVAL_NULL(z);
			return z;
		}
	}
	
	if(cd->copy){
		//this zval was copied from the sending thread's local storage 
		//to the persistent memeory, now we copy it from persistent to
		//our thread local
		
		PHPGO_ALLOC_ZVAL(z);
		*z = *(cd->z);
		PHPGO_INIT_PZVAL(z);  //init reference
		
		//copy zval from persistent to thread local
		//this is a complete copy, there won't be any pointer still hangs into 
		//the share memory
		zval_persistent_to_local_copy_ctor(z);
	}else{
		//z = cd->z;
		//phpgo_zval_add_ref(&z);
		PHPGO_ALLOC_ZVAL(z);
		*z = *(cd->z);
		PHPGO_INIT_PZVAL(z);  //init reference
		
		zval_copy_ctor(z);
	}
	
	return z;
}

GoChan::RCode GoChan::TryPush(void* handle, zval* z TSRMLS_DC){
	
	ChannelInfo* chinfo = (ChannelInfo*)handle;
	
	// cannot write to a closed channel
	if(chinfo->closed)
		return GoChan::RCode::channel_closed;
	
	auto cd = new ChannelData(z, chinfo->copy TSRMLS_CC);
	auto ch = (co_chan<ChannelData*>*)chinfo->chan;
	
	if( !ch->TryPush(cd) ){
		delete cd;
		return GoChan::RCode::channel_not_ready;
	}

	return GoChan::RCode::success;
}

zval* GoChan::TryPop(void* handle){
	
	zval* z             = nullptr;
	ChannelData* cd     = nullptr;
	ChannelInfo* chinfo = (ChannelInfo*)handle;
	
	defer{
		if(cd) delete cd;
	};
	
	auto ch = (co_chan<ChannelData*>*)chinfo->chan;
	
	if(chinfo->closed){
		if( !ch->TryPop(cd) || !cd ){
			//channel closed, and no data available in channel
			PHPGO_ALLOC_INIT_ZVAL(z);
			ZVAL_NULL(z);
			return z;
		}else{
			//data is available in this close channel,
			//should return the data
			//fall through...
		}
	}else{
		// return nullptr means channel is not ready to read
		if( !ch->TryPop(cd) || !cd )
			return nullptr;
	}
	
	if(cd->copy){
		//this zval was copied from the sending thread's local storage 
		//to the persistent memeory, now we copy it from persistent to
		//our thread local
		
		PHPGO_ALLOC_ZVAL(z);
		*z = *(cd->z);
		PHPGO_INIT_PZVAL(z);  //init reference
		
		//copy zval from persistent to thread local
		//this is a complete copy, there won't be any pointer still hangs into 
		//the share memory
		zval_persistent_to_local_copy_ctor(z);
		
	}else{
		//z = cd->z;
		//phpgo_zval_add_ref(&z);
		
		PHPGO_ALLOC_ZVAL(z);
		*z = *(cd->z);
		PHPGO_INIT_PZVAL(z);  //init reference
		
		zval_copy_ctor(z);
	}
	
	return z;
}

/*convert chan zval to the channel ChannelInfo*/
ChannelInfo* GoChan::ZvalToChannelInfo(zval* z_chan TSRMLS_DC){
	extern zend_class_entry* ce_go_chan_ptr;
	
	auto z_handler = phpgo_zend_read_property(ce_go_chan_ptr, z_chan, "handle",   sizeof("handle") - 1, true TSRMLS_CC);
	if( !z_handler || Z_TYPE_P(z_handler) == IS_NULL ){
		return nullptr;
	}
	
	auto chinfo = (ChannelInfo*) Z_LVAL_P(z_handler);
	
	return chinfo;
}
