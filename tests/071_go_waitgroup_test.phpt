--TEST--
Go Waitgroup test

--FILE--
<?php
use \go\Waitgroup;
use \go\Scheduler;

function subtc($seq){
    echo "SUB-TC: #$seq\n";
}

subtc(1);

$wg = new Waitgroup();
assert(!empty($wg));

go(function() use($wg){
	echo "go 0: wait\n";
	$wg->Wait();
	echo "go 0: wait done\n";
});

echo "add 10 to wg\n";
$wg->Add(10);

for($i=0;$i<10;$i++){
	go(function() use($wg, $i){
		echo "go $i: done\n";
		$wg->Done();
	});
}


Scheduler::RunJoinAll();

?>
--EXPECT--
SUB-TC: #1
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

