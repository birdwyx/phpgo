--TEST--
Go a hello world routine

--FILE--
<?php
use \Go\Scheduler;

go(function(){
	echo "Hello World";
});

Scheduler::join();

?>
--EXPECT--
Hello World
