--TEST--
Go error handler test

--FILE--
<?php
use \Go\Scheduler;

echo "main script set error handler\n";
set_error_handler(function ($errno, $errstr, $errfile, $errline) {
    echo "error handler in main: $errstr, line $errline\n";
});

go(function () {
    usleep(1);
    echo "go 1 trggers error\n";
    trigger_error("this is error from inside go 1");
    usleep(1);
    echo "go 1 set error handler\n";
    set_error_handler(function ($errno, $errstr, $errfile, $errline, $errcontext) {
        echo "error handler in go 1: $errstr, line $errline\n";
    });
    sleep(1);
});

go(function () {
    usleep(1);
    echo "go 2 set error handler\n";
    set_error_handler(function ($errno, $errstr, $errfile, $errline, $errcontext) {
        echo "error handler in go 2: $errstr, line $errline\n";
    });
    usleep(1);
    echo "go 2 trggers error\n";
    trigger_error("this is error from inside go 2");
    usleep(1);
});

go(function () {
    usleep(1);
    echo "go 3 trggers error\n";
    trigger_error("this is error from inside go 3");
    usleep(1);
});

echo "main script trggers error\n";
trigger_error("this is error from outside go");

Scheduler::join();

echo "main script returns\n";
?>
--EXPECT--
main script set error handler
main script trggers error
error handler in main: this is error from outside go, line 41
go 1 trggers error
error handler in main: this is error from inside go 1, line 12
go 2 set error handler
go 3 trggers error
error handler in go 2: this is error from inside go 3, line 36
go 1 set error handler
go 2 trggers error
error handler in go 1: this is error from inside go 2, line 29
main script returns
