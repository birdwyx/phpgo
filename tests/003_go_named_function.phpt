--TEST--
Go named function

--FILE--
<?php
use \go\Scheduler;

function f(){
	echo "Hello World\n";
}

go('f');

$var = 'f';
go($var);

Scheduler::join();

?>
--EXPECT--
Hello World
Hello World
