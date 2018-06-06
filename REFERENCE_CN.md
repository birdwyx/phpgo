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
### 资源类型
-
## 预定义常量
-
## phpgo函数
- [go](https://github.com/birdwyx/phpgo/md/go.md) — 创建go routine
- [goo](https://github.com/birdwyx/phpgo/md/go.md) — 创建go routine，支持可选参数
- [select](https://github.com/birdwyx/phpgo/md/select.md) — go事件选择器

## [\go\Scheduler](https://github.com/birdwyx/phpgo/md/scheduler.md) — go协程调度器类：Scheduler
- [\go\Scheduler::join](https://github.com/birdwyx/phpgo/md/scheduler-join.md) — 运行协程调度器直到所有协程运行完成
- [\go\Scheduler::loop](https://github.com/birdwyx/phpgo/md/scheduler-loop.md) — 运行协程调度器直到调度线程退出
- [\go\Scheduler::run](https://github.com/birdwyx/phpgo/md/scheduler-run.md) — 运行一次协程调度器，检查一遍所有待调度协程，如果协程就绪就运行该协程一次

## [\go\Chan](https://github.com/birdwyx/phpgo/md/chan.md) — go管道类：Chan
- [\go\Chan::close](https://github.com/birdwyx/phpgo/md/chan-close.md) — 关闭管道
- [\go\Chan::__construct](https://github.com/birdwyx/phpgo/md/chan-construct.md) — 创建管道
- [\go\Chan::__destruct](https://github.com/birdwyx/phpgo/md/chan-destruct.md) — 销毁管道
- [\go\Chan::pop](https://github.com/birdwyx/phpgo/md/chan-pop.md) — 从管道头部读取一条数据，如果没有数据则阻塞，如果管道关闭则返回NULL，如果出错则返回false
- [\go\Chan::push](https://github.com/birdwyx/phpgo/md/chan-push.md) — 在管道尾部添加一条数据，如果成功则返回true，如果管道已满、管道关闭或出错则返回false
- [\go\Chan::tryPop](https://github.com/birdwyx/phpgo/md/chan-trypop.md) — 从管道头部读取一条数据，如果管道关闭则返回NULL，如果没有数据或出错则返回false，
- [\go\Chan::tryPush](https://github.com/birdwyx/phpgo/md/chan-trypush.md) — 在管道尾部添加一条数据，如果成功则返回true，如果管道已满、管道关闭或出错则返回false

## [\go\WaitGroup](https://github.com/birdwyx/phpgo/md/waitgroup.md) — go WaitGroup类
- [\go\WaitGroup::Add](https://github.com/birdwyx/phpgo/md/chan-close.md) — 增加WaitGroup等待计数，可指定增加数量
- [\go\WaitGroup::__construct](https://github.com/birdwyx/phpgo/md/chan-construct.md) — 创建WaitGroup
- [\go\WaitGroup::__destruct](https://github.com/birdwyx/phpgo/md/chan-destruct.md) — 销毁WaitGroup
- [\go\WaitGroup::Done](https://github.com/birdwyx/phpgo/md/chan-pop.md) — WaitGroup等待计数减一，相当于WaitGroup::Add(-1)
- [\go\WaitGroup::Wait](https://github.com/birdwyx/phpgo/md/chan-push.md) — 等待WaitGroup等待计数减小到0，如果调用者为协程则当前协程挂起，如果调用者为调度线程，则运行调度器直到等待计数减小到0（tbc）

