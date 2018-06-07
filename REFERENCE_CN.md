# Phpgo 参考手册

## 简介
## 安装／配置
### 需求
- phpgo目前支持linux操作系统
- phpgo依赖c++协程库libgo实现底层协程特性。在安装phpgo之前，必须先安装好libgo
- libgo依赖boost context以获得最优的协程切换性能，因此在安装libgo之前建议安装boost
### 安装
#### 1. 安装boost(可选)
```
wget https://sourceforge.net/projects/boost/files/boost/1.59.0/boost_1_59_0.tar.gz
tar -xvf boost_1_59_0.tar.gz
cd boost_1_59_0
./bootstrap.sh
./b2 -q install
```

#### 2. 安装libgo
通过以下步骤安装libgo:
```
git clone https://github.com/yyzybb537/libgo
cd libgo
git checkout master
mkdir build
cd build
rm -rf *
cmake .. -DENABLE_BOOST_CONTEXT=ON  #如果未安装boost，则替换为 cmake ..
make
make install
ldconfig
```

#### 3. 安装phpgo

通过以下步骤安装phpgo:
```
git clone https://github.com/birdwyx/phpgo
cd phpgo
phpize
./configure -with-php-config=<the path to php-config>
make
make install
```

### 运行时配置
#### 1. 在php.ini 中添加如下配置
```
extension=phpgo.so
```
#### 2. 命令行方式
```
export LD_PRELOAD=liblibgo.so; php my_app.php; export LD_PRELOAD=
```
注意在运行之前设置了环境变量LD_PRELOAD。这样做得原因是你可以通过LD_PRELOAD 让你的php代码及第三方扩展中的涉及I/O操作的同步函数调用（如redis、mysql数据读写，网络读写，以及sleep/usleep等）自动“异步化”，在协程供调用这些涉及IO操作的函数时，当前协程会自动切出，将执行权利让给其他协程。

当然你也可以选择不设置LD_PRELOAD，这样你的代码及第三方扩展中的同步I/O操作会维持同步，在这些操作完成前，其他协程，甚至是调度器，在该操作完成之前不会运行。

如果你是在命令行下玩phpgo，运行完你的App之后，清除一下LD_PRELOAD，因为提前加载的libgo虽然和phpgo可以很好的配合，但未必不会跟linux下其他应用程序打架。

最好是做一个脚本，将LD_PRELOAD和你的php程序包装进去：
```
my_app.sh:
export LD_PRELOAD=liblibgo.so; 
php my_app.php; 
```
然后运行 my_app.sh 来启动你的应用程序
```
./my_app.sh&
```

#### 3. php-fpm方式
在php-fpm服务管理脚本（通常是 /etc/init.d/php-fpm）中 “start)” 一节中加入以下一行
```
export LD_PRELOAD=liblibgo.so
```
加完后，php-fpm脚本大概会长这样：
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
然后，命令行执行 service php-fpm restart重启php-fpm使改动生效
```
service php-fpm restart
```

## phpgo函数
- [go](https://github.com/birdwyx/phpgo/blob/master/md/cn/go.md) — 创建go routine
- [goo](https://github.com/birdwyx/phpgo/md/cn/goo.md) — 创建go routine，支持可选参数
- [select](https://github.com/birdwyx/phpgo/md/cn/select.md) — 进行一次事件轮询，并返回一个Selector对象

## [go\Scheduler](https://github.com/birdwyx/phpgo/md/cn/scheduler.md) — go协程调度器类：Scheduler
- [go\Scheduler::join](https://github.com/birdwyx/phpgo/md/cn/scheduler-join.md) — 运行协程调度器直到所有协程运行完成
- [go\Scheduler::loop](https://github.com/birdwyx/phpgo/md/cn/scheduler-loop.md) — 运行协程调度器直到调度线程退出
- [go\Scheduler::run](https://github.com/birdwyx/phpgo/md/cn/scheduler-run.md) — 运行一次协程调度器，检查一遍所有待调度协程，如果协程就绪就运行该协程一次

## [go\Selector](https://github.com/birdwyx/phpgo/md/cn/scheduler.md) — Selector类
- [go\Selector::__construct](https://github.com/birdwyx/phpgo/md/cn/selector-construct.md) — 创建Selector. 提供给select()调用，php代码不应直接调用Selector构造器
- [go\Selector::select](https://github.com/birdwyx/phpgo/md/cn/selector-select.md) — 进行一次事件轮询，返回本Selector对象
- [go\Selector::loop](https://github.com/birdwyx/phpgo/md/cn/selector-loop.md) — 运行事件轮询直到指定的Channel有数据加入或Channel关闭

## [go\Chan](https://github.com/birdwyx/phpgo/md/cn/chan.md) — 管道类：Chan
- [go\Chan::close](https://github.com/birdwyx/phpgo/md/cn/chan-close.md) — 关闭管道
- [go\Chan::__construct](https://github.com/birdwyx/phpgo/md/cn/chan-construct.md) — 创建管道
- [go\Chan::__destruct](https://github.com/birdwyx/phpgo/md/cn/chan-destruct.md) — 销毁管道
- [go\Chan::pop](https://github.com/birdwyx/phpgo/md/cn/chan-pop.md) — 从管道头部读取一条数据，如果没有数据则阻塞，如果管道关闭则返回NULL，如果出错则返回false
- [go\Chan::push](https://github.com/birdwyx/phpgo/md/cn/chan-push.md) — 在管道尾部添加一条数据，如果成功则返回true，如果管道已满、管道关闭或出错则返回false
- [go\Chan::tryPop](https://github.com/birdwyx/phpgo/md/cn/chan-trypop.md) — 从管道头部读取一条数据，如果管道关闭则返回NULL，如果没有数据或出错则返回false，
- [go\Chan::tryPush](https://github.com/birdwyx/phpgo/md/cn/chan-trypush.md) — 在管道尾部添加一条数据，如果成功则返回true，如果管道已满、管道关闭或出错则返回false

## [go\WaitGroup](https://github.com/birdwyx/phpgo/md/cn/waitgroup.md) — WaitGroup类
- [go\WaitGroup::Add](https://github.com/birdwyx/phpgo/md/cn/waitgroup-add.md) — 增加WaitGroup等待计数，可指定增加数量
- [go\WaitGroup::__construct](https://github.com/birdwyx/phpgo/md/cn/waitgroup-construct.md) — 创建WaitGroup
- [go\WaitGroup::__destruct](https://github.com/birdwyx/phpgo/md/cn/waitgroup-destruct.md) — 销毁WaitGroup
- [go\WaitGroup::Done](https://github.com/birdwyx/phpgo/md/cn/waitgroup-done.md) — WaitGroup等待计数减一，相当于WaitGroup::Add(-1)
- [go\WaitGroup::Wait](https://github.com/birdwyx/phpgo/md/cn/waitgroup-wait.md) — 等待WaitGroup的等待计数减小到0，调用者为协程时协程挂起，调用者为调度线程时，则运行调度器直到等待计数减为0

## [go\Mutex](https://github.com/birdwyx/phpgo/md/cn/mutex.md) — 互斥锁类：Mutex
- [go\Mutex::__construct](https://github.com/birdwyx/phpgo/md/cn/mutex-construct.md) — 创建协程互斥锁
- [go\Mutex::__destruct](https://github.com/birdwyx/phpgo/md/cn/mutex-destruct.md) — 销毁协程互斥锁
- [go\Mutex::isLock](https://github.com/birdwyx/phpgo/md/cn/mutex-islock.md) — 检查互斥锁是已被加锁
- [go\Mutex::lock](https://github.com/birdwyx/phpgo/md/cn/mutex-lock.md) — 获取互斥锁，如果互斥锁已被其让协程获取，则阻塞；获得互斥锁的协程可以重复获取而不会阻塞
- [go\Mutex::tryLock](https://github.com/birdwyx/phpgo/md/cn/mutex-trylock.md) — 获取互斥锁，获取成功返回true，如果互斥锁已被其让协程获取，返回false
- [go\Mutex::unlock](https://github.com/birdwyx/phpgo/md/cn/mutex-unlock.md) — 释放互斥锁

## [go\Time](https://github.com/birdwyx/phpgo/md/cn/time.md) — Time类
- [go\Time::after](https://github.com/birdwyx/phpgo/md/cn/time-after.md) — 创建并返回一个Channel，在指定纳秒时间后phpgo将当时的精确时间写入Channel
- [go\Time::sleep](https://github.com/birdwyx/phpgo/md/cn/time-sleep.md) — 协程挂起一段时间（单位纳秒），如果在协程外调用，则调度线程挂起相应时长
- [go\Time::tick](https://github.com/birdwyx/phpgo/md/cn/time-tick.md) — 创建并返回一个Channel，phpgo周期性地、以指定纳秒时间间隔将当时的精确时间写入Channel

## [go\Runtime](https://github.com/birdwyx/phpgo/md/cn/runtime.md) — Runtime类
- [go\Runtime::numGoroutine](https://github.com/birdwyx/phpgo/md/cn/runtime-numGoroutine.md) — 返回当前调度器管辖下的未运行完毕的协程数
- [go\Runtime::gosched](https://github.com/birdwyx/phpgo/md/cn/runtime-gosched.md) — 在协程中调用，则本协程阻塞；在协程外调用，则运行一次协程调度器
- [go\Runtime::goid](https://github.com/birdwyx/phpgo/md/cn/runtime-goid.md) — 返回当前协程id，若在携程外调用，返回0
