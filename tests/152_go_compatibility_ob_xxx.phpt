--TEST--
Go compatibility test for ob_xxx functions

--FILE--
<?php
use \Go\Scheduler;

//go_debug( 0x1 << 13);


go(function(){
	ob_start();
	sleep(2);
	echo "go 1\n";
	
	ob_end_clean();
});

go(function(){
	ob_start();
	echo "go 2\n";
	sleep(1);
	ob_end_flush();
});

Scheduler::join();

?>
--EXPECT--
