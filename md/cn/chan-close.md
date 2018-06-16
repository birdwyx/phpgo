# go\Chan::close
  
\(PHP 5 >= 5.4.0, PHP 7, phpgo >= 0.9.0\)

close — 关闭管道

## 说明
#### bool close()

管道关闭之后，资源并不立即释放，管道资源将在最后一个使用者退出时释放。

>读写关闭的管道：  
>从已关闭管道读取数据时，如果管道内还有数据，则返回该数据，如果没有数据则返回NULL；  
>向关闭的管道写入数据会失败并返回FALSE。  

## 返回值
成功则返回TRUE，失败则返回FALSE。

## 参见
- [go\Chan::pop](https://github.com/birdwyx/phpgo/blob/master/md/cn/chan-pop.md) — 从管道头部读取一条数据
- [go\Chan::push](https://github.com/birdwyx/phpgo/blob/master/md/cn/chan-push.md) — 向管道尾部添加一条数据

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
