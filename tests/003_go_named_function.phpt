--TEST--
Go named function

--FILE--
<?php
use \Go\Scheduler;

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
