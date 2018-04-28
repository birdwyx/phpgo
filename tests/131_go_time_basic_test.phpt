--TEST--
Go time basic test

--FILE--
<?php
use \Go\Chan;
use \Go\Time;
use \Go\Scheduler;

function subtc($seq){
    echo "SUB-TC: #$seq\n";
}

go_debug(0);

subtc(1);
echo Time::Nanosecond . PHP_EOL;
echo Time::Microsecond . PHP_EOL;
echo Time::Millisecond . PHP_EOL;
echo Time::Second . PHP_EOL;   
echo Time::Minute . PHP_EOL;     
echo Time::Hour . PHP_EOL;      

subtc(2);
echo "verify sleep of 1 second : outside of go routine: ";
$t = microtime(true);
Time::sleep(Time::Second);
$t1 = microtime(true);
if ( $t1 - $t > 1.001 || $t1 - $t < 0.999 ){
	echo "should sleep 1 second but slept " 
         . ( $t1-$t ) . " seconds\n";
}else{
	echo "pass\n";
}

subtc(3);
go(function (){  //go 1
	$chan = Time::After(1*Time::Second);  //go 2
	
	go(function () use($chan){   //go 3
		$time0 = time();
		while( true ){
			$v = $chan->tryPop() ;
			if( !empty($v) ) break;
			Time::sleep(20 * Time::Millisecond); // = usleep(20 * 1000);
			if( time()-$time0 > 1.1 ) {
				echo "time should expire at 1 second but expired at " 
				     . ( time()-$time0 ) . " seconds\n"; 
				return;
			}
		}
		echo "verify Time::After : 1 second : pass\n";
	});
});

Scheduler::join();

subtc(4);
go(function (){  //go 1
	$c = 2000;
	for($i=0;$i<$c; $i++){
		$ch[$i] = Time::After($i*Time::Millisecond);
	}
	
	for($i=0;$i<$c; $i++){
		if( empty($ch[$i]) ){
			echo "some timers were failed to start\n";
			return;
		}
	}
	
	go( function() use($ch, $c){
		$count = 0;
		$time0 = time();
		while($count<$c){
			for($i=0; $i<$c; $i++){
				$p = $ch[$i]->TryPop();
				if(!empty($p)){
					$count ++;
					//echo "ch $i returns $p\n";
				}
			}
			
			Time::sleep(10 * Time::Millisecond); // = usleep(10*1000);
			
			if( time()-$time0 > ($c+100) / 1000 ) {
				echo "time should expire at ". ($c/1000) .
				     " seconds but expired at " . (time()-$time0) . 
					 " seconds\n"; 
				return; 
			}
		}
		echo "verify start of 2000 timers : pass\n";
	});
});

Scheduler::join();

subtc(5);
go(function (){  //go 1

	$ch = Time::tick(100*Time::Millisecond);
	$done = new Chan(1);
	
	go( function() use($ch, $done){
		select(
			['case', $ch, function($v) use($done){
				static $t0 = 0;
				$t1 = microtime(true);
				if($t0){
					if( $t1-$t0 > 0.12 || $t1 - $t0 < 0.08 ){
						echo "timer interval expected to be 0.1 seconds, ". ($t1-$t0) ." in actual\n";
					}
				}
				$t0 = $t1;
				//echo microtime(true) . ":". $v . PHP_EOL;
				
				static $i = 0; 
				if($i++ >= 10) {
					$done->close();
				}
			}]
		)->loop($done);
		
		echo "verify Timer::tick(): pass\n";
	});
});

Scheduler::join();

?>
--EXPECT--
SUB-TC: #1
1
1000
1000000
1000000000
60000000000
3600000000000
SUB-TC: #2
verify sleep of 1 second : outside of go routine: pass
SUB-TC: #3
verify Time::After : 1 second : pass
SUB-TC: #4
verify start of 2000 timers : pass
SUB-TC: #5
verify Timer::tick(): pass
