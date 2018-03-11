--TEST--
Go Mutex basic test

--FILE--
<?php
use \go\Mutex;
use \go\Scheduler;

function subtc($seq){
    echo "SUB-TC: #$seq\n";
}

subtc(1);

echo "create mutex, signaled\n";
$m = new Mutex();
assert(!empty($m));

go(function() use($m){
	echo "go 1 try to obtain lock\n";
	$m->lock();
	echo "go 1 obtained lock\n";
});

go(function() use($m){
	echo "go 2 try to obtain lock\n";
	$m->lock();
	echo "go 2 obtained lock\n";
});

go(function() use($m){
	usleep(1000);
	echo "go 3 release lock\n";
	$m->unlock();
});


Scheduler::RunJoinAll();

subtc(2);

echo "create mutex, not signaled\n";
$m1 = new Mutex(false);
assert(!empty($m1));

go(function() use($m1){
	echo "go 3 try to obtain lock\n";
	$m1->lock();
	echo "go 3 obtained lock\n";
	echo "go 3 release lock\n";
	$m1->unlock();
});

go(function() use($m1){
	echo "go 4 try to obtain lock\n";
	$m1->lock();
	echo "go 4 obtained lock\n";
	echo "go 4 release lock\n";
	$m1->unlock();
});

go(function() use($m1){
	usleep(1000);
	echo "go 5 release lock\n";
	$m1->unlock();
});

Scheduler::RunJoinAll();

?>
--EXPECT--
SUB-TC: #1
create mutex, signaled
go 1 try to obtain lock
go 1 obtained lock
go 2 try to obtain lock
go 3 release lock
go 2 obtained lock
SUB-TC: #2
create mutex, not signaled
go 3 try to obtain lock
go 4 try to obtain lock
go 5 release lock
go 3 obtained lock
go 3 release lock
go 4 obtained lock
go 4 release lock


