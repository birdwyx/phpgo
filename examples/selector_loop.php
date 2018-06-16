<?php
use \Go\Chan;
use \Go\Time;
use \Go\Scheduler;

$ch1 = new Chan(["capacity"=>1]);
$ch2 = new Chan(["capacity"=>1]);
$exit = new Chan(0);

go(function() use($ch1, $ch2, $exit){
    select(
        [
            'case', $ch1, function($value){
                echo "go 0: receive from ch1: " . $value . PHP_EOL;
            }
        ],
        [
            'case', $ch2, function($value){
                echo "go 0: receive from ch2: " . $value . PHP_EOL;
            }
        ]
    )->loop($exit);
});

go(function() use($ch1, $exit){
    $i = 0;
    while( $exit->tryPop() !== NULL ){ //===NULL: channel closed
        echo "go 1: push $i\n";
        $ch1->push($i++);
    }
});

go(function() use($ch2, $exit){
    $i = 0;
    while( $exit->tryPop() !== NULL ){
        echo "go 2: push $i\n";
        $ch2->push($i++);
    }
});

go(function() use($exit){
    echo "main: now sleep 5ms\n";
    Time::sleep(5*1000*1000);
    echo "main: now close the exit channel\n";
    $exit->close();
});

Scheduler::join();
