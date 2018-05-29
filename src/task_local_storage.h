#pragma once
#include <libgo/config.h>
#include <libgo/scheduler.h>

#define INITIAL_TASK_CAPACITY (1024 + 1)         /* 0 not used */
#define MAX_TASK_CAPACITY     (1024 * 1024 +1)   /* 0 not used */
#define INITIAL_KEY_CAPACITY  (16 + 1)           /* 0 not used */
#define MAX_KEY_CAPACITY      (1024 + 1)         /* 0 not used */

namespace co
{
	
class Freeable;
class Task;

typedef uint64_t kls_key_t;

class TaskLocalStorage{
	static std::vector<std::vector<Freeable*>>     kls; 
	static std::map<std::string, kls_key_t>             kls_key_map;
	static uint64_t                                kls_key_count;
	static std::mutex                              kls_mutex;
   
public:
	static kls_key_t CreateKey(std::string str_key);
	static bool SetSpecific(kls_key_t key, Freeable* pointer);
	
	ALWAYS_INLINE static Freeable* GetSpecific(kls_key_t key){
		//cout << "get key:" << key << endl;
		Task* tk = g_Scheduler.GetCurrentTask();
		
		if(!tk) return nullptr;
		
		auto task_id = tk->id_;
		
		if(kls.size() < task_id +1) return nullptr;
		if(kls[task_id].size() < key + 1) return nullptr;
		
		return kls[task_id][key];
	}
	
	ALWAYS_INLINE static Freeable* GetSpecific(kls_key_t key, uint64_t task_id){
		//printf( "get key: %d task %d\n", key,  task_id );
		if(kls.size() < task_id +1) return nullptr;
		if(kls[task_id].size() < key + 1) return nullptr;
		
		return kls[task_id][key];
	}
	
	static void FreeSpecifics(uint64_t);
	static void Dump();
};

template <class T>
class TaskLocalMaker {
	T*        tlv;
	kls_key_t key;
	
public:
	TaskLocalMaker<T>(std::string str_key){
		tlv = nullptr;
		key = TaskLocalStorage::CreateKey(str_key);
		if( g_Scheduler.GetCurrentTask() && !(tlv = (T*)TaskLocalStorage::GetSpecific(key)) ){
			tlv = new T();
			TaskLocalStorage::SetSpecific(key, tlv);
		}
	}
	
	ALWAYS_INLINE operator T*(){
		return tlv;
	}
	
	ALWAYS_INLINE operator T&(){
		return *tlv;
	}
};


} //namespace co