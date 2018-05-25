# phpgo

phpgo's mission is to provide a small, simple, stable extension for existing and future PHP applications for much higher concurrency, better performance, with lower resouce consumption and work well with most of the existing php features, extensions and frameworks

phpgo: A php extension that brings into php the go core features:
- go routine
- channel
- select
- mutex
- waitgroup
- timer
- go routine scheduler

phpgo also brings in a break-through feature which: 
- automatically coverts the synchronized function calls in the php extensions (e.g. PDO, redis... etc) into asynchronized 
This allows the swithing of execution to another go routine if one go routine is blocked by blocking I/O, thus provides hundred to thousand times of concurrent execution under the same running environment

phpgo can be used under both CLI mode and fast-cgi (php-fpm) mode

phpgo supports php5.5 to 7.2 on linux as of today

# 1 Getting started
## 1.1 Install libgo
The phpgo relies on the the libgo library (thanks libgo!) to provide the underlying coroutine capability, which includes:
- coroutine creation, execution and scheduling
- coroutine local storage
- coroutine listener with the task swap hook capability
- go channel
- go mutex
- hook of system calls (thus allow corroutine switching during blocking system call)

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


## 1.2 Build phpgo from source - supported on linux OS 
steps to build phpgo:
```
#git clone https://github.com/birdwyx/phpgo
#cd phpgo
#phpize
#./configure -with-php-config=<the path to php-config>
#make
#make install
```
you may probably want to do a test first to ensure everything is fine before start using of phpgo
```
#cd phpgo
#export TEST_PHP_EXECUTABLE=./test_php
#php run-tests.php tests
```
ensure there are no FAIL'ed test cases. You'll need to mannually ensure the PDO/redis works well if the related cases are skipped.

then add the following line into the php.ini
```
extension=phpgo.so
```

## 1.3 First phpgo program

hello_world.php:
```
<?php
use \go\Scheduler; //for the Scheduler functions 

go(function(){   //creates a go routine
    echo "Hello World!" . PHP_EOL;
});

Scheduler::join();  //execute all go routines
```

run this program under the command line:
```
#php hello_world.php
Hello World!
#
```

Have fun!

# 2 Go Live
###### For a better performance: install libgo with boost context support
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
