# select
  
\(PHP 5 >= 5.4.0, PHP 7, phpgo >= 0.9.0\)

select — 进行一次事件轮询，并返回一个Selector对象

和golang的select语法结构类似， phpgo select轮询所有指定的分支，随机选择一个可读写（即，channel可读或可写）的分支，执行对应读写操作后，再执行该分支对应的回调函数，并传入所读到/写入的数据。

如果没有可读写分支，select执行'default'分支对应的回调函数。如果没有'default'分支，则什么都不做正常返回。

## 说明
#### Selector select ( array $case1, [ array $case2, ...] )

## 参数
#### case
一个case代表select事件轮询的一个分支，每个case是一个数组，组成如下：
##### array( string $switch_type, \[ Chan $channel, string $operator, mixed $value, \] Callable $callback )

其中 switch_type 可以以下两者之一：

- 'case'，普通分支，类似golang中的case关键字所代表的的分支。

- 'default'，默认分支，类似golang中的default关键字所代表的的分支。

switch_type大小写不敏感。

#### 'case'分支
语法如下：
##### array('case',  Chan $channel, \['->', \[mixed &$value\]\] |  \['<-', mixed $value\], Callable $callback)
其中：  
'->'操作符表示从通道$channel中读取数据到$value中，然后调用$callback($value), $value省略时，则从$channel读取数据到临时变量，并传入$callback中。$value不省略时，必须是引用参数。  
'<-'操作符表示将$value写入到$channel中，写入后调用$callback($value)。此时$value不能省略。

#### 'default'分支
语法如下：
##### array('default', Callable $callback)
select轮询所有分支，随机选择一个可读写的'case'分支来执行，如果所有'case'分支都不可读或写，则select执行'default'分支的回调函数$callback，不传入参数。

## 返回值
select 分析传入的分支参数，生成内部的Selector对象，将分支信息存入该对象，然后返回该Selector对象。
>使用Selector对象可以优化性能，因为不用每次都解析select的参数了，可以节省CPU资源。

## 参见
- [go\Selector](https://github.com/birdwyx/phpgo/blob/master/md/cn/selector.md) — Selector类

## 示例

### 1. Select读取数据
```
<?php
use \Go\Chan;
use \Go\Scheduler;

$ch = new Chan(["capacity"=>1]);

go(function() use($ch){
    $read = false; $df = false; $v=0;
    $ch->push(1);
    select(
        [
            'case', $ch, "->", &$v, function($value) use(&$read){
                //$value (==$v) should be 1 as read from $ch
                if($value===1)
                    $read = true;
            }
        ],
        [
            'default', function() use(&$df){
                $df = true;
             }
		]
    );
    
    assert(
        $read===true &&  //should hit the channel-read branch
        $df===false &&   //should not hit the default branch
        $v === 1         //$v should be 1 as the popped data from channel is assigned to its reference
    );
    echo "success\n";
});

Scheduler::join();
```
输出success，无断言失败：
```
success
```
