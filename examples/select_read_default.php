<?php
use \Go\Chan;
use \Go\Scheduler;

$ch = new Chan(["capacity"=>1]);

go(function() use($ch){
    $read = false; $df = false; $v=0;
    $ch->push(1);
    select(
        [
            'case', $ch, "->", &$v, function($value) use(&$read){
                //$value (==$v) should be 1 as read from $ch
                if($value===1)
                    $read = true;
            }
        ],
        [
            'default', function() use(&$df){
                $df = true;
             }
		]
    );
    
    assert(
        $read===true &&  //should hit the channel-read branch
        $df===false &&   //should not hit the default branch
        $v === 1         //$v should be 1 as the popped data from channel is assigned to its reference
    );
    echo "success\n";
});

Scheduler::join();
