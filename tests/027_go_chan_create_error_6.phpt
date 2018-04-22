--TEST--
Go channel create error Chan(["capacity"=>"100"])

--FILE--
<?php
use \Go\Chan;
use \Go\Scheduler;

function subtc($seq){
    echo "SUB-TC: #$seq\n";
}

subtc(1);
$ch = new Chan(["capacity"=>"100"]);

?>
--EXPECTREGEX--
.*option \"capacity\" must be long.*