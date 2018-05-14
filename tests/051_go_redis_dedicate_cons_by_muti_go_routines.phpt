--TEST--
Go redis dedicated connection by mutilple go routines

--SKIPIF-- 
<?php   
	if( !class_exists ("Redis") | !getenv('TEST_REDIS_SERVER') ){
		echo "skip\n";
	}else{
		$r = new Redis();
		
		$server = getenv('TEST_REDIS_SERVER');
		$arr = explode(':', $server);
		$r->connect($arr[0], $arr[1]);
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
use \Go\Chan;
use \Go\Scheduler;

function subtc($seq){
    echo "SUB-TC: #$seq\n";
}

$server = getenv('TEST_REDIS_SERVER');
if(!$server){
	echo "please set environment variable TEST_REDIS_SERVER (host:port) for this test to run\n";
	return;
}
$arr = explode(':', $server);
$host = !empty($arr[0])? $arr[0] : "127.0.0.1"; $port = !empty($arr[1]) ? $arr[1]:6379;

subtc(1);
$ch = new Chan(["capacity"=>1000]);

$redis = new \Redis();
$redis->connect($host, $port);

go(function() use($redis){
	for($i=0; $i<10000; $i++){
		$redis->set("foo", $i);
		
		$r = $redis->get("foo");
		//echo "foo: " . $redis->get("foo") . PHP_EOL;
		assert( $r == $i);
		
		if($r != $i) break;
	}
	if($i==10000)
		echo "get set of foo: success\n";
});

$redis = new \Redis();
$redis->connect($host, $port);

go(function() use($redis){
	for($i=0; $i<10000; $i++){
		$redis->set("bar", $i);
		$r = $redis->get("bar");
		assert( $r == $i);
		
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
