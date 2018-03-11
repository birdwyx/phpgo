--TEST--
Go Select read write

--FILE--
<?php
use \go\Chan;
use \go\Scheduler;

function subtc($seq){
    echo "SUB-TC: #$seq\n";
}

subtc(1);
$ch = new Chan(["capacity"=>1000]);


go(function() use($ch){
	for($i=0;$i<500;$i++){
		$ch->Push($i);
	}
	$v = -1; $u = $v; $fail = false;
	while(true){
		select(
			_case($ch, "->", $v, function($data) use($i, &$read){
				$read++;
				//echo "pass $i: read from chan: $data\n";
			}),
			_case($ch, "<-", $i, function($data) use($i, &$write){
				$write++;
				//echo "pass $i: written to chan: $data\n";
			})
			/*,
			_default(function(){
				echo "this is default\n";
				sleep(1);
			})*/
		);
		if( abs($write-$read) > 50 ) {
			echo "randomness check failed: diff: " .($write-$read) .PHP_EOL;
			$fail = true;
		}
		$i++;
		if($i==1000) break;
		usleep(100);
	}
	if($fail) echo "failed\n";
	else echo "success\n";
});

Scheduler::RunJoinAll();

?>
--EXPECT--
SUB-TC: #1
success
