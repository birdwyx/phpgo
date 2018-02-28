--TEST--
Go channel create error Chan(["copy"=>"true"])

--FILE--
<?php
use \go\Chan;
use \go\Scheduler;

function subtc($seq){
    echo "SUB-TC: #$seq\n";
}

subtc(1);
$ch = new Chan(["copy"=>"true"]);

?>
--EXPECTREGEX--
.*option \"copy\" must be bool in.*