//g++ -std=c++11 -llibgo test_mutex.cc

#include <mutex>
#include "libgo/coroutine.h"
#include "mutex.h"
#include <iostream>
using namespace std;

bool f(){
	return true;
}
int main()
{
	co::CoRecursiveMutex m;
	
	go [=]()mutable{
		m.lock();
		m.lock();
		m.lock();
		m.unlock();
		m.unlock();
		m.unlock();
		cout << "g1" <<endl;
	};
	
	//m.try_lock();
	cout << "main, try_lock=" << m.try_lock() << endl;
	//cout << "main, try_lock=" << m.try_lock() << endl;
	m.unlock();
	
	cout << "about to sched go routine" << endl;
	
	co_sched.RunUntilNoTask();
	
	cout << "exit" <<endl;
	
	return 0;
	
}