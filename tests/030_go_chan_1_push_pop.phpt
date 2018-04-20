--TEST--
Go Chan(1) push pop

--FILE--
<?php
use \go\Chan;
use \go\Scheduler;

function subtc($seq){
    echo "SUB-TC: #$seq\n";
}

subtc(1);
$ch = new Chan(1);

go(function($ch){
	echo "pop\n";
	$v = $ch->pop();
	echo "popped:";
	var_dump($v);
},$ch);

go(function($ch){
	echo "push\n";
	$v = "abc";
	$ch->push($v);
	echo "pushed $v\n";
},$ch);

Scheduler::join();

?>
--EXPECT--
SUB-TC: #1
pop
push
pushed abc
popped:string(3) "abc"

