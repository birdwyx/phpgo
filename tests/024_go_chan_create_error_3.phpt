--TEST--
Go channel create error Chan(["name"=>1])

--FILE--
<?php
use \go\Chan;
use \go\Scheduler;

function subtc($seq){
    echo "SUB-TC: #$seq\n";
}

subtc(1);
$ch = new Chan(["name"=>1]);

?>
--EXPECTREGEX--
.*option \"name\" must be string in.*