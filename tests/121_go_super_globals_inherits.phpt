--TEST--
Go super globals inherits

--SKIPIF--


--FILE--
<?php
use \go\Chan;
use \go\Scheduler;
use \go\Runtime;

function subtc($seq){
    echo "SUB-TC: #$seq\n";
}

subtc(1);

$foo = "bar";

$_GET['var'] = "hello";
$_POST['var'] = "hello";
$_COOKIE['var'] = "hello";
$_SERVER['var'] = "hello";
$_ENV['var'] = "hello";
$_REQUEST['var'] = "hello";
$_FILES['var'] = "hello";

$get = $_GET;
$post = $_POST;
$cookie = $_COOKIE;
$server = $_SERVER;
$env = $_ENV;
$request = $_REQUEST;
$files = $_FILES;
$globals = $GLOBALS;

$options = [
	'stack_size' => 10 * 1024, 
	'isolate_http_globals' => true
];

goo($options, function() use($get, $post, $cookie, $server, $env, $request, $files, $globals){
	if($get == $_GET) echo "verify go routine inherits super global - get ok\n";
	if($post == $_POST)  echo "verify go routine inherits super global - post ok\n";
	if($cookie == $_COOKIE)  echo "verify go routine inherits super global - cookie ok\n";
	if($server == $_SERVER)  echo "verify go routine inherits super global - server ok\n";
	if($env == $_ENV)  echo "verify go routine inherits super global - env ok\n";
	if($request == $_REQUEST)  echo "verify go routine inherits super global - request ok\n";
	if($files == $_FILES)  echo "verify go routine inherits super global - files ok\n";
	if($globals == $GLOBALS)  echo "verify go routine inherits super global - globals ok\n";
	echo PHP_EOL;
	
	$_GET['var'] = "world";
	$_POST['var'] = "world";
	$_COOKIE['var'] = "world";
	$_SERVER['var'] = "world";
	$_ENV['var'] = "world";
	$_REQUEST['var'] = "world";
	$_FILES['var'] = "world";
	
	$options = [
		'stack_size' => 0, 
		'isolate_http_globals' => true
	];
	
	$ch = new Chan(1);
	goo($options, function() use($get, $post, $cookie, $server, $env, $request, $files, $globals, $ch){
		if($_GET['var'] == "world") echo "--inner go: verify go routine inherits super global - get ok\n";
		if($_POST['var'] == "world") echo "--inner go: verify go routine inherits super global - post ok\n";
		if($_COOKIE['var'] == "world") echo "--inner go: verify go routine inherits super global - cookie ok\n";
		if($_SERVER['var'] == "world") echo "--inner go: verify go routine inherits super global - server ok\n";
		if($_ENV['var'] == "world")  echo "--inner go: verify go routine inherits super global - env ok\n";
		if($_REQUEST['var'] == "world")  echo "--inner go: verify go routine inherits super global - request ok\n";
		if($_FILES['var'] == "world") echo "--inner go: verify go routine inherits super global - files ok\n";
		if($globals == $GLOBALS)  echo "--inner go: verify go routine inherits super global - globals ok\n";
		
		echo PHP_EOL;
		
		$_GET['var'] = "world2";
		$_POST['var'] = "world2";
		$_COOKIE['var'] = "world2";
		$_SERVER['var'] = "world2";
		$_ENV['var'] = "world2";
		$_REQUEST['var'] = "world2";
		$_FILES['var'] = "world2";
		$GLOBALS['var'] = "world2";
		
		$ch->Push("");
	});
	
	$ch->Pop();
	
	if($_GET['var'] == "world") echo "in go: verify change of super global in inner g does not change the outside - get ok\n";
	if($_POST['var'] == "world") echo "in go: verify change of super global in inner g does not change the outside - post ok\n";
	if($_COOKIE['var'] == "world") echo "in go: verify change of super global in inner g does not change the outside - cookie ok\n";
	if($_SERVER['var'] == "world") echo "in go: verify change of super global in inner g does not change the outside - server ok\n";
	if($_ENV['var'] == "world")  echo "in go: verify change of super global in inner g does not change the outside - env ok\n";
	if($_REQUEST['var'] == "world")  echo "in go: verify change of super global in inner g does not change the outside - request ok\n";
	if($_FILES['var'] == "world") echo "in go: verify change of super global in inner g does not change the outside - files ok\n";
	
	if($GLOBALS['var'] == "world2")  
		echo 'in go: verify $GLOBALS always kept updated  - ok' .PHP_EOL;
	else 
		echo 'in go: verify $GLOBALS always kept updated  - failed' .PHP_EOL;	
	
	echo PHP_EOL;
	
	if($get != $_GET) echo "in go: verify change of super global in g does not change the outside - get ok\n";
	if($post != $_POST)  echo "in go: verify change of super global in g does not change the outside - post ok\n";
	if($cookie != $_COOKIE)  echo "in go: verify change of super global in g does not change the outside - cookie ok\n";
	if($server != $_SERVER)  echo "in go: verify change of super global in g does not change the outside - server ok\n";
	if($env != $_ENV)  echo "in go: verify change of super global in g does not change the outside - env ok\n";
	if($request != $_REQUEST)  echo "in go: verify change of super global in g does not change the outside - request ok\n";
	if($files != $_FILES)  echo "in go: verify change of super global in g does not change the outside - files ok\n";
	
	if($globals == $GLOBALS)  
		echo 'in go: verify $GLOBALS always kept updated  - ok' .PHP_EOL;
	else 
		echo 'in go: verify $GLOBALS always kept updated  - failed' .PHP_EOL;
	
	echo $GLOBALS['foo'] ." ". $GLOBALS['foo'] ." black sheep :)\n";
	$GLOBALS['foo'] = "rab";
	
	echo PHP_EOL;
});

Scheduler::join();

if($get == $_GET) echo "outside of go: verify change of super global in g does not change the outside - get ok\n";
if($post == $_POST)  echo "outside of go: verify change of super global in g does not change the outside - post ok\n";
if($cookie == $_COOKIE)  echo "outside of go: verify change of super global in g does not change the outside - cookie ok\n";
if($server == $_SERVER)  echo "outside of go: verify change of super global in g does not change the outside - server ok\n";
if($env == $_ENV)  echo "outside of go: verify change of super global in g does not change the outside - env ok\n";
if($request == $_REQUEST)  echo "outside of go: verify change of super global in g does not change the outside - request ok\n";
if($files == $_FILES)  echo "outside of go: verify change of super global in g does not change the outside - files ok\n";

if($globals == $GLOBALS)  
	echo 'in go: verify $GLOBALS always kept updated  - ok' .PHP_EOL;
else 
	echo 'in go: verify $GLOBALS always kept updated  - failed' .PHP_EOL;

echo $GLOBALS['foo'] ." ". $GLOBALS['foo'] ." black sheep :)\n";
?>
--EXPECT--
SUB-TC: #1
verify go routine inherits super global - get ok
verify go routine inherits super global - post ok
verify go routine inherits super global - cookie ok
verify go routine inherits super global - server ok
verify go routine inherits super global - env ok
verify go routine inherits super global - request ok
verify go routine inherits super global - files ok
verify go routine inherits super global - globals ok

--inner go: verify go routine inherits super global - get ok
--inner go: verify go routine inherits super global - post ok
--inner go: verify go routine inherits super global - cookie ok
--inner go: verify go routine inherits super global - server ok
--inner go: verify go routine inherits super global - env ok
--inner go: verify go routine inherits super global - request ok
--inner go: verify go routine inherits super global - files ok
--inner go: verify go routine inherits super global - globals ok

in go: verify change of super global in inner g does not change the outside - get ok
in go: verify change of super global in inner g does not change the outside - post ok
in go: verify change of super global in inner g does not change the outside - cookie ok
in go: verify change of super global in inner g does not change the outside - server ok
in go: verify change of super global in inner g does not change the outside - env ok
in go: verify change of super global in inner g does not change the outside - request ok
in go: verify change of super global in inner g does not change the outside - files ok
in go: verify $GLOBALS always kept updated  - ok

in go: verify change of super global in g does not change the outside - get ok
in go: verify change of super global in g does not change the outside - post ok
in go: verify change of super global in g does not change the outside - cookie ok
in go: verify change of super global in g does not change the outside - server ok
in go: verify change of super global in g does not change the outside - env ok
in go: verify change of super global in g does not change the outside - request ok
in go: verify change of super global in g does not change the outside - files ok
in go: verify $GLOBALS always kept updated  - ok
bar bar black sheep :)

outside of go: verify change of super global in g does not change the outside - get ok
outside of go: verify change of super global in g does not change the outside - post ok
outside of go: verify change of super global in g does not change the outside - cookie ok
outside of go: verify change of super global in g does not change the outside - server ok
outside of go: verify change of super global in g does not change the outside - env ok
outside of go: verify change of super global in g does not change the outside - request ok
outside of go: verify change of super global in g does not change the outside - files ok
in go: verify $GLOBALS always kept updated  - ok
rab rab black sheep :)
