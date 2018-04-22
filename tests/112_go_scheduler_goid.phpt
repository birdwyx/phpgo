--TEST--
Go goid

--SKIPIF--


--FILE--
<?php
use \Go\Scheduler;
use \Go\Runtime;

function subtc($seq){
    echo "SUB-TC: #$seq\n";
}

subtc(1);

go(function(){
	assert(Runtime::goid() == 1);
	//var_dump(Runtime::goid());
});

go(function(){
	assert(Runtime::goid() == 2);
	//var_dump(Runtime::goid());
});

assert(Runtime::goid() == 0);
//var_dump(Runtime::goid());

Scheduler::join();

echo "success\n";

?>
--EXPECT--
SUB-TC: #1
success


