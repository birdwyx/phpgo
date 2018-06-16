<?php
use \Go\Chan;
use \Go\Scheduler;
use \Go\Time;

function write_data($ch, $data){
    $data_written = new Chan(0);
    
    // try to write data to the channel, if the channel is not available to write, wait a 50ms
    // loop until the data is successfully written
    select(
        [
            'case', $ch, "<-", $data, function($value) use($data_written){
                echo "data written:";
                var_dump($value);
                $data_written->close();
            }
        ],
        [
            'default', function(){
                echo "channel full, wait 50ms for consumer to consume\n";
                Time::sleep(50*1000*1000);
            }
        ]
    )->loop($data_written);
}

function producer($ch, $close){
    select(
        [
            'default', function() use($ch){
                $data = rand();
                write_data($ch, $data);
            }
        ]
    )->loop($close);
}

function consumer($ch, $close){
    select(
        [
            'case', $ch, function($value) {
                echo "data consumed:";
                var_dump($value);
                
                // wait a 10ms: simulating a slow consumer:
                // just to see the output from producer:
                // "channel full, wait 50ms for consumer to consume"
                Time::sleep(10*1000*1000);
            }
        ]
    )->loop($close);
}

// main routine
go( function(){
    $ch = new Chan(["capacity"=>20]); 
    $close = new Chan(0);
    go('producer', [$ch, $close]);
    go('consumer', [$ch, $close]);
    
    // tell producer and consumer to exit after 10 seconds
    // note: Time::sleep follows go convension: time unit in nano-second 
    // please note the difference with php sleep() witch has time unit in seconds
    Time::sleep(10* 1000*1000*1000); 
    $close->close();
});

Scheduler::join();