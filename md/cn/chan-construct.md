# go\Chan::__construct
  
\(PHP 5 >= 5.4.0, PHP 7, phpgo >= 0.9.0\)

__construct — 创建管道

## 说明
#### __construct( [array $create_opptions | int $capacity] )  

## 参数
#### create_opptions

当__construct第一个参数为数组时，代表传入的是创建参数。例如：
```
$create_opptions =  array(
  'name' => 'chanName',   //管道名称
  'copy' => true,         //是否复制
  'capacity' => 10,       //最大容量
);

$ch = new Chan($create_opptions);
```
管道名称：可以命名一个管道，通过相同的管道名称可以获取同一个管道对象。非首次调用时copy/capacity不起作用   
是否复制：为true时压入管道的数据将被复制到共享内存块中，一般在需要线程之间传递数据时才设为true  
最大容量：管道中最多容纳的数据项个数  

#### capacity

当__construct第一个参数为整数时，代表最大容量。例如以下语句创建容量为10的管道：
````
$ch = new Chan(10);
````

## 返回值
返回创建的管道对象
