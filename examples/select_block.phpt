<?php
use go\Chan;
use go\Scheduler;

$ch = new Chan(1);
go(function() use($ch){
	$v = 0;
    select(
		[
			'case', $ch, function($v){
				echo "read from channel: ". $v;
			}
		]
	);
	echo "after select\n";
});

$ch->push("hello world\n");

Scheduler::join();