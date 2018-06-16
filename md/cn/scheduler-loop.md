# go\Selector::loop
  
\(PHP 5 >= 5.4.0, PHP 7, phpgo >= 0.9.0\)

loop — 持续进行事件轮询，直到指定channel可读或关闭

## 说明
#### mixed loop(Chan $channel)
  
循环进行事件轮询，直到指定channel可读或关闭

## 返回值
如果轮询终止原因是指定channel可读，则返回读到的数据；
如果轮询终止原因是指定channel关闭，则返回NULL。

## 参见
- [go\Selector::select](https://github.com/birdwyx/phpgo/blob/master/md/cn/selector-select.md) — 进行一次事件轮询

## 示例
### 1. loop常规用法
```
<?php
use \Go\Chan;
use \Go\Time;
use \Go\Scheduler;

$ch1 = new Chan(["capacity"=>1]);
$ch2 = new Chan(["capacity"=>1]);
$exit = new Chan(0);

go(function() use($ch1, $ch2, $exit){
    select(
        [
            'case', $ch1, function($value){
                echo "go 0: receive from ch1: " . $value . PHP_EOL;
            }
        ],
        [
            'case', $ch2, function($value){
                echo "go 0: receive from ch2: " . $value . PHP_EOL;
            }
        ]
    )->loop($exit);
});

go(function() use($ch1, $exit){
    $i = 0;
    while( $exit->tryPop() !== NULL ){ //===NULL: channel closed
        echo "go 1: push $i\n";
        $ch1->push($i++);
    }
});

go(function() use($ch2, $exit){
    $i = 0;
    while( $exit->tryPop() !== NULL ){
        echo "go 2: push $i\n";
        $ch2->push($i++);
    }
});

go(function() use($exit){
    echo "main: now sleep 5ms\n";
    Time::sleep(5*1000*1000);
    echo "main: now close the exit channel\n";
    $exit->close();
});

Scheduler::join();
```
输出：
```
go 1: push 0
go 1: push 1
go 2: push 0
go 2: push 1
main: now sleep 5ms
go 0: receive from ch1: 0
go 0: receive from ch2: 0
go 1: push 2
go 2: push 2
go 0: receive from ch2: 1
go 0: receive from ch1: 1
go 2: push 3
go 1: push 3
go 0: receive from ch1: 2
go 0: receive from ch2: 2
go 1: push 4
go 2: push 4
go 0: receive from ch1: 3
go 0: receive from ch2: 3
go 1: push 5
go 2: push 5
go 0: receive from ch1: 4
go 0: receive from ch2: 4
go 1: push 6
go 2: push 6
go 0: receive from ch2: 5
go 0: receive from ch1: 5
go 2: push 7
go 1: push 7
go 0: receive from ch2: 6
go 0: receive from ch1: 6
go 2: push 8
go 1: push 8
go 0: receive from ch1: 7
go 0: receive from ch2: 7
go 1: push 9
go 2: push 9
go 0: receive from ch1: 8
go 0: receive from ch2: 8
go 1: push 10
go 2: push 10
go 0: receive from ch2: 9
go 0: receive from ch1: 9
main: now close the exit channel
go 0: receive from ch2: 10
```
