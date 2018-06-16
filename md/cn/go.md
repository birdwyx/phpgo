# go
  
\(PHP 5 >= 5.4.0, PHP 7, phpgo >= 0.9.0\)

go — 创建协程（go routine）

## 说明
#### bool go ( callable $callback \[, array $parameters\] )

## 参数
#### callback

mixed callback (\[mixed $parameter1, \[ mixed $parameter2, ...\] \])

回调函数，也就是go routine的主函数，go routine的主函数在协程创建后立即执行。

>go 的回调函数参数可以是：
>- 匿名函数，如：
>```
>go(function(){
>    echo "Hello World!\n";
>});
>```
>- 普通函数，如:
>```
>go('hello', ['World']); //相当于在协程里执行hello('World');
>```
>- 类的静态方法，如：
>```
>go('ClassA::hello', ['phpgo']);  //相当于在协程里执行 ClassA::Hello('phpgo');
>go(["ClassA", "hello"], ['phpgo']);   //相当于在协程里执行 ClassA::Hello('phpgo');
>```
>- 类的非静态方法，如：
>```
>go([$obj, 'hello'], ['phpgo']); //相当于在协程里执行 $obj->hello('phpgo');
>```

#### parameters

传递给go routine主函数（callback）的参数数组。

$parameters数组中的每个参数与$callback参数表里的参数依次匹配。

如果在callback参数表里定义了引用传参方式，则$parameters数组里对应参数必须为引用，否则在callback里对该引用的修改对外部参数不起作用。

## 返回值
成功返回TURE，失败返回FALSE。

## 参见
- [goo](https://github.com/birdwyx/phpgo/blob/master/md/cn/goo.md) — 创建go routine，支持可选参数

## 示例
### 1. Hello World
```
<?php
use go\Scheduler;

go(function(){
  echo "Hello World!";
});

Scheduler::join();
```
输出：
```
Hello world!
```

### 2. 传递参数
```
<?php
use go\Scheduler;

$name = "phpgo";
go(function(){
    echo "Hello ${name}!\n";
}, [$name]);

Scheduler::join();
```
输出：
```
Hello phpgo!
```

### 3. 引用传参
```
<?php
use go\Scheduler;

$message = '';
go(function(&$msg){
    echo "Alice sends a greeting\n";
    $msg = "Greeting from Alice\n";
}, [&$message]);

Scheduler::join();
echo $message;
```
输出：
```
Alice sends a greeting
Greeting from Alice
```
上面示例展示了使用引用变量来与go routine通信的方法，但这种方法并不推荐，很明显这增加了go routine及其调用者之间的耦合，也会违背go语言“Do not communicate by sharing memory; instead, share memory by communicating.”的原则。

更好的方式是使用channel来通信：

### 4. 使用channel和go routine通信
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
上面这个示例展示了使用channel来与go routine通信的方法，也展示了可以使用use关键字结合匿名函数来向go routine传参，一般来说仅使用use来传递只读参数，如果在use里传入引用并在go routine里对相应参数进行更改，则与上面示例3一样存在耦合问题。

关于channel的使用，参见 [go\Chan](https://github.com/birdwyx/phpgo/md/cn/chan.md)

### 5. 使用命名函数
```
<?php
use go\Scheduler;

function hello($name){
    echo "Hello ${name}!\n";
}

go('hello', ['phpgo']);

Scheduler::join();
```
输出：
```
Hello phpgo!
```


