--TEST--
Go Goid

--SKIPIF--


--FILE--
<?php
use \go\Scheduler;
use \go\Runtime;

function subtc($seq){
    echo "SUB-TC: #$seq\n";
}

subtc(1);

go(function(){
	assert(Runtime::Goid() == 1);
	//var_dump(Runtime::Goid());
});

go(function(){
	assert(Runtime::Goid() == 2);
	//var_dump(Runtime::Goid());
});

assert(Runtime::Goid() == 0);
//var_dump(Runtime::Goid());

Scheduler::join();

echo "success\n";

?>
--EXPECT--
SUB-TC: #1
success


