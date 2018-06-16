# go\Chan::pop
  
\(PHP 5 >= 5.4.0, PHP 7, phpgo >= 0.9.0\)

pop — 从管道头部读取一条数据

## 说明
#### mixed pop()

## 返回值
成功则返回读取的数据，如果没有数据则阻塞。
如果管道关闭则返回NULL，如果出错则返回FALSE。

## 参见
- [go\Chan::push](https://github.com/birdwyx/phpgo/blob/master/md/cn/chan-push.md) — 在管道尾部添加一条数据，如果成功则返回TRUE，如果管道已满、管道关闭或出错则返回FALSE
- [go\Chan::tryPop](https://github.com/birdwyx/phpgo/blob/master/md/cn/chan-trypop.md) — 从管道头部读取一条数据，如果管道关闭则返回NULL，如果没有数据或出错则返回FALSE，

## 示例
### 1. 使用channel和go routine通信
```
<?php
use go\Chan;
use go\Scheduler;

$ch = new Chan(1);
go(function() use($ch){
    echo "Alice sends a greeting\n";
    $ch->push("Greeting from Alice\n");
});

$message = $ch->pop();
echo $message;
```
输出：
```
Alice sends a greeting
Greeting from Alice
```
