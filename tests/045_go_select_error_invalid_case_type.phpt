--TEST--
Go Select invalid case type

--FILE--
<?php
use \go\Chan;
use \go\Scheduler;

function subtc($seq){
    echo "SUB-TC: #$seq\n";
}

$ch = new Chan(["capacity"=>10]);

//error_reporting(0);

go(function() use($ch){
	$seq = 1;
	
	subtc(1);  // unexpected extra parameter
	$read = 0; $write = 0; $df = 0; $v=1111;
	$ch->push(1);

	select(
		[
			'invalid', $ch, '->', &$v, function($v) use(&$read){
				if($v===1)
					$read = 1;
			}
		],
		[
			'default', function() use(&$df){
				$df = 1;
			}, 'unexpected extra parameter'
		]
	);

});


Scheduler::join();

?>
--EXPECTREGEX--
.*invalid case type invalid.*
