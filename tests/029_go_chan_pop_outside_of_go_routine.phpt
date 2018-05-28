--TEST--
Go Chan pop outside of go routine

--FILE--
<?php
use \Go\Chan;
use \Go\Scheduler;

function subtc($seq){
    echo "SUB-TC: #$seq\n";
}

subtc(1);
$ch = new Chan(1);

go(function($ch){
	$ch->push(100);
},[$ch]);

var_dump($ch->pop());

subtc(2);
$ch = new Chan(0);

go(function($ch){
	sleep(0.1);
	$ch->push("abc");
},[$ch]);

var_dump($ch->pop());


?>
--EXPECT--
SUB-TC: #1
int(100)
SUB-TC: #2
string(3) "abc"
