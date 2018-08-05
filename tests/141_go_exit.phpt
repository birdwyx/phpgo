--TEST--
Go exit test

--FILE--
<?php
use \Go\Scheduler;
use \Go\Runtime;

function subtc($seq){
    echo "SUB-TC: #$seq\n";
}

subtc(1);
$go1_verify_ok = false;
$go2_verify_ok = false;
$go3_verify_ok = false;
$go4_alive     = false;

go( function() use(&$go1_verify_ok){
    $go1_verify_ok = true;
    throw new Exception("an exception");
    $go1_verify_ok = false;
});

go( function() use(&$go2_verify_ok){
    $go2_verify_ok = true;
    exit(1);
    $go2_verify_ok = false;
});

go( function() use(&$go3_verify_ok){
    $go3_verify_ok = true;
    die("go3 dying\n");
    $go3_verify_ok = false;
});

go( function() use(&$go4_alive){
    sleep(1);
    $go4_alive = true;
});

Scheduler::join();

if($go1_verify_ok && $go2_verify_ok && $go3_verify_ok && $go4_alive){
    echo "verify exceptions/exit/die can only terminate the affected go routine - success\n";
}else{
    echo "verify exceptions/exit/die can only terminate the affected go routine - failure\n";
    var_dump($go1_verify_ok,$go2_verify_ok,$go3_verify_ok,$go4_alive);
}

subtc(2); 
go(function() {
    sleep(1);
    echo "this shouldn't show\n";
});

go(function() {
    Runtime::quit("this should show\n");
});

Scheduler::join();

exit(1);
echo "this shouldn't show\n";

?>
--EXPECTREGEX--
SUB-TC: #1.*can only terminate the affected go routine - success[\s]*SUB-TC: #2.*this should show