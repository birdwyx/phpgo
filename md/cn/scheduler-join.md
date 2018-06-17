# go\Scheduler::join
  
\(PHP 5 >= 5.4.0, PHP 7, phpgo >= 0.9.0\)

join — 运行协程调度器直到所用户协程数小于指定数量

## 说明
#### mixed join([int $remains])
  
运行协程调度器，等待所有用户协程运行完毕后返回；若指定了remains参数，则当前用户协程数小于等于remains时返回。
注意这里的“用户协程”指通过在php代码调用go/goo启动的协程；phpgo内部也可能会启动协程，这些协程不算在内。

## 参数

#### remains  
join()在用户协程数减小到remains时返回


## 返回值
返回TRUE。

## 参见
- [go\Scheduler::loop](https://github.com/birdwyx/phpgo/blob/master/md/cn/scheduler-loop.md) — 运行协程调度器直到调度线程退出
- [go\Scheduler::run](https://github.com/birdwyx/phpgo/blob/master/md/cn/scheduler-run.md) — 检查一遍所有协程，如果有就绪的就运行一次
