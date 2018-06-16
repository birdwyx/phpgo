# go\Selector::select
  
\(PHP 5 >= 5.4.0, PHP 7, phpgo >= 0.9.0\)

select — 进行一次事件轮询，并返回selector对象本身

## 说明
#### Selector select()
  
go\Selector::select的行为与\select()行为一致。

## 返回值
返回selector对象本身

## 参见
- [select](https://github.com/birdwyx/phpgo/blob/master/md/cn/select.md) — 进行一次事件轮询

## 示例
### 1. select常规用法
```
<?php
use \Go\Chan;
use \Go\Time;
use \Go\Scheduler;

$ch1 = new Chan(["capacity"=>1]);
$ch2 = new Chan(["capacity"=>1]);
$exit = new Chan(0);

go(function() use($ch1, $ch2, $exit){
    $selector = select(
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
    );
    
    echo "go 0: after 1st select\n";
    
    while( $exit->tryPop() !== NULL ) $selector->select();
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
go 0: receive from ch2: 0
go 0: after 1st select
go 0: receive from ch1: 0
go 2: push 2
go 1: push 2
go 0: receive from ch1: 1
go 0: receive from ch2: 1
go 1: push 3
go 2: push 3
go 0: receive from ch1: 2
go 0: receive from ch2: 2
go 1: push 4
go 2: push 4
go 0: receive from ch2: 3
go 0: receive from ch1: 3
go 2: push 5
go 1: push 5
go 0: receive from ch2: 4
go 0: receive from ch1: 4
go 2: push 6
go 1: push 6
go 0: receive from ch2: 5
go 0: receive from ch1: 5
main: now close the exit channel
go 0: receive from ch2: 6
```
