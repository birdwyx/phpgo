--TEST--
Go select read

--FILE--
<?php
use \Go\Chan;
use \Go\Scheduler;

$tcfailures = array();
function my_assert($seq, $cond){
	global $tcfailures;
	if(!$cond){
		echo "SUB-TC #$seq: assertion failed @ line " . ( __LINE__ + 4) .PHP_EOL;
		$tcfailures[$seq] = true;
	}
}

function subtc($seq, $title){
    echo "SUB-TC #$seq: $title\n";
}

function passtc($seq, $cond){
	global $tcfailures;
	echo ( empty($tcfailures[$seq]) && $cond) ? 
	     "SUB-TC #$seq: PASS\n" : 
		 "SUB-TC #$seq: FAIL\n";
}

subtc(1, "Verify reading int from channel into a reference");
$ch = new Chan(["capacity"=>10]);

go(function($ch){
	for($i = 1; $i<=20; $i++){
		//echo "push $i:\n";
		$ch->push($i);
	}
},[$ch]);


go(function() use($ch){
	$v = -1; $u = $v;  $i=1; 
	$cases = ['case', 'CasE', 'CAse', 'CASe', 'CASE'];
	while(true){
		select(
			[
				$cases[$i%5], $ch, "->", &$v, function($v) use($i){
					my_assert(1, $v==$i);
				}
			]
		);
		if($i>20) break;
		$i++;
		usleep(100);
	}
	passtc(1, $v == 20 && $u = -1);
});

Scheduler::join();
//passtc(1, $v == 20 && $u = -1);

subtc(2, "Verify reading string from channel into a reference");
$ch = new Chan(["capacity"=>10]);

go(function($ch){
	for($i = 1; $i<=20; $i++){
		//echo "push $i:\n";
		$ch->push("$i");
	}
},[$ch]);


go(function() use($ch){
	$v = -1; $u = $v;  $i=1; 
	$cases = ['case', 'CasE', 'CAse', 'CASe', 'CASE'];
	while(true){
		select(
			[
				$cases[$i%5], $ch, "->", &$v, function($v) use($i){
					my_assert(2, $v==$i);
				}
			]
		);
		if($i>20) break;
		$i++;
		usleep(100);
	}
	passtc(2, $v == 20 && $u = -1);
});

Scheduler::join();


subtc(3, "Verify reading array from channel into a reference");
$ch = new Chan(["capacity"=>10]);

go(function($ch){
	for($i = 1; $i<=20; $i++){
		//echo "push $i:\n";
		$ch->push( array("value"=>$i) );
	}
},[$ch]);

go(function() use($ch){
	$v = -1; $u = $v;  $i=1; 
	$cases = ['case', 'CasE', 'CAse', 'CASe', 'CASE'];
	while(true){
		select(
			[
				$cases[$i%5], $ch, "->", &$v, function($v) use($i){
					my_assert(3, $v== array("value"=>$i) );
				}
			]
		);
		if($i>20) break;
		$i++;
		usleep(100);
	}
	passtc(3, $v == array("value"=>20) && $u = -1);
});

Scheduler::join();

?>
--EXPECT--
SUB-TC #1: Verify reading int from channel into a reference
SUB-TC #1: PASS
SUB-TC #2: Verify reading string from channel into a reference
SUB-TC #2: PASS
SUB-TC #3: Verify reading array from channel into a reference
SUB-TC #3: PASS

