--TEST--
A hello world go routine

--FILE--
<?php
use \go\Scheduler;

go(function(){
	echo "Hello World";
});

Scheduler::RunJoinAll();

?>
--EXPECT--
Hello World
