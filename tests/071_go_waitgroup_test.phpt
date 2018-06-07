--TEST--
Go Waitgroup test

--FILE--
<?php
use \Go\Waitgroup;
use \Go\Scheduler;

function subtc($seq, $title){
    echo "SUB-TC: #$seq - $title\n";
}

subtc(1, "verify wg wait in a go routine");
$wg = new Waitgroup();
assert(!empty($wg));

go(function() use($wg){
	echo "go 0: wait\n";
	$wg->wait();
	echo "go 0: wait done\n";
});

echo "add 10 to wg\n";
$wg->add(10);

for($i=0;$i<10;$i++){
	go(function() use($wg, $i){
		echo "go $i: done\n";
		$wg->done();
	});
}
Scheduler::join();


subtc(2,  "verify wg wait outside a go routine");

$wg = new Waitgroup();
assert(!empty($wg));

echo "add 10 to wg\n";
$wg->add(10);

for($i=0;$i<10;$i++){
	go(function() use($wg, $i){
		echo "go $i: done\n";
		$wg->done();
	});
}
echo "wait outside go\n";
$wg->wait();
echo "wait outside go: done\n"
//Scheduler::join();

?>
--EXPECT--
SUB-TC: #1 - verify wg wait in a go routine
add 10 to wg
go 0: wait
go 0: done
go 1: done
go 2: done
go 3: done
go 4: done
go 5: done
go 6: done
go 7: done
go 8: done
go 9: done
go 0: wait done
SUB-TC: #2 - verify wg wait outside a go routine
add 10 to wg
wait outside go
go 0: done
go 1: done
go 2: done
go 3: done
go 4: done
go 5: done
go 6: done
go 7: done
go 8: done
go 9: done
wait outside go: done
