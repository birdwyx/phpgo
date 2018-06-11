<?php
use go\Chan;
use go\Scheduler;

/*
this tc will trigger a system error:
terminate called after throwing an instance of 'std::system_error'
  what():  block object was waiting when destructor
  
The resean is that the main program is ended after it has popped a data
while the go is still waiting on $ch->pop()

The main progrom got the popped data by creating an (invisible) child go routine and
do channel pop in that child go routine

the usleep() must be in place otherwise this error won't happen: because if the usleep is not
in place, the main program's child go routining won't have chance to run, so it's the go routine 
created by go() that get data pushed and then poped, and so the main program won't get data
and won't end, and so no error reported
*/

$ch = new Chan(1);
go(function($ch){
	usleep(1);
	$ch->push(1);
	var_dump($ch->pop());
	echo "return from go\n";
},[$ch]);

$v = $ch->pop(); // wait the go routine to finish
