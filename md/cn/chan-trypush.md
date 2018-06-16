# go\Chan::tryPush
  
\(PHP 5 >= 5.4.0, PHP 7, phpgo >= 0.9.0\)

tryPush — 尝试在管道尾部添加一条数据

## 说明
#### mixed tryPush()  
尝试在管道尾部添加一条数据

## 返回值
如果成功则返回TRUE，如果管道已满或出错则返回FALSE，如果管道关闭则返回NULL

## 参见
- [go\Chan::push](https://github.com/birdwyx/phpgo/blob/master/md/cn/chan-push.md) — 在管道尾部添加一条数据
- [go\Chan::pop](https://github.com/birdwyx/phpgo/blob/master/md/cn/chan-trypop.md) — 从管道头部读取一条数据

## 示例
### 1. 模拟push
```
<?php
use go\Chan;
use go\Runtime;

$ch = new Chan(1);
go(function() use($ch){
    echo "Alice sends a greeting\n";
    while(true){
        $ok = $ch->tryPush("Greeting from Alice\n");
        if($ok) break;
        Runtime::Gosched();
    }
});

$message=$ch->pop();
echo $message;
```
输出：
```
Alice sends a greeting
Greeting from Alice
```
