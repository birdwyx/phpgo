--TEST--
Go select read

--FILE--
<?php
use \go\Chan;
use \go\Scheduler;

function subtc($seq){
    echo "SUB-TC: #$seq\n";
}

subtc(1);
$ch = new Chan(["capacity"=>10]);

go(function($ch){
	for($i = 1; $i<=20; $i++){
		//echo "push $i:\n";
		$ch->push($i);
	}
},$ch);


go(function() use($ch){
	$v = -1; $u = $v;  $i=0; 
	$cases = ['case', 'CasE', 'CAse', 'CASe', 'CASE'];
	while(true){
		select(
			[
				$cases[$i%5], $ch, "->", &$v, function($v){
					var_dump($v);
				}
			]
		);
		if($i==20) break;
		$i++;
		usleep(100);
	}
	echo 'now $v is: '. $v .PHP_EOL;
	echo 'now $u is: '. $u .PHP_EOL;
});

Scheduler::join();

?>
--EXPECT--
SUB-TC: #1
int(1)
int(2)
int(3)
int(4)
int(5)
int(6)
int(7)
int(8)
int(9)
int(10)
int(11)
int(12)
int(13)
int(14)
int(15)
int(16)
int(17)
int(18)
int(19)
int(20)
now $v is: 20
now $u is: -1
