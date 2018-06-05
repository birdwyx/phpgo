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

This allows the swithing of execution to another go routine if one go routine is blocked by blocking I/O, thus provides hundred to thousand times of concurrent executions under the same running environment

phpgo can be used under both CLI mode and fast-cgi (php-fpm) mode

phpgo supports php5.5 to 7.2 on linux as of today

# 0. Reference Manuals
### [Phpgo中文参考手册](https://github.com/birdwyx/phpgo/REFERENCE_CN.md) 
### [Phpgo Reference Manual](https://github.com/birdwyx/phpgo/REFERENCE_EN.md)

# 1. Getting started
## 1.1. Install libgo
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

## 1.2. Build phpgo from source - supported on linux OS 
steps to build phpgo:
```
#git clone https://github.com/birdwyx/phpgo
#cd phpgo
#phpize
#./configure -with-php-config=<the path to php-config>
#make
#make install
```

>you may probably want to do a test first to ensure everything is fine before start using phpgo
>```
>#cd phpgo
>#export TEST_PHP_EXECUTABLE=./test_php
>#php run-tests.php tests
>```
>ensure there are no FAIL'ed test cases

then, add the following line into the php.ini
```
extension=phpgo.so
```

## 1.3. First phpgo program

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

## 1.4. Using phpgo under fast-cgi mode

phpgo can be used under fast-cgi (php-fpm) mode,  following are the steps:

### Make libgo preloaded
You need to modify your php-fpm service management script (/etc/init.d/php-fpm), add the following line to the /etc/init.d/php-fpm at the very begining of the "start)" section:
```
export LD_PRELOAD=liblibgo.so
```
after your modification, the file should look like the following:
```
/etc/init.d/php-fpm:
...
case "$1" in
    start)
        echo -n "Starting php-fpm - with libgo preloaded"
        export LD_PRELOAD=liblibgo.so
        $php_fpm_BIN --daemonize $php_opts
        if [ "$?" != 0 ] ; then
            echo " failed"
            exit 1
        fi
...
```
then issue a "service php-fpm reload" to make the modification take effect
```
#service php-fpm reload
```
The reason to do that is for you to obtain the capability that allows the swithing of execution to another go routine while a go routine is I/O blocked, the LD_PRELOAD=liblibgo.so does the trick. For more information, see the dedicated section below that describes the details of what libgo has done for this capability, why the LD_PRELOAD is needed and how it functions.

### Setup your go scheduler
A typical way to setup the go scheduler is to add the Scheduler::join() into the index.php
```
index.php:
use \go\Scheduler
...
your existing code which typically boots up a framework(e.g, laravel) 
who in turns runs your web service
...

Scheduler::join(); // the last line
```

### Feel free to use the phpgo capability in your web service code
The places that you most likely to use the phpgo functions/methods are the Models / Controllers.
The following code demonstrates a uses case where an API gateway consolidate a user's basic information, order placed, and browsing history and send back to the web broswer:
```
use \go\Scheduler;
...
function getUserDetailInfo(){
    $userid = $_GET['userid']; 
    $user_details = [];
    $user_orders = [];
    $user_browsing_history = [];
    
    go(function(&$user_details){
        $redis = new Redis();
        $redis->connect(...);
        $user_details = $redis->get( "user_details_of_" . $userid );
    }, [&$user_details]);
    
    go(function(&$user_orders){
        $pdo = new PDO(...);
        $user_orders = $pdo->query("select * from tb_order where userid = $userid");
    }, [&$user_orders]);
    
    go(function(&$user_browsing_history){
        $curl = new Curl(...);
        $user_browsing_history =
            $curl->get("http://browsing.history.micro.service/path_to_the_browsing_history_query_api");
    }, [&$user_browsing_history]);
    
    Scheduler::join();
    
    $result = array(
        'user_details' = > $user_details,
        'user_orders' => $user_orders,
        'user_browsing_history' => $user_browsing_history,
    );
    
    echo json_encode($result);
}
...
```
The code above creates 3 go routines, which run in parallel getting the user basic information, order information and browsing history from redis, databases, and a microservice running http interface; the Scheduler::join() schedules and wait all the 3 go routines to finish running; then the code send back the consolidated result to the broswer

As we all know, due to the synchronized nature of php, in a php script all operations have to be exectued in sequence, in an API gateway that has a lot of interactions with other conter-parts, the total amount of execution time of a script can easily become unacceptable. By using phpgo, the amount of total execution time can reduce to the time of a single operation(given the operations are independent and can be executed in parallel)

# 2. LD_PRELOAD
to be updated...

# 3. Go Live

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

Then, you'll need to add -DENABLE_BOOST_CONTEXT=ON option to the "cmake" command
```
#cd libgo/build
#rm -rf *
#cmake .. -DENABLE_BOOST_CONTEXT=ON
#make && make install
```
note: if you see compiler errors during the "make" step, make sure you've done the "rm -rf * " under the build directory


