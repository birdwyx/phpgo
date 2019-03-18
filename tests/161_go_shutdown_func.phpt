--TEST--
Go shutdown function test

--FILE--
<?php
use \Go\Scheduler;

register_shutdown_function(function(){
	echo "this is shutdown function outside a go\n";
	exit;
});

go(function(){
	register_shutdown_function(function(){
		echo "this is shutdown function inside go1\n";
		//exit;
	});

	echo "go1 returns\n";
});

go(function(){
	register_shutdown_function(function(){
		echo "this is shutdown function inside go2\n";
		exit;
	});

	echo "go2 returns\n";
});

Scheduler::join();

echo "main script returns\n";
?>
--EXPECT--
go1 returns
this is shutdown function inside go1
go2 returns
this is shutdown function inside go2
main script returns
this is shutdown function outside a go
