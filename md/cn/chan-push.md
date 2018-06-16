# go\Chan::push
  
\(PHP 5 >= 5.4.0, PHP 7, phpgo >= 0.9.0\)

push — 在管道尾部添加一条数据

## 说明
#### bool push ( mixed $data )  
在管道尾部添加一条数据

## 参数
#### data

要向channel写入的数据

## 返回值
成功返回TURE；
如果管道已满则阻塞；
如果管道关闭则返回NULL；
出错则返回FALSE。

## 参见
- [go\Chan::tryPush](https://github.com/birdwyx/phpgo/blob/master/md/cn/chan-trypush.md) — 尝试在管道尾部添加一条数据，不阻塞
- [go\Chan::pop](https://github.com/birdwyx/phpgo/blob/master/md/cn/chan-pop.md) — 从管道头部读取一条数据

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
