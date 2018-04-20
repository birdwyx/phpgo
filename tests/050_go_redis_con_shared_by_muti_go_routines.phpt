--TEST--
Go redis connection shared by mutilple go routines (deemed to be FAIL)

--SKIPIF-- 
<?php   
	if( !class_exists ("Redis") ){
		echo "skip\n";
	}else{
		$r = new Redis();
		$r->connect("127.0.0.1", "6379");
		$r->set("___test__redis_availability__", 1);
		if( $r->get("___test__redis_availability__") != 1 )
			echo "skip\n";
		else{
			$r->del("___test__redis_availability__");
		}
	}
?>
--FILE--
<?php
use \go\Chan;
use \go\Scheduler;

function subtc($seq){
    echo "SUB-TC: #$seq\n";
}

subtc(1);
$ch = new Chan(["capacity"=>1000]);

$redis = new \Redis();
$redis->connect("127.0.0.1", "6379");

go(function() use($redis){
	for($i=0; $i<10000; $i++){
		$redis->set("foo", $i);
		//echo "foo: " . $redis->get("foo") . PHP_EOL;
		assert( ($r = $redis->get("foo")) == $i);
		
		if($r != $i) break;
	}
	if($i==10000)
		echo "get set of foo: success\n";
});

//$redis = new \Redis();
//$redis->connect("127.0.0.1", "6379");

go(function() use($redis){
	for($i=0; $i<10000; $i++){
		$redis->set("bar", $i);
		assert( ($r = $redis->get("bar")) == $i);
		
		if($r != $i) break;
	}
	if($i==10000)
		echo "get set of bar: success\n";
});

Scheduler::join();

?>
--EXPECT--
SUB-TC: #1
get set of foo: success
get set of bar: success
