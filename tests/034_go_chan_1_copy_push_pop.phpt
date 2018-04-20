--TEST--
Go Chan(1,copy) push pop

--FILE--
<?php
use \go\Chan;
use \go\Scheduler;

function subtc($seq){
    echo "SUB-TC: #$seq\n";
}

subtc(1);
$ch = new Chan(["capacity"=>1, "copy"=>true]);

go(function($ch){
	$v = "abc";
	
	echo "go1: try push $v\n";
	$ch->push($v);
	echo "go1: pushed $v\n";
	
	echo "go1: try push 100\n";
	$ch->push(100);
	echo "go1: pushed 100\n";
	
	echo "go1: try push true\n";
	$ch->push(true);
	echo "go1: pushed true\n";
	
	echo "go1: try push false\n";
	$ch->push(false);
	echo "go1: pushed false\n";
},$ch);

go(function($ch){
	echo "go2: try pop\n";
	$v = $ch->pop();
	echo "go2: popped:";	var_dump($v);
	
	echo "go2: try pop\n";
	$v = $ch->pop();
	echo "go2: popped:";	var_dump($v);
	
	echo "go2: try pop\n";
	$v = $ch->pop();
	echo "go2: popped:";	var_dump($v);
	
	echo "go2: try pop\n";
	$v = $ch->pop();
	echo "go2: popped:";	var_dump($v);
},$ch);

Scheduler::join();

?>
--EXPECT--
SUB-TC: #1
go1: try push abc
go1: pushed abc
go1: try push 100
go2: try pop
go2: popped:string(3) "abc"
go2: try pop
go1: pushed 100
go1: try push true
go1: pushed true
go1: try push false
go2: popped:int(100)
go2: try pop
go2: popped:bool(true)
go2: try pop
go1: pushed false
go2: popped:bool(false)
