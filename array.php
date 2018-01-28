<?php

use \go\Chan;
use \go\Mutex;
use \go\Waitgroup;
use \go\Scheduler;
use \go\Timer;
use \go\Runtime;



//exit;

if(false){
	echo "num g: ". runtime::NumGoroutine() . PHP_EOL;

	Runtime::Gosched();

	$time = Timer::After(1000*1000);

	var_dump($time);

	$v = $time->Pop();

	var_dump($v);

	$mtx = new Mutex(false);
	var_dump($mtx);

	$wg = new Waitgroup();
	var_dump($wg);

	$wg->Add(2);
	go(function($mtx) use($wg){
		$mtx->lock();
		sleep(2);
		$mtx->unlock();
		
		$wg->Done();
	}, $mtx);

	go(function($mtx) use($wg){
		$mtx->lock();
		sleep(2);
		$mtx->unlock();
		
		$wg->Done();
		
		$wg->wait();
	}, $mtx);

	//go_schedule_once();

	$mtx->unlock();

	$wg->wait();

	$mtx->lock();



	var_dump('trylock returns:' . var_export($mtx->TryLock(),true));




	//exit;

	$chan = new Chan(1);
	var_dump($chan);

	go(function() use($chan){
		$chan->Push(11111);
	});
	//$chan->push(100);

	//go(function() use($chan){
		$pop = $chan->Pop();
		var_dump($pop);
	//});



	//go_schedule_all();

	//exit;

	//\go\go_debug(1, 1);

	$mutex = go_mutex_create();

	//var_dump($mutex);

	go(function($mutex){
		go_mutex_lock($mutex);
		go_mutex_lock($mutex);
		go_mutex_unlock($mutex);
		go_mutex_unlock($mutex);
	}, $mutex);

	go_mutex_lock($mutex);
	go_mutex_unlock($mutex);

	//var_dump($mutex);


	scheduler::runjoinall();

	//go_schedule_all();

	go_mutex_destroy($mutex);

	var_dump($mutex);

	//exit;
	//$chan1;

	function f(){
		//global $chan1;
		
		$chan = go_chan_create(1);
		
		//$chan1 = $chan;
		
		//var_dump( "close chan: ". var_export(go_chan_close($chan),true));
		var_dump($chan);
		//var_dump($chan1);
		
		go_chan_push($chan, 1111);
		
		$chan = null;
	}

	f();
	
	Scheduler::RunJoinAll();
}

$ch = new Chan(1);

//var_dump( go_chan_close($ch) );


//go_schedule_all();


// mixed _case(mixed $ch, string rw, mixed& $value, callable callback)

$v = 100;

go( function() use($ch, $v){
	$done = new Chan(1);
	$i  = 0;
	$sel = select(
		/*_case($ch, "<-", 2, function($value){
			
			echo "sent v: $value\n";

		}),*/
		_case($ch, "->", $v, function($value){
			
			
			echo "receive v: $value\n";
		}),
		_case(Timer::After(1000*1000), "->", null, function() use($done){
			
			$done->Push( array("a"=>"b", "c"=>"d"));
			echo "timer elapsed---------------\n";
			
		}),
		_case($done, "->", null, function($value){
			
			
			echo "i am told to finish\n";
			
		}),
		_default(function(){
			usleep(100*1000);
			echo "i am default\n";
		})
	);
	
	//Runtime::Gosched();
	
	echo "num g: ". runtime::NumGoroutine() . PHP_EOL;
	
	var_dump($sel);
	
	$l = $sel->Loop($done);
	
	var_dump($l);
	
	var_dump($sel);
	
	$l = $sel->Loop($done);
	
	var_dump($l);
		
	while(1){
		echo "$i round:".PHP_EOL;

		
		
		//$sel->Select();
		//select(select(select($sel)));
		
		
		
		//var_dump($sel);
		
		if($i == 2) {
			//$done->Close();
		}
		
		//Runtime::Gosched();
		
		if($i++>100) break;
		usleep(50*1000);
	}
	
	echo "v:$v\n";
	
});

echo "num g: ". Runtime::NumGoroutine() . PHP_EOL;
/*
go(function(){
	$i = 0;
	
	$ch = new Chan(1);
	while(1){
		echo "$i seconds...".PHP_EOL;
		sleep(1);
		
		var_dump( $ch->TryPush(1000) );
		var_dump( $ch->TryPop() );
		
		var_dump( $ch->TryPush(1001) );
		var_dump( $ch->TryPush(1002) );
		var_dump( $ch->TryPop() );
		var_dump( $ch->TryPop() );
		
		$i++;
	}
});*/

//$v = go_chan_pop();

//Scheduler::RunForeverMultiThreaded(4);

$ch = new Chan(1);
$ch->Pop();


//$ch->Pop();

//Scheduler::RunJoinAll();
//go_schedule_all();
echo "num g: ". Runtime::NumGoroutine() . PHP_EOL;

echo "at the end\n";