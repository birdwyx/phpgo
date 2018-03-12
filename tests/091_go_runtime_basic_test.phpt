--TEST--
Go runtime basic test

--FILE--
<?php
use \go\Runtime;
use \go\Scheduler;

function subtc($seq){
    echo "SUB-TC: #$seq\n";
}

subtc(1);

go(function(){
	echo "go 1: ";
	echo "NumGoroutine: " . Runtime::NumGoroutine() .PHP_EOL;
});

go(function(){
	usleep(10*1000);
	echo "go 2: ";
	echo "NumGoroutine: " . Runtime::NumGoroutine() .PHP_EOL;
});


go(function(){
	Runtime::Gosched();
	echo "go 3\n";
});

go(function(){
	echo "go 4\n";
});

Scheduler::RunOnce();
Scheduler::RunOnce();
Scheduler::RunOnce();

echo "NumGoroutine:" . Runtime::NumGoroutine() .PHP_EOL;

Scheduler::RunJoinAll();
echo "NumGoroutine:" . Runtime::NumGoroutine() .PHP_EOL;


?>
--EXPECT--
SUB-TC: #1
go 1: NumGoroutine: 4
go 4
go 3
NumGoroutine:1
go 2: NumGoroutine: 1
NumGoroutine:0

