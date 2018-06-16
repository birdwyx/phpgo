<?php
use go\Chan;
use go\Scheduler;

$ch = new Chan(1);
go(function() use($ch){
    echo "go 1 try to read\n";
    $data = $ch->pop();
    echo "data read in go 1:"; var_dump($data);
});

go(function() use($ch){
    sleep(1);
    echo "go 2 close channel\n";
    $data = $ch->close();
});

Scheduler::join();
