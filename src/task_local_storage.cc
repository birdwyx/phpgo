#include "task_local_storage.h"
#include "freeable.h"
#include <iostream>
#include <unistd.h>

using namespace std;

namespace co {
	
map<string, kls_key_t>        TaskLocalStorage::kls_key_map;
uint64_t                      TaskLocalStorage::kls_key_count = 0;
vector<vector<Freeable*>>     TaskLocalStorage::kls; 
std::mutex                    TaskLocalStorage::kls_mutex;

kls_key_t TaskLocalStorage::CreateKey(string str_key){
	kls_key_t key;
	if( (key = kls_key_map[str_key]) ){
		return key;
	}else{
		std::lock_guard<std::mutex> guard(TaskLocalStorage::kls_mutex);
		kls_key_map[str_key] = ++kls_key_count;
		return kls_key_count;
	}
}
	
bool TaskLocalStorage::SetSpecific(kls_key_t key, Freeable* pointer){
	Task* tk = g_Scheduler.GetCurrentTask();
	
	if(!tk) return false;
	
	auto task_id = tk->id_;
	//note: task id starts from 1
	if(task_id >= MAX_TASK_CAPACITY) return false;
	
	//cout << "task:" << tk << " id:" << task_id << " size:" << kls.size() << " cap:" << kls.capacity() <<endl;
	
	{   // lock section
		// auto increase task capacity
		std::lock_guard<std::mutex> guard(TaskLocalStorage::kls_mutex);
		auto cap = kls.capacity();
		if( !cap ){
			// first time
			kls.reserve( INITIAL_TASK_CAPACITY );
		}else if( cap < task_id + 1 ){
			// extend of capacity required
			if( task_id < MAX_TASK_CAPACITY ){
				kls.reserve( 2 * task_id > MAX_TASK_CAPACITY ? MAX_TASK_CAPACITY : 2 * task_id );
			}else{
				//task id out of range
				return false;
			}
		}else{
			// capacity ok, just fall-through
			
		}
	
		auto sz = kls.size(); cap = kls.capacity();
		while( sz++ < cap ) {
			kls.push_back( vector<Freeable*>() );
		}
		// lock section
	}
	
	//cout << "task:" << tk << " id:" << tk->id_ << " size:" << kls.size() << " cap:" << kls.capacity() <<endl;
	
	// auto increase key capacity
	auto& task_kls = kls[task_id];
	auto cap = task_kls.capacity();
	if( !cap ){
		// first time
		task_kls.reserve( INITIAL_KEY_CAPACITY );
	}else if( cap < key + 1 ){
		// extend of capacity required
		if( key < MAX_KEY_CAPACITY ){
			task_kls.reserve( 2 * key > MAX_KEY_CAPACITY ? MAX_KEY_CAPACITY : 2 * key );
		}else{
			//task id out of range
			return false;
		}
	}else{
		// capacity ok, just fall-through
		
	}
	
	auto sz = task_kls.size(); cap = task_kls.capacity();
	while( sz++ < cap ) {
		//cout << sz << endl;
		task_kls.push_back( nullptr );
	}
	//cout << "keys capacity:" << task_kls.capacity() << " size:" << task_kls.size() << endl;
	//cout << "task:" << tk << " id:" << tk->id_ << " size:" << kls.size() << " cap:" << kls.capacity() <<endl;
	
	// if the slot currently being used, clear it first
	if( task_kls[key] ){
		task_kls[key]->Free();
	}
	
	task_kls[key] = pointer;   // note: directly assignment wont change the kls[task_id][key].size (thus always 0)
	
	
	//cout << "keys capacity:" << task_kls.capacity() << " size:" << task_kls.size() << endl;
	//cout << "task:" << tk << " id:" << tk->id_ << " size:" << kls.size() << " cap:" << kls.capacity() <<endl;
	
	
	return true;
}
	
void TaskLocalStorage::FreeSpecifics(uint64_t task_id){
	//cout << "TaskLocalStorage::FreeSpecifics " << task_id;

	auto size = kls.size();
	
	if( size < task_id + 1 )
		return;
	
	auto& task_kls = kls[task_id];
	
	for( auto& pointer:task_kls ){
		if(pointer){
			pointer->Free();
			pointer = nullptr;
		}
	}
}
	
void TaskLocalStorage::Dump(){
	
	for( auto& task_kls: kls ){
		for( auto& pointer:task_kls){
			cout << pointer << ",";
		}
	}
	cout << endl;
	
}

} //namespace co
