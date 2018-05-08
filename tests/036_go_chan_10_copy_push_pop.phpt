--TEST--
Go Chan(10,copy) push pop

--FILE--
<?php
use \Go\Chan;
use \Go\Scheduler;

function subtc($seq){
    echo "SUB-TC: #$seq\n";
}

subtc(1);
$ch = new Chan(["capacity"=>10, "copy"=>true]);

go(function($ch){
	for($i = 1; $i<=20; $i++){
		echo "push $i:\n";
		$ch->push($i);
	}
},[$ch]);

go(function($ch){
	$i = 1;
	while($i <= 20){
		echo "pop for the $i time:\n";
		var_dump($ch->pop());
		$i++;
	}
},[$ch]);

Scheduler::join();

?>
--EXPECT--
SUB-TC: #1
push 1:
push 2:
push 3:
push 4:
push 5:
push 6:
push 7:
push 8:
push 9:
push 10:
push 11:
pop for the 1 time:
int(1)
pop for the 2 time:
int(2)
pop for the 3 time:
int(3)
pop for the 4 time:
int(4)
pop for the 5 time:
int(5)
pop for the 6 time:
int(6)
pop for the 7 time:
int(7)
pop for the 8 time:
int(8)
pop for the 9 time:
int(9)
pop for the 10 time:
int(10)
pop for the 11 time:
push 12:
push 13:
push 14:
push 15:
push 16:
push 17:
push 18:
push 19:
push 20:
int(11)
pop for the 12 time:
int(12)
pop for the 13 time:
int(13)
pop for the 14 time:
int(14)
pop for the 15 time:
int(15)
pop for the 16 time:
int(16)
pop for the 17 time:
int(17)
pop for the 18 time:
int(18)
pop for the 19 time:
int(19)
pop for the 20 time:
int(20)



