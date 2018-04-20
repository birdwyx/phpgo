--TEST--
Go Select read write default

--FILE--
<?php
use \go\Chan;
use \go\Scheduler;

function subtc($seq){
    echo "SUB-TC: #$seq\n";
}

$ch = new Chan(["capacity"=>10]);


go(function() use($ch){
	$seq = 1;
	
	subtc(1);
	$read = 0; $write = 0; $df = 0; $v=1111;
	$ch->push(1);
	select(
		_case($ch, "->", $v, function($v) use(&$read){
			if($v===1)
				$read = 1;
		}),
		_default(function() use(&$df){
			$df = 1;
		})
	);
	assert($read===1 && $df===0);
	echo "success\n";
	
	subtc(2);
	$read = 0; $write = 0; $df = 0; $v=1111;
	
	select(
		_case($ch, "<-", $v, function($v) use(&$write){
			if($v==1111)
				$write = 1;
		}),
		_default(function() use(&$df){
			$df = 1;
		})
	);
	$ch->Pop();
	assert($write===1 && $df===0);
	echo "success\n";
	
	subtc(3);
	$read = 0; $write = 0; $df = 0; $v=1111;
	for($i=0; $i<10;$i++){
		$ch->push($i);
	}
	
	select(
		_case($ch, "<-", $v, function($v) use(&$write){
			//if($v==1111)
				$write = 1;
		}),
		_default(function() use(&$df){
			$df = 1;
		})
	);
	assert($write===0 && $df===1);
	echo "success\n";
	
	subtc(4);
	$read = 0; $write = 0; $df = 0; $v=1111;
	for($i=0; $i<10;$i++){
		$ch->pop();
	}
	select(
		_case($ch, "->", $v, function($v) use(&$read){
			//if($v==1111)
				$read = 1;
		}),
		_default(function() use(&$df){
			$df = 1;
		})
	);
	assert($read===0 && $df===1);
	echo "success\n";

});
Scheduler::join();

?>
--EXPECT--
SUB-TC: #1
success
SUB-TC: #2
success
SUB-TC: #3
success
SUB-TC: #4
success

