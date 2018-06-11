# select
  
\(PHP 5 >= 5.4.0, PHP 7, phpgo >= 0.9.0\)

select — 进行一次事件轮询，并返回一个Selector对象

和golang的select语法结构类似， phpgo select轮询指定的分支，随机选择一个可读写（即，channel可读或可写）的分支，执行对应读写操作后，执行该分支对应的回调函数，并传入所读/写的数据。

如果没有可读写分支，select执行'default'分支对应的回调函数。如果没有'default'分支，函数正常返回。

## 说明
#### Selector select ( array $case1, [ array $case2, ...] )

## 参数
#### case
一个case代表select事件轮询的一个分支，每个case是一个数组，组成如下：
array( string $switch_type, \[ Chan $channel, string $operator, mixed $operand, \] callable $callback )

其中 switch_type 可以以下两者之一：
- 'case'，普通分支，类似golang中的case关键字所代表的的分支
- 'default'，默认分支，类似golang中的default关键字所代表的的分支


## 返回值
select 分析传入的分支参数，生成内部的Selector对象，将分支信息存入该对象，然后返回该Selector对象。
>使用Selector对象可以优化性能，因为不用每次都解析select的参数了，可以节省CPU资源。

## 参见
- [go\Selector](https://github.com/birdwyx/phpgo/blob/master/md/cn/selector.md) — Selector类

## 示例

