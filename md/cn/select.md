# select
  
\(PHP 5 >= 5.4.0, PHP 7, phpgo >= 0.9.0\)

select — 进行一次事件轮询，并返回一个Selector对象

和golang的select语法结构类似， phpgo select轮询所有指定的分支，随机选择一个可读写（即，channel可读或可写）的分支，执行对应读写操作后，再执行该分支对应的回调函数，并传入所读到/写入的数据。

如果没有可读写分支，select执行'default'分支对应的回调函数。如果连'default'分支都没有，则select阻塞一直到有读写操作可进行。

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
'<-','->'和$value都省略时，默认为读操作，即从$channel读取数据到临时变量，并传入$callback中。

>也就是说，总共有以下几种合法情形：
>```
>select( ['case', $ch, '->', &$value, function($v){...} ], [...] );  //从$ch读取数据到$value中，并传给function
>select( ['case', $ch, '->', function($v){...} ], [...] );           //从$ch读取数据到临时变量，并传给function
>select( ['case', $ch, function($v){...} ], [...] );                 //从$ch读取数据到临时变量，并传给function
>
>select( ['case', $ch, '<-', $value, function($v){...} ], [...] ); //将$value写到$ch中，然后将$value传给function
>```

#### 'default'分支
语法如下：
##### array('default', Callable $callback)
select轮询所有分支，随机选择一个可读写的'case'分支来执行，如果所有'case'分支都不可读或写，则select执行'default'分支的回调函数$callback，不传入参数。  
>例如，以下案例在各个case分支没有读写操作可进行的时候，等待一秒钟：
>```
>select(
>    ['case', ...], 
>    ['case', ...],
>    ['default', function(){ sleep(1); }]
>);
>```

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


### 2. 生产者与消费者
```
<?php
use \Go\Chan;
use \Go\Scheduler;
use \Go\Time;

function write_data($ch, $data){
    $data_written = new Chan(0);
    
    // try to write data to the channel, if the channel is not available to write, wait a 50ms
    // loop until the data is successfully written
    select(
        [
            'case', $ch, "<-", $data, function($value) use($data_written){
                echo "data written:";
                var_dump($value);
                $data_written->close();
            }
        ],
        [
            'default', function(){
                echo "channel full, wait 50ms for consumer to consume\n";
                Time::sleep(50*1000*1000);
            }
        ]
    )->loop($data_written);
}

function producer($ch, $close){
    select(
        [
            'default', function() use($ch){
                $data = rand();
                write_data($ch, $data);
            }
        ]
    )->loop($close);
}

function consumer($ch, $close){
    select(
        [
            'case', $ch, function($value) {
                echo "data consumed:";
                var_dump($value);
                
                // wait a 10ms: simulating a slow consumer:
                // just to see the output from producer:
                // "channel full, wait 50ms for consumer to consume"
                Time::sleep(10*1000*1000);
            }
        ]
    )->loop($close);
}

// main routine
go( function(){
    $ch = new Chan(["capacity"=>20]); 
    $close = new Chan(0);
    go('producer', [$ch, $close]);
    go('consumer', [$ch, $close]);
    
    // tell producer and consumer to exit after 10 seconds
    // note: Time::sleep follows go convension: time unit in nano-second 
    // please note the difference with php sleep() witch has time unit in seconds
    Time::sleep(10* 1000*1000*1000); 
    $close->close();
});

Scheduler::join();
```
输出:
```
...
data written:int(1848193587)
data consumed:int(1848193587)
data written:int(1572420815)
data written:int(1142275719)
data written:int(761289258)
data written:int(251548635)
data written:int(695961973)
data written:int(1035384689)
data written:int(1090100059)
data written:int(220684829)
data written:int(1971664342)
data written:int(1672374281)
data written:int(1560995315)
data written:int(1982880169)
data written:int(1726045443)
data written:int(390291502)
data written:int(1979133962)
data written:int(162498409)
data written:int(1624140423)
data written:int(856873465)
data written:int(1547478199)
data written:int(881099376)
channel full, wait 50ms for consumer to consume
data consumed:int(1572420815)
data consumed:int(1142275719)
data consumed:int(761289258)
data consumed:int(251548635)
data consumed:int(695961973)
data written:int(215756702)
data written:int(1076793154)
data written:int(53161470)
data written:int(591741440)
data written:int(2144247704)
channel full, wait 50ms for consumer to consume
data consumed:int(1035384689)
data consumed:int(1090100059)
data consumed:int(220684829)
data consumed:int(1971664342)
data consumed:int(1672374281)
data written:int(321058344)
...
```
上例是经典的生产者消费者场景：生产者生产并写入数据，消费者读取并处理数据，以channel为缓冲区。
- 主过程生成一个20缓冲项的channel，传给producer和consumer；
- producer()生成随机数，并持续调用write_data将数据写入缓冲区$ch，直到收到退出信号；  
>如果缓冲区$ch满了，write_data会等50ms重试，一直到缓冲区有空了，成功写入数据才返回。  
- consumer()从$ch读取并打印出读取的值，直到收到退出信号。

退出信号即$close被关闭：select()返回一个Selector对象，通过调用Selector::loop($close)循环执行select中的case直到$close中有值或者$close被关闭，更多信息参见[Selector::Loop()](https://github.com/birdwyx/phpgo/blob/master/md/cn/selector-loop.md)）。  
