<?php

//$a = "world";

//sleep(1);

///---- debugger flags
/*
static const uint64_t dbg_none              = 0;
static const uint64_t dbg_all               = ~(uint64_t)0;
static const uint64_t dbg_hook              = 0x1;
static const uint64_t dbg_yield             = 0x1 << 1;
static const uint64_t dbg_scheduler         = 0x1 << 2;
static const uint64_t dbg_task              = 0x1 << 3;
static const uint64_t dbg_switch            = 0x1 << 4;
static const uint64_t dbg_ioblock           = 0x1 << 5;
static const uint64_t dbg_wait              = 0x1 << 6;
static const uint64_t dbg_exception         = 0x1 << 7;
static const uint64_t dbg_syncblock         = 0x1 << 8;
static const uint64_t dbg_timer             = 0x1 << 9;
static const uint64_t dbg_scheduler_sleep   = 0x1 << 10;
static const uint64_t dbg_sleepblock        = 0x1 << 11;
static const uint64_t dbg_spinlock          = 0x1 << 12;
static const uint64_t dbg_fd_ctx            = 0x1 << 13;
static const uint64_t dbg_debugger          = 0x1 << 14;
static const uint64_t dbg_signal            = 0x1 << 15;
static const uint64_t dbg_sys_max           = dbg_debugger;
*/

go_debug($argv[1]);
//go_debug( 0x1 << 11 |  0x1 << 10 | 0x1 << 9 | 0x1 << 2 | 0x1 << 3 );



/*
go(function(){
	sleep(1);
	echo "go2..............\n";
});

go(function(){
	//mysleep(1);
	echo "go3..............\n";
});
go(function(){
	//mysleep(1);
	echo "go4..............\n";
});
go(function(){
	//mysleep(1);
	echo "go5..............\n";
});

//go( 'f1');
//go( 'f2');

echo "0\n";
//$res = go( function(){
	//mysleep(1);
	//echo "i am go0 callback in php\n";  
	//mysleep(1);
//});


go('f1');

echo "1\n";

go('f1');

*/
$ch = go_make_chan(1);
$ch2 = go_make_chan(0);

//echo "global scope: ch=";
//var_dump($ch);

function x($v){
	
	echo "xxxx $v\n";
	
}

class E extends Exception{
	public $name = 'abc';
}

function f1(){
	
	global $ch;
	
	echo "i am f1\n";
	
	$a = "a";
	
	go_chan_push($ch, $a ); //array("a"=>"b", "c"=>"d"));
	
	echo "f1-1: ch=";
	
	//$xxx= 1;
	$a = $xxx;
	
	/*
	try{
	
	throw new E();
	
	}catch(\Exception $e){
		echo "caught an exception\n";
	}*/
	
	$a = 1;
	//$yyy();
	
	echo "f1-2: ch=";
	var_dump($ch);
	
	echo "return from f1\n";
}

function f2(){
	
	global $ch;
	
	echo "f2: ch=";
	var_dump($ch);
	
	echo "i am f2\n";
}

function f3(){
	global $ch;
	
	echo "f3: ch=";
	var_dump($ch);
	
	$v = go_chan_pop($ch);
	
	var_dump($v);
	
	echo "receive $v from ch\n";
	
}

function redis(){
	echo "i am redis\n";
	$redis = new \Redis();
	$redis->connect('127.0.0.1', 6379);
	
	var_dump($redis);
	
	$redis->set("foo", "1111\n");
	echo $redis->get("foo");
	$redis->set("foo", "2222\n");
	echo $redis->get("foo");
	$redis->set("foo", "3333\n");
	sleep(1);
	echo $redis->get("foo");
	$redis->set("foo", "4444\n");
	echo $redis->get("foo");
	$redis->set("foo", "5555\n");
	echo $redis->get("foo");
	$redis->set("foo", "6666\n");
	sleep(2);
	echo $redis->get("foo");
	$redis->set("foo", "7777\n");
	echo $redis->get("foo");
	$redis->set("foo", "8888\n");
	echo $redis->get("foo");
	
	global $ch;
	go_chan_push($ch, $redis->get("foo"));
}


function ch2(){
	global $ch2;
	echo "anno func\n";
	go_chan_push($ch2, "push from anno func");
};
	
function root(){
	global $ch;
	$co = go('redis');
	$v = "8888";
	echo "receive from ch:". var_export($v,true) .PHP_EOL;
	
	go('ch2');
	global $ch2;
	$res = go_chan_pop($ch2);
	var_dump($res);
	
	go('f1');
	go('f2');
	go('f3');
	
	$ch2 = go_make_chan(1);
	go(function() use($ch2){
		echo "anno func\n";
		go_chan_push($ch2, "push from anno func");
	});
	$res = go_chan_pop($ch2);
	var_dump($res);
	
	//$v = go_chan_pop($ch);
	var_dump($v);
	
	
	go(function(){
		$a = 100;
		sleep(1);
		var_dump($a);
	});
	
	go(function(){
		$a = 200;
		sleep(1);
		go(function(){
			$b = 2000;
			sleep(1);
			var_dump($b);
		});
		var_dump($a);
	});
	
	go(function(){
		$a = 300;
		sleep(1);
		var_dump($a);
	});
}

$f1 = 100;

function root1(){
	$ch2 = go_make_chan(1);
	go(function() use($ch2){
		echo "anno func\n";
		go_chan_push($ch2, "push from anno func");
	});
	$res = go_chan_pop($ch2);
	var_dump($res);
	
	go('f1');
	
	global $ch;
	$co = go('redis');
	
	

	
	go('f2');
	go('f3');


	$v = go_chan_pop($ch);
	echo "receive from ch:". var_export($v,true) .PHP_EOL;

}

//go('root');

/*
go(function(){
	echo "abc\n";
});
*/

/*

$res = go_await( function() use($a){
	echo  "i am go_await callback in php\n";
	return "hello $a\n";
});*/

//echo "4\n";



//echo "5\n";

//go_schedule_all();

//echo $res .PHP_EOL;

//echo "6\n";

//$server = new swoole_server('127.0.0.1', 9999);

//var_dump($server);

/*
go(function(){
	$f = 100;$g = 100;$h = 100;
	go(function(){
		$f = 1000;$g = 1000;$h = 1000;
		go(function(){
			$f = 10000;$g = 10000;$h = 10000;
			echo "layer3: $f, $g, $h\n";
		});
		sleep(1);
		echo "layer2: $f, $g, $h\n";
	});
	sleep(2);
	
	echo "layer1: $f, $g, $h\n";
});*/

function f($v){
	$f = 100;$g = 100;$h = 100;
	g($f, $g, $h);
	//sleep(rand()%3 + 1);
	echo "layer1: $f, $g, $h\n";
}

function g($f, $g, $h){
	$f = 1000;$g = 1000;$h = 1000;
	h($f, $g, $h);
	//sleep(rand()%3 + 1);
	echo "layer2: $f, $g, $h\n";
}

function h($f, $g, $h){
	$f = 10000;$g = 10000;$h = 10000;
	sleep(1);
	echo "layer3: $f, $g, $h\n";
}

function ff(){
	$f = 9999;$g = 9999;$h = 9999;
	gg($f, $g, $h);
	//sleep(rand()%3 + 1);
	echo "ff layer1: $f, $g, $h\n";
}

function gg($f, $g, $h){
	$f = 8888;$g = 8888;$h = 8888;
	hh($f, $g, $h);
	//sleep(rand()%3 + 1);
	echo "gg layer2: $f, $g, $h\n";
}

function hh($f, $g, $h){
	$f = 7777;$g = 7777;$h = 7777;
	sleep(1);
	echo "hh layer3: $f, $g, $h\n";
}

function xxx($a, $b, $c, $d){
	
	echo "test xxx $a, $b, $c\n";
	var_dump($d);
	
}
go( function(){
	echo "test go\n";
});


go('root');

//sleep(2);
go('f');
sleep(1);
go('ff');

$x = 111; $y = 222; $z = 333;
go(function($v, $j) use($x, $y, $z){
	echo "v,j,x,y,z:".$v.",".$j.",".$x.",".$y.",".$z."\n";
	call_user_func('xxx', 100,200, 'testarg3', array('a'=>'b', 'c'=>'d'));
}, 100, array('hello'=>'world!'));


class C{
	function printc(){
		echo "i am c\n";
	}
}

$c = new C();

echo "go C::printc\n";
go( array($c, "printc") );

go_schedule_all();




echo "7\n";




//exit();

try{

	register_shutdown_function( function(){
		echo "i am shutting down\n";
	});
	
	

}catch(\Exception $e){
	
	echo $e->getMessage() .PHP_EOL;
}




//sleep(1000);

/*
$serv = new swoole_server('0.0.0.0', 9501, SWOOLE_BASE, SWOOLE_SOCK_TCP);

var_dump($serv);


Swoole\Event::defer(function(){
	echo "cycle\n";
});

$serv->run();
*/

