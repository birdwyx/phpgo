--TEST--
Go compatibility test for curl

--FILE--
<?php
use \Go\Scheduler;
use \Go\Runtime;
use \Go\Mutex;

$mutex = new Mutex();

function post($url, $post, $return_array =  true){

	$ch = curl_init();
	curl_setopt($ch, CURLOPT_URL,$url);  
	curl_setopt($ch, CURLOPT_HEADER, 0);   
	curl_setopt($ch, CURLOPT_POST, 1);  
	curl_setopt($ch, CURLOPT_POSTFIELDS,  $post);  
	curl_setopt($ch, CURLOPT_AUTOREFERER,true);  
	curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1); 
	curl_setopt($ch, CURLOPT_FOLLOWLOCATION, 1);
	if(!empty($GLOBALS['COOKIE'])) 
		curl_setopt($ch, CURLOPT_COOKIE, $GLOBALS['COOKIE']);
	
	$result = curl_exec($ch);
	curl_close($ch);
	
	if( $return_array && ( $arr = json_decode($result, true) ) !== NULL){
		return $arr;
	}else{
		return $result;
	}
}

function get($url, $return_array =  true){
	$ch = curl_init();
	curl_setopt($ch, CURLOPT_URL,$url);  
	curl_setopt($ch, CURLOPT_HEADER, 0);    
	curl_setopt($ch, CURLOPT_AUTOREFERER,true);  
	curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1); 
	curl_setopt($ch, CURLOPT_FOLLOWLOCATION, 1);
	if(!empty($GLOBALS['COOKIE'])) 
		curl_setopt($ch, CURLOPT_COOKIE, $GLOBALS['COOKIE']);
	
	$result = curl_exec($ch); 
	curl_close($ch);

	if( $return_array && ( $arr = json_decode($result, true) ) !== NULL){
		return $arr;
	}else{
		return $result;
	}
}

//go_debug( 0x1 << 13);

go( function() use($mutex){

	$i = 0;
	while($i++ < 3){
		$res = get("http://www.baidu.com");
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
		$res = get("https://www.bing.com");
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
