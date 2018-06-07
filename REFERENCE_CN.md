# Phpgo 参考手册

## 简介
## 安装／配置
### 需求
- phpgo目前支持linux操作系统
- phpgo依赖c++协程库libgo实现底层协程特性。在安装phpgo之前，必须先安装好libgo。
### 安装
- 安装libgo
- 安装phpgo
### 运行时配置
- 在php.ini 中添加如下配置
```
extension=phpgo.so
```
## phpgo函数md/cn
- [go](https://github.com/birdwyx/phpgo/md/cn/go.md) — 创建go routine
- [goo](https://github.com/birdwyx/phpgo/md/cn/goo.md) — 创建go routine，支持可选参数
- [select](https://github.com/birdwyx/phpgo/md/cn/select.md) — 进行一次事件轮询，并返回一个Selector对象

## [\go\Scheduler](https://github.com/birdwyx/phpgo/md/cn/scheduler.md) — go协程调度器类：Scheduler
- [\go\Scheduler::join](https://github.com/birdwyx/phpgo/md/cn/scheduler-join.md) — 运行协程调度器直到所有协程运行完成
- [\go\Scheduler::loop](https://github.com/birdwyx/phpgo/md/cn/scheduler-loop.md) — 运行协程调度器直到调度线程退出
- [\go\Scheduler::run](https://github.com/birdwyx/phpgo/md/cn/scheduler-run.md) — 运行一次协程调度器，检查一遍所有待调度协程，如果协程就绪就运行该协程一次

## [\go\Selector](https://github.com/birdwyx/phpgo/md/cn/scheduler.md) — Selector类
- [\go\Selector::__construct](https://github.com/birdwyx/phpgo/md/cn/selector-construct.md) — 创建Selector. 提供给select()调用，php代码不应直接调用Selector构造器
- [\go\Selector::select](https://github.com/birdwyx/phpgo/md/cn/selector-select.md) — 进行一次事件轮询，返回本Selector对象
- [\go\Selector::loop](https://github.com/birdwyx/phpgo/md/cn/selector-loop.md) — 运行事件轮询直到指定的Channel有数据加入或Channel关闭

## [\go\Chan](https://github.com/birdwyx/phpgo/md/cn/chan.md) — 管道类：Chan
- [\go\Chan::close](https://github.com/birdwyx/phpgo/md/cn/chan-close.md) — 关闭管道
- [\go\Chan::__construct](https://github.com/birdwyx/phpgo/md/cn/chan-construct.md) — 创建管道
- [\go\Chan::__destruct](https://github.com/birdwyx/phpgo/md/cn/chan-destruct.md) — 销毁管道
- [\go\Chan::pop](https://github.com/birdwyx/phpgo/md/cn/chan-pop.md) — 从管道头部读取一条数据，如果没有数据则阻塞，如果管道关闭则返回NULL，如果出错则返回false
- [\go\Chan::push](https://github.com/birdwyx/phpgo/md/cn/chan-push.md) — 在管道尾部添加一条数据，如果成功则返回true，如果管道已满、管道关闭或出错则返回false
- [\go\Chan::tryPop](https://github.com/birdwyx/phpgo/md/cn/chan-trypop.md) — 从管道头部读取一条数据，如果管道关闭则返回NULL，如果没有数据或出错则返回false，
- [\go\Chan::tryPush](https://github.com/birdwyx/phpgo/md/cn/chan-trypush.md) — 在管道尾部添加一条数据，如果成功则返回true，如果管道已满、管道关闭或出错则返回false

## [\go\WaitGroup](https://github.com/birdwyx/phpgo/md/cn/waitgroup.md) — WaitGroup类
- [\go\WaitGroup::Add](https://github.com/birdwyx/phpgo/md/cn/waitgroup-add.md) — 增加WaitGroup等待计数，可指定增加数量
- [\go\WaitGroup::__construct](https://github.com/birdwyx/phpgo/md/cn/waitgroup-construct.md) — 创建WaitGroup
- [\go\WaitGroup::__destruct](https://github.com/birdwyx/phpgo/md/cn/waitgroup-destruct.md) — 销毁WaitGroup
- [\go\WaitGroup::Done](https://github.com/birdwyx/phpgo/md/cn/waitgroup-done.md) — WaitGroup等待计数减一，相当于WaitGroup::Add(-1)
- [\go\WaitGroup::Wait](https://github.com/birdwyx/phpgo/md/cn/waitgroup-wait.md) — 等待WaitGroup的等待计数减小到0，调用者为协程时协程挂起，调用者为调度线程时，则运行调度器直到等待计数减为0

## [\go\Mutex](https://github.com/birdwyx/phpgo/md/cn/mutex.md) — 互斥锁类：Mutex
- [\go\Mutex::__construct](https://github.com/birdwyx/phpgo/md/cn/mutex-construct.md) — 创建协程互斥锁
- [\go\Mutex::__destruct](https://github.com/birdwyx/phpgo/md/cn/mutex-destruct.md) — 销毁协程互斥锁
- [\go\Mutex::isLock](https://github.com/birdwyx/phpgo/md/cn/mutex-islock.md) — 检查互斥锁是已被加锁
- [\go\Mutex::lock](https://github.com/birdwyx/phpgo/md/cn/mutex-lock.md) — 获取互斥锁，如果互斥锁已被其让协程获取，则阻塞；获得互斥锁的协程可以重复获取而不会阻塞
- [\go\Mutex::tryLock](https://github.com/birdwyx/phpgo/md/cn/mutex-trylock.md) — 获取互斥锁，获取成功返回true，如果互斥锁已被其让协程获取，返回false
- [\go\Mutex::unlock](https://github.com/birdwyx/phpgo/md/cn/mutex-unlock.md) — 释放互斥锁

## [\go\Time](https://github.com/birdwyx/phpgo/md/cn/time.md) — Time类
- [\go\Time::after](https://github.com/birdwyx/phpgo/md/cn/time-after.md) — 创建并返回一个Channel，在指定纳秒时间后phpgo将当时的精确写入Channel
- [\go\Time::sleep](https://github.com/birdwyx/phpgo/md/cn/time-sleep.md) — 进行一次事件轮询，返回本Selector对象
- [\go\Time::tick](https://github.com/birdwyx/phpgo/md/cn/time-tick.md) — 创建并返回一个Channel，phpgo周期性地、以指定纳秒时间间隔将当时的精确时间写入Channel

## [\go\Runtime](https://github.com/birdwyx/phpgo/md/cn/runtime.md) — Runtime类
- [\go\Runtime::numGoroutine](https://github.com/birdwyx/phpgo/md/cn/runtime-numGoroutine.md) — 返回当前调度器管辖下的未运行完毕的协程数
- [\go\Runtime::gosched](https://github.com/birdwyx/phpgo/md/cn/runtime-gosched.md) — 在协程中调用，则本协程阻塞；在协程外调用，则运行一次协程调度器
- [\go\Runtime::goid](https://github.com/birdwyx/phpgo/md/cn/runtime-goid.md) — 返回当前协程id，若在携程外调用，返回0
