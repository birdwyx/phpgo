# go\Chan::close
  
\(PHP 5 >= 5.4.0, PHP 7, phpgo >= 0.9.0\)

close — 关闭管道

## 说明
#### bool close()

管道关闭之后，资源并不立即释放，管道资源将在最后一个使用者退出时释放。

>读写关闭的管道：  
>从已关闭管道读取数据时，如果管道内还有数据，则返回该数据，如果没有数据则返回NULL；  
>向关闭的管道写入数据会失败并返回FALSE。  
>注：关闭管道时，已经在管道上阻塞等待的push()和pop()调用并不会返回。这与golang的行为是不同的：golang中一旦关闭管道，所有在管道读写的go routine都会返回。
>之所以有这个限制的原因是目前libgo正式版本尚未支持管道关闭操作，而phpgo目前并不希望修改libgo并维护一个私有的libgo版本。

## 返回值
成功则返回TRUE，失败则返回FALSE。

## 示例
### 1. 读写关闭的管道
```
<?php
use go\Chan;
use go\Scheduler;

$ch = new Chan(1);
go(function() use($ch){
    echo "go 1 try to read\n";
    $data = $ch->pop();
    echo "data read in go 1:"; var_dump($data);
});

go(function() use($ch){
    sleep(1);
    echo "go 2 close channel\n";
    $data = $ch->close();
});

Scheduler::join();
```
输出：
```
go 1 try to read
go 2 close channel
data read in go 1:
```
