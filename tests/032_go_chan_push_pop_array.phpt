--TEST--
Go Chan push pop array

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
	echo "push\n";
	$v = [ "abc"=>"d", 3, 4, "subarr"=>[1,2,"c"=>"d"] ];
	$ch->push($v);
	echo "pushed:";
	var_dump($v);
},[$ch]);

go(function($ch){
	echo "pop\n";
	$v = $ch->pop();
	echo "popped:";
	var_dump($v);
},[$ch]);

Scheduler::join();

?>
--EXPECT--
SUB-TC: #1
push
pushed:array(4) {
  ["abc"]=>
  string(1) "d"
  [0]=>
  int(3)
  [1]=>
  int(4)
  ["subarr"]=>
  array(3) {
    [0]=>
    int(1)
    [1]=>
    int(2)
    ["c"]=>
    string(1) "d"
  }
}
pop
popped:array(4) {
  ["abc"]=>
  string(1) "d"
  [0]=>
  int(3)
  [1]=>
  int(4)
  ["subarr"]=>
  array(3) {
    [0]=>
    int(1)
    [1]=>
    int(2)
    ["c"]=>
    string(1) "d"
  }
}

