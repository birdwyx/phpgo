# phpgo
PHPGO: A php extension that brings into php the go core features: go routines, channel, select, mutex, waitgroup, timer and scheduler... now supports php5.5 to 7.2 on linux

# Getting start
## 1.install libgo

The phpgo relies on the the libgo library to provide the underlying coroutine capability, which includes:
##### - coroutine creation, execution and scheduling
##### - coroutine local storage
##### - coroutine listener with the task swap hook capability
##### - go channel
##### - go mutex
(libgo contains more excellent features and you may be want to check them out at https://github.com/yyzybb537/libgo )

steps to install the libgo:
```
#git clone https://github.com/yyzybb537/libgo
#cd libgo
#git checkout master
#mkdir build
#cd build
#cmake .. -DENABLE_BOOST_CONTEXT=ON
#make
#make install
#ldconfig
```

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
//use class Sechduler under the go namespace for the Scheduler functions 
use \go\Scheduler; 

// the go() function creates a go routine, this go routine is executed later 
// by the Scheduler::join()
go( function(){
    echo "Hello World!" . PHP_EOL;
});

//Scheduler::join executes all go routines (in this example there is only 1 go routine ) 
//and until all go routines returns
Scheduler::join();  

```

Have fun!
