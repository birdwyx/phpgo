--TEST--
Go channel create error Chan(-1)

--FILE--
<?php
use \go\Chan;
use \go\Scheduler;

function subtc($seq){
    echo "SUB-TC: #$seq\n";
}

subtc(1);
$ch = new Chan(-1);

?>
--EXPECTREGEX--
.*the capacity must be greater than or equal to 0.*