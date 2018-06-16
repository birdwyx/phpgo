# go\Chan::tryPop
  
\(PHP 5 >= 5.4.0, PHP 7, phpgo >= 0.9.0\)

tryPop — 尝试从管道头部读取一条数据

## 说明
#### mixed tryPop()

## 返回值
尝试从管道头部读取一条数据。如果管道关闭则返回NULL，如果没有数据或出错则返回FALSE。

## 参见
- [go\Chan::push](https://github.com/birdwyx/phpgo/blob/master/md/cn/chan-push.md) — 在管道尾部添加一条数据
- [go\Chan::pop](https://github.com/birdwyx/phpgo/blob/master/md/cn/chan-trypop.md) — 从管道头部读取一条数据

## 示例
### 1. 模拟pop
```
<?php
use go\Chan;
use go\Scheduler;

$ch = new Chan(1);
go(function() use($ch){
    echo "Alice sends a greeting\n";
    $ch->push("Greeting from Alice\n");
});

while(!($message=$ch->tryPop()))  Scheduler::run();
echo $message;
```
输出：
```
Alice sends a greeting
Greeting from Alice
```
