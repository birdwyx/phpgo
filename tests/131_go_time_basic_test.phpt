--TEST--
Go time basic test

--FILE--
<?php
use \Go\Chan;
use \Go\Timer;
use \Go\Scheduler;

function subtc($seq){
    echo "SUB-TC: #$seq\n";
}

go_debug(0);

subtc(1);
go(function (){  //go 1
	$chan = Timer::After(1000*1000);  //go 2
	
	go(function () use($chan){   //go 3
		$time0 = time();
		while( true ){
			$v = $chan->tryPop() ;
			if( !empty($v) ) break;
			usleep(20 * 1000);
			if( time()-$time0 > 1 ) return;
		}
		echo "got $v\n";
	});
});

Scheduler::join(1);

subtc(2);
go(function (){  //go 1
	$c = 10000;
	for($i=0;$i<$c; $i++){
		$ch[$i] = Timer::After($i*1000);
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
			usleep(10*1000);
			
			if( time()-$time0 > ($c+1000) / 1000 ) return; 
		}
		echo "all timers expired\n";
	});
});

Scheduler::join(1);
?>
--EXPECT--
SUB-TC: #1
got 1
SUB-TC: #2
all timers expired


