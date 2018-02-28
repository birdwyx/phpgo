--TEST--
Go channel create

--FILE--
<?php
use \go\Chan;
use \go\Scheduler;

function subtc($seq){
    echo "SUB-TC: #$seq\n";
}

subtc(1);
$ch = new Chan();
echo "name: {$ch->name}; capacity {$ch->capacity}; copy ".var_export($ch->copy, true)."\n";

subtc(2);
$ch = new Chan(0);
echo "name: {$ch->name}; capacity {$ch->capacity}; copy ".var_export($ch->copy, true)."\n";

subtc(3);
$ch = new Chan(10000);
echo "name: {$ch->name}; capacity {$ch->capacity}; copy ".var_export($ch->copy, true)."\n";

subtc(4);
$ch = new Chan(['name'=>"chan", "capacity"=>100, "copy"=>false]);
echo "name: {$ch->name}; capacity {$ch->capacity}; copy ".var_export($ch->copy, true)."\n";

subtc(5);
$ch = new Chan(['name'=>"chan", "capacity"=>0]);
echo "name: {$ch->name}; capacity {$ch->capacity}; copy ".var_export($ch->copy, true)."\n";

subtc(6);
$ch = new Chan(['name'=>"chan", "copy"=>false]);
echo "name: {$ch->name}; capacity {$ch->capacity}; copy ".var_export($ch->copy, true)."\n";

subtc(7);
$ch = new Chan(["capacity"=>100, "copy"=>true]);
echo "name: {$ch->name}; capacity {$ch->capacity}; copy ".var_export($ch->copy, true)."\n";

subtc(8);
$ch = new Chan(['name'=>"chan"]);
echo "name: {$ch->name}; capacity {$ch->capacity}; copy ".var_export($ch->copy, true)."\n";

subtc(9);
$ch = new Chan(['name'=>""]);
echo "name: {$ch->name}; capacity {$ch->capacity}; copy ".var_export($ch->copy, true)."\n";

subtc(10);
$ch = new Chan(['name'=>"123"]);
echo "name: {$ch->name}; capacity {$ch->capacity}; copy ".var_export($ch->copy, true)."\n";

subtc(11);
$ch = new Chan(["capacity"=>0]);
echo "name: {$ch->name}; capacity {$ch->capacity}; copy ".var_export($ch->copy, true)."\n";

subtc(12);
$ch = new Chan(["capacity"=>100]);
echo "name: {$ch->name}; capacity {$ch->capacity}; copy ".var_export($ch->copy, true)."\n";

subtc(13);
$ch = new Chan(["copy"=>false]);
echo "name: {$ch->name}; capacity {$ch->capacity}; copy ".var_export($ch->copy, true)."\n";

subtc(14);
$ch = new Chan(["copy"=>true]);
echo "name: {$ch->name}; capacity {$ch->capacity}; copy ".var_export($ch->copy, true)."\n";

subtc(15);
$ch = new Chan([]);
echo "name: {$ch->name}; capacity {$ch->capacity}; copy ".var_export($ch->copy, true)."\n";

subtc(16);
$ch = new Chan(["abc"=>"d"]);
echo "name: {$ch->name}; capacity {$ch->capacity}; copy ".var_export($ch->copy, true)."\n";

subtc(17);
$ch = new Chan(["abc"=>"d", "name"=>"chan"]);
echo "name: {$ch->name}; capacity {$ch->capacity}; copy ".var_export($ch->copy, true)."\n";
?>
--EXPECT--
SUB-TC: #1
name: ; capacity 0; copy false
SUB-TC: #2
name: ; capacity 0; copy false
SUB-TC: #3
name: ; capacity 10000; copy false
SUB-TC: #4
name: chan; capacity 100; copy false
SUB-TC: #5
name: chan; capacity 0; copy false
SUB-TC: #6
name: chan; capacity 0; copy false
SUB-TC: #7
name: ; capacity 100; copy true
SUB-TC: #8
name: chan; capacity 0; copy false
SUB-TC: #9
name: ; capacity 0; copy false
SUB-TC: #10
name: 123; capacity 0; copy false
SUB-TC: #11
name: ; capacity 0; copy false
SUB-TC: #12
name: ; capacity 100; copy false
SUB-TC: #13
name: ; capacity 0; copy false
SUB-TC: #14
name: ; capacity 0; copy true
SUB-TC: #15
name: ; capacity 0; copy false
SUB-TC: #16
name: ; capacity 0; copy false
SUB-TC: #17
name: chan; capacity 0; copy false