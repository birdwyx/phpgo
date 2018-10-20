--TEST--
Go compatibility test for file_get_contents

--FILE--
<?php
use \Go\Scheduler;
use \Go\Runtime;
use \Go\Mutex;

$mutex = new Mutex();

//go_debug( 0x1 << 13);

go( function() use($mutex){
	$i = 0;
	while($i++ < 3){
		$res = file_get_contents("http://www.baidu.com");
		if( !strstr($res, '百度') ){
			echo "access baidu failure\n";
			var_dump($res);
			break;
		}
		usleep(500*1000);
	}
	
	echo "success\n";
});

go( function() use($mutex){
	$i = 0;
	while($i++ < 3){
		$res = file_get_contents("https://www.bing.com");
		if( !strstr($res, 'Bing') ){
			echo "access bing failure\n"; 
			var_dump($res);
			break;
		}
		usleep(1*1000);
	}
	
	echo "success\n";
});

Scheduler::join();

?>
--EXPECT--
success
success
