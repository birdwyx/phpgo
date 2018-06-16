<?php
use go\Chan;
use go\Runtime;

$ch = new Chan(1);
go(function() use($ch){
    echo "Alice sends a greeting\n";
    while(true){
        $ok = $ch->tryPush("Greeting from Alice\n");
        if($ok || $ok===NULL) break;
        Runtime::Gosched();
    }
});

$message=$ch->pop();
echo $message;
