--TEST--
Go Mutex recursively lock

--FILE--
<?php
use \go\Mutex;
use \go\Scheduler;

function subtc($seq){
    echo "SUB-TC: #$seq\n";
}

subtc(1);

echo "create mutex, signaled\n";
$m = new Mutex(true);
assert(!empty($m));

go(function() use($m){
	echo "go 1 try to obtain lock\n";
	$m->lock();
	echo "go 1 obtained lock\n";
	echo "..go 1 try to obtain lock: 2nd time\n";
	$m->lock();
	echo "..go 1 obtained lock: 2nd time\n";
	echo "....go 1 try to obtain lock: 3rd time\n";
	$m->lock();
	echo "....go 1 obtained lock: 3rd time\n";
	
	
	echo "....go 1 release lock\n";
	$m->unlock();
	echo "..go 1 release lock: 2nd time\n";
	$m->unlock();
	echo "go 1 release lock: 3rd time\n";
	$m->unlock();
});

go(function() use($m){
	echo "go 2 try to obtain lock\n";
	$m->lock();
	echo "go 2 obtained lock\n";
});

Scheduler::RunJoinAll();

?>
--EXPECT--
SUB-TC: #1
create mutex, signaled
go 1 try to obtain lock
go 1 obtained lock
..go 1 try to obtain lock: 2nd time
..go 1 obtained lock: 2nd time
....go 1 try to obtain lock: 3rd time
....go 1 obtained lock: 3rd time
....go 1 release lock
..go 1 release lock: 2nd time
go 1 release lock: 3rd time
go 2 try to obtain lock
go 2 obtained lock

