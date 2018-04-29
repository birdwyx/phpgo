--TEST--
Go Mutex basic test

--FILE--
<?php
use \Go\Mutex;
use \Go\Scheduler;
use \Go\Time;

$tcfailures = array();
function my_assert($seq, $cond){
	global $tcfailures;
	if(!$cond){
		echo "SUB-TC #$seq: assertion failed @ line " . (debug_backtrace()[0]['line']) .PHP_EOL;
		$tcfailures[$seq] = true;
	}
}

function subtc($seq, $title){
    echo "SUB-TC #$seq: $title\n";
}

function passtc($seq, $cond){
	global $tcfailures;
	echo ( empty($tcfailures[$seq]) && $cond) ? 
	     "SUB-TC #$seq: PASS\n" : 
		 "SUB-TC #$seq: FAIL\n";
}

subtc(1, "Mutex created signaled, lock unlock test");
$m = new Mutex();
my_assert(1, !empty($m));

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


Scheduler::join();

subtc(2, "Mutex created not signaled, lock unlock test" );
$m1 = new Mutex(false);
my_assert(2,!empty($m1));

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

Scheduler::join();


subtc(3, "Mutex tryLock/isLock test");
$m1 = new Mutex(false);
my_assert(3,!empty($m1));
my_assert(3, $m1->isLock());

go(function() use($m1){
	$i = 0;
	while( !$m1->tryLock() ){
		++$i;
		my_assert(3, $m1->isLock());
		Time::sleep(100*Time::MILLISECOND);
	}
	my_assert(3, $i == 10);
	my_assert(3, $m1->isLock());
	$m1->unlock();
	my_assert(3, !$m1->isLock());
});

go(function() use($m1){
	Time::sleep(1*Time::SECOND);
	$m1->unlock();
	my_assert(3, !$m1->isLock());
});

Scheduler::join();
passtc(3,true);

?>
--EXPECT--
SUB-TC #1: Mutex created signaled, lock unlock test
go 1 try to obtain lock
go 1 obtained lock
go 2 try to obtain lock
go 3 release lock
go 2 obtained lock
SUB-TC #2: Mutex created not signaled, lock unlock test
go 3 try to obtain lock
go 4 try to obtain lock
go 5 release lock
go 3 obtained lock
go 3 release lock
go 4 obtained lock
go 4 release lock
SUB-TC #3: Mutex tryLock/isLock test
SUB-TC #3: PASS



