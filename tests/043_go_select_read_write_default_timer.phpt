--TEST--
Go select read write default timer

--FILE--
<?php
use \Go\Chan;
use \Go\Time;
use \Go\Scheduler;

function subtc($seq){
    echo "SUB-TC: #$seq\n";
}

subtc(1);
$done = new Chan(1);

go(function() use($done){

	$begin = time();
	$dummy = 0;
	$sel = select(
		[ 'case', Time::after(1*Time::Second), "->", &$dummy, function($v) use($done, $begin){
			//assert( $v===1 );
			//var_dump($v);
			assert( time()-$begin == 1 );
			
			$done->push("done");
		}]
	);
	
	$ret = $sel->loop($done);
	if(assert($ret=="done")){
		echo "success\n";
	}

});

Scheduler::join();

subtc(2);
$done = new Chan(1);

go(function() use($done){

	$begin = microtime(true); $i=0;
	
	//echo $begin;
	$dummy = 0;
	$sel = select(
		[ 'case', Time::tick(100*Time::Millisecond), /*"->", &$dummy, */ function($v) use($done, &$begin, &$i){
			//assert( $v===1 );
			assert( ($diff = abs( microtime(true)-$begin-0.1 )) < 0.05 );
			//echo $diff . " ";
			$begin = microtime(true);
			
			$i++;
		}],
		['default', function(){
			usleep(1*1000);
		}]
	);
	 
	while($i<10){
		$sel = $sel->select();
	}
	
	echo "test completed\n";
	
	//exit;

});

Scheduler::join();

?>
--EXPECT--
SUB-TC: #1
success
SUB-TC: #2
test completed


