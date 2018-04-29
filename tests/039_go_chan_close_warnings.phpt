--TEST--
Go chan close warnings test

--FILE--
<?php
use \Go\Chan;
use \Go\Scheduler;
use \Go\Time;

$tcfailures = array();
function my_assert($seq, $cond){
	global $tcfailures;
	if(!$cond){
		echo "SUB-TC #$seq: assertion failed @ line " . (debug_backtrace()[0]['line']) .PHP_EOL;
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

//======>
subtc(1, "chan close test - ensure warning generated while pushing to closed channel and false returned");
$ch1 = new Chan(0);

go(function($ch){
	$res = $ch->push("push#1");
	my_assert(1, $res===false);
},$ch1);
$ch1->close();

Scheduler::join();
passtc(1, true);

subtc(2, "chan close test - ensure warning generated while tryPushing to closed channel and false returned");
$ch1 = new Chan(0);

go(function($ch){
	$res = $ch->tryPush("push#1");
	my_assert(2, $res===false);
},$ch1);
$ch1->close();

Scheduler::join();
passtc(2, true);

?>
--EXPECTREGEX--
.*SUB-TC #1.*allready closed.*SUB-TC #1: PASS.*SUB-TC #2.*allready closed.*SUB-TC #2: PASS.*
