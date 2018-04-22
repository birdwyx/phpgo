--TEST--
Go runtime basic test

--FILE--
<?php
use \Go\Runtime;
use \Go\Scheduler;

function subtc($seq){
    echo "SUB-TC: #$seq\n";
}

subtc(1);

go(function(){
	echo "go 1: ";
	echo "numGoroutine: " . Runtime::numGoroutine() .PHP_EOL;
});

go(function(){
	usleep(10*1000);
	echo "go 2: ";
	echo "numGoroutine: " . Runtime::numGoroutine() .PHP_EOL;
});


go(function(){
	Runtime::gosched();
	echo "go 3\n";
});

go(function(){
	echo "go 4\n";
});

Scheduler::run();
Scheduler::run();
Scheduler::run();

echo "numGoroutine:" . Runtime::numGoroutine() .PHP_EOL;

Scheduler::join();
echo "numGoroutine:" . Runtime::numGoroutine() .PHP_EOL;


?>
--EXPECT--
SUB-TC: #1
go 1: numGoroutine: 4
go 4
go 3
numGoroutine:1
go 2: numGoroutine: 1
numGoroutine:0

