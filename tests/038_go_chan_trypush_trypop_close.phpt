--TEST--
Go Chan tryPush tryPop close

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

subtc(1, "tryPush tryPop test");
$ch = new Chan(1);
my_assert(1, $ch->tryPush("push#0") );

go(function($ch){
	$push = 0;
	my_assert(1, !$ch->tryPush("push#1"));
	while( !$ch->tryPush("push#1") ){
		$push++;
		Time::sleep(100 * Time::MILLISECOND);
	}
	my_assert(1, $push == 10);
	my_assert(1, $ch->tryPop() == "push#1");
},[$ch]);

go(function($ch){
	Time::sleep(1 * Time::SECOND);
	my_assert(1, $ch->pop() == "push#0" );
},[$ch]);

Scheduler::join();
passtc(1, true);

//=======>
subtc(2, "chan close test - ensure data can be read from close channel if available");
$ch = new Chan(1);

go(function($ch){
	$v = $ch->pop();
	my_assert(2, $v==="push#0");
},[$ch]);

my_assert(2, $ch->push("push#0"));
$ch->close();

Scheduler::join();
passtc(2, true);

//===>

subtc(3, "chan close test - ensure warning generated while tryPushing to closed channel and false returned");
$ch1 = new Chan(0);

go(function($ch){
	@$res = $ch->tryPush("push#1");
	my_assert(3, $res===false);
},[$ch1]);
$ch1->close();

Scheduler::join();
passtc(3, true);


//=======>
subtc(4, "chan close test - ensure data can be tryPop from close channel if available");
$ch = new Chan(1);

go(function($ch){
	$v = $ch->tryPop();
	my_assert(4, $v==="push#0");
},[$ch]);

$ch->push("push#0");
$ch->close();

Scheduler::join();
passtc(4, true);

//=======>
subtc(5, "chan close test - ensure NULL retuned for tryPop if channel closed and no data ready");
$ch = new Chan(1);

go(function($ch){
	$v = $ch->tryPop();
	my_assert(5, $v===NULL);
},[$ch]);

$ch->close();

Scheduler::join();
passtc(5, true);

?>
--EXPECT--
SUB-TC #1: tryPush tryPop test
SUB-TC #1: PASS
SUB-TC #2: chan close test - ensure data can be read from close channel if available
SUB-TC #2: PASS
SUB-TC #3: chan close test - ensure warning generated while tryPushing to closed channel and false returned
SUB-TC #3: PASS
SUB-TC #4: chan close test - ensure data can be tryPop from close channel if available
SUB-TC #4: PASS
SUB-TC #5: chan close test - ensure NULL retuned for tryPop if channel closed and no data ready
SUB-TC #5: PASS

