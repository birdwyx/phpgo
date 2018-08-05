--TEST--
Go scheduler run

--SKIPIF--


--FILE--
<?php
use \Go\Mutex;
use \Go\Scheduler;
use \Go\Runtime;

function subtc($seq){
    echo "SUB-TC: #$seq\n";
}

subtc(1);

go(function(){
	echo "return from go 1\n";
});

go(function(){
	echo "go 2\n";
	usleep(100*1000);
	echo "return from go 2\n";
	
});

go(function(){
	echo "go 3\n";
	usleep(100*1000);
	echo "return from go 3\n";
});

go(function(){
	echo "go 4\n";
	usleep(200*1000);
	echo "return from go 4\n";
});

$pass = 0;
while(true){
	$pass++;
	$run = Scheduler::run();
	if($run>0)
		echo "^run $run tasks\n";
	
	if(Runtime::numGoroutine() ===0 )
		break;
}

subtc(2);

go(function(){
	echo "go 1\n";
});
go(function(){
	usleep(100*1000);
	echo "go 2\n";
});

Scheduler::join(1);
echo "after join(1)\n";
Scheduler::join(0);

subtc(3);

go(function(){
	echo "go 1\n";
});
go(function(){
	usleep(100*1000);
	echo "go 2\n";
});
go(function(){
	usleep(200*1000);
	echo "go 3\n";
	Runtime::quit(1);
});

Scheduler::loop();

?>
--EXPECT--
SUB-TC: #1
return from go 1
go 2
^run 2 tasks
go 3
^run 1 tasks
go 4
^run 1 tasks
return from go 2
^run 1 tasks
return from go 3
^run 1 tasks
return from go 4
^run 1 tasks
SUB-TC: #2
go 1
after join(1)
go 2
SUB-TC: #3
go 1
go 2
go 3


