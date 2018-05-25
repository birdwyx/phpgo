# phpgo
PHPGO: A php extension that brings into php the go core features: go routines, channel, select, mutex, waitgroup, timer and scheduler... now supports php5.5 to 7.2 on linux

# Getting start
## 1.install libgo
The phpgo relies on the the libgo library to provide the underlying coroutine capability, which includes:
- coroutine creation, execution and scheduling
- coroutine local storage
- coroutine listener with the task swap hook capability
- go channel
- go mutex

(libgo contains more excellent features and you may want to check them out @ https://github.com/yyzybb537/libgo )

steps to install the libgo:
```
#git clone https://github.com/yyzybb537/libgo
#cd libgo
#git checkout master
#mkdir build
#cd build
#rm -rf *
#cmake ..
#make
#make install
#ldconfig
```

### 1.1 for a better performance: install libgo with boost context support
libgo supports using boost context for context switching (default context switching mechanism is u_context) which provides a much better coroutine switch performance (5+ times). 

If you are using the phpgo under production environment, it's recommended that you enable the boost context switching by the steps below:

Firstly, you'll need to install boost 1.59.0+, I'll suggest you use 1.59.0, since libgo is using 1.59.0 in auto-integeration testing (as of today: 2018/5/25)
```
#wget https://sourceforge.net/projects/boost/files/boost/1.59.0/boost_1_59_0.tar.gz
#tar -xvf boost_1_59_0.tar.gz
#cd boost_1_59_0
#./bootstrap.sh
#./b2 -q install
```

Then you'll need to add -DENABLE_BOOST_CONTEXT=ON option to the "cmake" command
```
#cd libgo/build
#rm -rf *
#cmake .. -DENABLE_BOOST_CONTEXT=ON
#make && make install
```
note: if you see compiler errors during the "make" step, make sure you've done the "rm -rf * " under the build directory


## 2. build phpgo from source - supported on linux OS 

steps to build phpgo:
```
#git clone https://github.com/birdwyx/phpgo
#cd phpgo
#phpize
#./configure -with-php-config=<the path to php-config>
#make
#make install
```

then add the following line into the php.ini
```
extension=phpgo.so
```

## First phpgo program

hello_world.php:
```
<?php
use \go\Scheduler;

go( function(){
    echo "Hello World!" . PHP_EOL;
});

Scheduler::join();
```
use class Sechduler under the go namespace for the Scheduler functions 
the go() function creates a go routine, this go routine is executed later by the Scheduler::join()
Scheduler::join executes all go routines (in this example there is only 1 go routine) and until all go routines returns
 
Have fun!
