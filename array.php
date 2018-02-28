<?php

use \go\Chan;
use \go\Mutex;
use \go\Waitgroup;
use \go\Scheduler;
use \go\Timer;
use \go\Runtime;

if(0){
	$ch = new Chan(["name"=>"dd", "copy"=>true, "capacity"=>1]);
	
	var_dump($ch);
	
	exit;
}

if(0){
	class T extends Thread{
		public function run() {
			while(true){
				echo "about to run\n";
				//Scheduler::RunJoinAll();
				sleep(1);
				echo "all tasks completed\n";
				break;
			}
		}
	}
	$t1 = new T();
	$t1->start();
	$t1->join();
	echo "Main trhead: after run\n";
	exit;
}

if(0){
	
	$ch = new Chan(1,"dd",true);
	var_dump($ch);
	
	$ch1 = new Chan("dd");
	
	var_dump($ch1);
	
	go(function() use($ch){
		echo "1\n";
		$i = 0;
		while($i<300){
			$ch->Push($i);
			$i++;
			//usleep(1000*10);
		}
		echo "1.o\n";
	});
	
	go(function() use($ch){
		echo "2\n";
		
		$i = 0;
		while(true){
			$i++;
			var_dump("main thread go pop:". var_export($ch->Pop(),true));
			if($i>=100) break;
		}
		echo "2.o\n";
	});
	
	class AAA extends Thread{
		public function run() {
			//go_initialize_thread();
			
			go(function(){
				$ch = new Chan("dd");
				var_dump("in run: dd=". var_export($ch, true));
				
				$i = 0;
				while(1){
					$i++;
					$data = $ch->Pop();
					var_dump("in thread: from ch: ".var_export($data, true));
					
					if($i>=100) break;
				}
			});
			
			echo "about to run\n";
			while(true){
				Scheduler::RunJoinAll();
				//sleep(1);
				break;
			}
			echo "all tasks completed\n";
		}
	}
	
	$t = new AAA();
	$t->start();
	
	$t1 = new AAA();
	$t1->start();
	
	//$t->join();
	
	echo "3\n";
	Scheduler::RunJoinAll();
	
	exit;
	
}
	
if(0){
	
	//go_debug( 0x1 << 8);
	
	$arr[1] = new Chan(1,"dfd",true);
	
	$ch1 = new Chan("dfd");
	
	go( function() use($ch1){
		echo "in go 1\n";
		$a = array("key1"=>"b", "key2"=>1, "xxx"=>100, "arr"=> [1,2,3, "4"=> "5"] );
		$a["recurse"] = &$a;
		
		var_dump($a);
		$ch1->Push($a);
		echo "leaving go 1\n";
	});
	
	echo "main pop\n";
	var_dump ($arr[1]->Pop());
	
	exit;
}

//exit;

if(0){
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

	$mutex = new Mutex();

	//var_dump($mutex);

	go(function($mutex){
		$mutex->lock();
		$mutex->lock();
		$mutex->unlock();
		$mutex->unlock();
	}, $mutex);

	$mutex->lock();
	$mutex->unlock();

	//var_dump($mutex);


	scheduler::runjoinall();

	//go_schedule_all();

	var_dump($mutex);

	//exit;
	//$chan1;

	function f(){
		//global $chan1;
		
		$chan = new Chan(1);
		
		//$chan1 = $chan;
		
		//var_dump( "close chan: ". var_export(go_chan_close($chan),true));
		var_dump($chan);
		//var_dump($chan1);
		
		$chan->Push(1111);
		
		$chan = null;
	}

	f();
	
	Scheduler::RunJoinAll();
}

$ls = 0;
if($ls){
	//global $aaaa;
	/*
	$aaaa = 1000;
	$wg = new Waitgroup();
	
	function xxx(){
		global $aaaa;
		echo "aaaa = $aaaa\n";
	}
	
	xxx();

	//var_dump($wg);
	 
	function f($i, $wg){
		echo "i am $i\n";
		echo "Gs:". Runtime::NumGoroutine() .PHP_EOL;
			//sleep(rand(1,10));
		
		$wg->Add(1);
		go('g', $i, $wg);
		
		//var_dump($wg);
		$wg->Done();
	}


	function g($i,$wg){
		echo "i am in $i inner go: \n";
		$wg->Done();
	}


	for($i=0; $i<1; $i++){
		global $wg;
		$wg->Add(1);
		
		go( 'f', $i, $wg );
	}*/
	/*
	class C {
		public $no;
		public function __construct($no){
			echo "C::__construct $no\n";
			$this->no = $no;
		}
		
		public function say(){
			echo "I am C $this->no\n";
		}
		
	}*/

	class T extends Thread{
		/*private $c;
		public function __construct($c){
			$this->c = $c;
		}
		
		public function setc($c){
			$this->c = $c;
		}*/
		
		public function run() {
			//go_initialize_thread();
		
				
			//$var = $this->shift();
			//var_dump($var);

			
		
			//$this->c->say();
			/*
			$wg = new Waitgroup();
			for($i=0; $i<1; $i++){
				//global $wg;
				$wg->Add(1);
				echo "add $i\n";
				go( function($i,$wg){
					echo "go $i\n";
					$wg->Done();
				}, $i, $wg);
			}*/
			
			while(true){
				echo "about to run\n";
				//Scheduler::RunJoinAll();
				sleep(1);
				echo "all tasks completed\n";
				break;
			}
		}
	}

	//$v1 = new C(1); $v2 = new C(2);
	$t1 = new T();
	//$t2 = new T();
	//$v1->no = 1111; $v2->no=2222;
	
	///$t1->setc($v1); $t2->setc($v2);
	
	//$t1[]= ['a'=>100, 'b'=>200 ];
	//$t2[]= "t2";
	//$t3 = new T();
	//$t4 = new T();


	$t1->start();
	//$t2->start();
	//$t3->start();
	//$t4->start();

	/*
	$i = 0;
	for(;;){
		echo "push to t1: $i\n";
		$t1[]= $i++;
		sleep(1);
	}*/

	$t1->join();
	//$t2->join();
	//$t3->join();
	//$t4->join();

	//$wg->Wait();

	echo "Main trhead: after run\n";

	//Scheduler::RunJoinAll();


	exit;

}

$ch = new Chan(1);

var_dump($ch);

//var_dump( go_chan_close($ch) );


//go_schedule_all();


// mixed _case(mixed $ch, string rw, mixed& $value, callable callback)

$v = 100;

go( function() use($ch, $v){
	$done = new Chan(1);
	var_dump($done);
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
	
	echo "l is:\n";
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

Scheduler::RunJoinAll();

$ch = new Chan(1);
$ch->Pop();


//$ch->Pop();

//Scheduler::RunJoinAll();
//go_schedule_all();
echo "num g: ". Runtime::NumGoroutine() . PHP_EOL;

echo "at the end\n";