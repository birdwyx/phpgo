# goo
  
\(PHP 5 >= 5.4.0, PHP 7, phpgo >= 0.9.0\)

goo — 创建协程（go routine）, 可以设置协程参数

## 说明
#### bool goo (array $options, callable $callback \[, array $parameters\] )

## 参数
#### options

协程参数数组： 以key value对形式的协程参数组成的数组。

可以设置以下协程参数:

#####stack_size  创建协程的最大栈大小(bytes)，默认1024M， stack_size不能小于32M；如果指定了小于32M的值，phpgo会自动设置为32M。
事实上协程栈是自动增长的，使用比较大的值也不会造成内存浪费。这个参数一般不用设置。

#####isolate_http_globals 是否将子协程和父协程的超全局变量分离，当isolate_http_globals为true时，则将子协程使用自己独立的超全局变量。

当子协程修改了超全局变量的值时不会影响父协程和其他协程里超全局变量的值。 

这里提到的超全局变量指：

$_GET, $_POST, $_COOKIE, $_SERVER, $_ENV, $_REQUEST, $_FILES

注意这里不包含$GLOBALS超全局变量。在任何地方修改$GLOBALS都会影响到协程外(调度器)或其他协程看到的值。

> isolate_http_globals 有什么用呢？
>
> 最大的用处是在你用多个协程来处理http的请求的时候，还可以调用一些会使用/修改超全局变量的现有方法, 这在你移植一些老代码到phpgo上的时候会有用。如果没有isolate_http_globals，在协程里你是不能随便修改这些超全局变量的，因为这会污染到协程外或其他协程内的超全局变量，
而你的老代码是肯定不会料到后面还会出现协程这回事，当然不会考虑这种影响。通过isolate_http_globals=true 你就可以放心了。

#### callback

mixed callback (\[mixed $parameter1, \[ mixed $parameter2, ...\] \])

回调函数，也就是go routine的主函数，go routine的主函数在协程创建后立即执行。

#### parameters

传递给go routine主函数（callback）的参数数组。

$parameters数组中的每个参数与$callback参数表里的参数依次匹配。

如果在callback参数表里定义了引用传参方式，则$parameters数组里对应参数必须为引用，否则在callback里对该引用的修改对外部参数不起作用。

## 返回值
成功返回TURE，失败返回FALSE。

## 示例
```
<?php
use go\Chan;
use go\Scheduler;

$_GET['var'] = "hello";

$options = [
	'stack_size' => 64 * 1024, 
	'isolate_http_globals' => true
];

$ch = new Chan(1);
goo($options, function() use ($get, $ch) {
    if($get == $GET) echo 'go routine correctly inherits $_GET' . PHP_EOL;
    $_GET['var'] = 'world';
    $ch->push(1);
}
$ch->pop(); // wait the go routine to finish

if($_GET['var'] == 'hello') echo 'change of $_GET in go routine does not impact the ouside' . PHP_EOL;

```
输出
```
go routine correctly inherits $_GET
change of $_GET in go routine does not impact the ouside
```
