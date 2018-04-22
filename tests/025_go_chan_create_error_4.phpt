--TEST--
Go channel create error Chan(["capacity"=>-1])

--FILE--
<?php
use \Go\Chan;
use \Go\Scheduler;

function subtc($seq){
    echo "SUB-TC: #$seq\n";
}

subtc(1);
$ch = new Chan(["capacity"=>-1]);

?>
--EXPECTREGEX--
.*the capacity must be greater than or equal to 0.*