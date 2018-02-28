--TEST--
Go channel create error Chan(888888888888888888888888888888888888888)

--FILE--
<?php
use \go\Chan;
use \go\Scheduler;

function subtc($seq){
    echo "SUB-TC: #$seq\n";
}

subtc(1);
$ch = new Chan(888888888888888888888888888888888888888);

?>
--EXPECTREGEX--
.*parameter 1 must be long or array in.*