<?php


$x = 100; $y=100;/*
go_debug( $x, function($arg){
	$arg = 99999;
});

echo $x .PHP_EOL;
*/
/*
go( function(&$a, $b){
   sleep(1);
   $a = 11111111;
   echo $a . ", " .$b ."\n";
}, $x, $y );*/

//go_schedule_all();

//echo $y .PHP_EOL;

//exit;
function go2( callable $callback, &$arg1, $arg2 ){
	$callback($arg1, $arg2);
}

$x = 1000; $y = 1000;

go( function(&$arg, $arg2){$arg = 99999; echo $arg; echo "\n";}, $x, $y );

go_schedule_all();

echo $x . ", " .$y ."\n";


