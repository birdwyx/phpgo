--TEST--
Go select shortcut syntax

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

	ob_start();
	select(
		[
			'case', $ch, '->', &$v, function($v) use(&$read){
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
	$res = ob_get_clean();
	if( !preg_match("/.*case 2: unexpected extra parameter detected after the callable.*/s", $res) ){
		echo "verify unexpected extra parameter can be detected: failed\n";
	}

	assert($read===1 && $df===0);
	echo "success\n";
	
	subtc(2); //receiving variable ommited
	$read = 0; $write = 0; $df = 0; $v=1111;
	$ch->push(1);

	select(
		[
			'case', $ch, '->', function($v) use(&$read){
				if($v===1)
					$read = 1;
			}
		],
		[
			'default', function() use(&$df){
				$df = 1;
			}
		]
	);

	assert($read===1 && $df===0);
	echo "success\n";
	
	subtc(3); //operator and receiving variable ommited
	$read = 0; $write = 0; $df = 0; $v=1111;
	$ch->push(1);

	select(
		[
			'case', $ch, function($v) use(&$read){
				if($v===1)
					$read = 1;
			}
		],
		[
			'default', function() use(&$df){
				$df = 1;
			}
		]
	);

	assert($read===1 && $df===0);
	echo "success\n";

});


Scheduler::join();

?>
--EXPECT--
SUB-TC: #1
success
SUB-TC: #2
success
SUB-TC: #3
success


