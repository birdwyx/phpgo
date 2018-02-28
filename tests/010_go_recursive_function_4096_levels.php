<?php
use \go\Scheduler;

function subtc($seq){
    echo "SUB-TC: #$seq\n";
}

class TestClass{
};

function f($level){
	//echo "level:$level\n";
	if($level == 4096)
	return;
	$obj = new TestClass();
	
	$a00=1;$a10=true;$a20=false;$a30=[1,2];$a40=$obj;$a50=1;$a60=2;$a70=3;$a80=4;$a90=5;$aa0=6;$ab0=7;$ac0=8;$ad0=9;$ae0=10;$af0=11;
	$a01=1;$a11=true;$a21=false;$a31=[1,2];$a41=$obj;$a51=1;$a61=2;$a71=3;$a81=4;$a91=5;$aa1=6;$ab1=7;$ac1=8;$ad1=9;$ae1=10;$af1=11;
	$a02=1;$a12=true;$a22=false;$a32=[1,2];$a42=$obj;$a52=1;$a62=2;$a72=3;$a82=4;$a92=5;$aa2=6;$ab2=7;$ac2=8;$ad2=9;$ae2=10;$af2=11;
	$a03=1;$a13=true;$a23=false;$a33=[1,2];$a43=$obj;$a53=1;$a63=2;$a73=3;$a83=4;$a93=5;$aa3=6;$ab3=7;$ac3=8;$ad3=9;$ae3=10;$af3=11;
	$a04=1;$a14=true;$a24=false;$a34=[1,2];$a44=$obj;$a54=1;$a64=2;$a74=3;$a84=4;$a94=5;$aa4=6;$ab4=7;$ac4=8;$ad4=9;$ae4=10;$af4=11;
	$a05=1;$a15=true;$a25=false;$a35=[1,2];$a45=$obj;$a55=1;$a65=2;$a75=3;$a85=4;$a95=5;$aa5=6;$ab5=7;$ac5=8;$ad5=9;$ae5=10;$af5=11;
	$a06=1;$a16=true;$a26=false;$a36=[1,2];$a46=$obj;$a56=1;$a66=2;$a76=3;$a86=4;$a96=5;$aa6=6;$ab6=7;$ac6=8;$ad6=9;$ae6=10;$af6=11;
	$a07=1;$a17=true;$a27=false;$a37=[1,2];$a47=$obj;$a57=1;$a67=2;$a77=3;$a87=4;$a97=5;$aa7=6;$ab7=7;$ac7=8;$ad7=9;$ae7=10;$af7=11;
	$a08=1;$a18=true;$a28=false;$a38=[1,2];$a48=$obj;$a58=1;$a68=2;$a78=3;$a88=4;$a98=5;$aa8=6;$ab8=7;$ac8=8;$ad8=9;$ae8=10;$af8=11;
	$a09=1;$a19=true;$a29=false;$a39=[1,2];$a49=$obj;$a59=1;$a69=2;$a79=3;$a89=4;$a99=5;$aa9=6;$ab9=7;$ac9=8;$ad9=9;$ae9=10;$af9=11;
	$a0a=1;$a1a=true;$a2a=false;$a3a=[1,2];$a4a=$obj;$a5a=1;$a6a=2;$a7a=3;$a8a=4;$a9a=5;$aaa=6;$aba=7;$aca=8;$ada=9;$aea=10;$afa=11;
	$a0b=1;$a1b=true;$a2b=false;$a3b=[1,2];$a4b=$obj;$a5b=1;$a6b=2;$a7b=3;$a8b=4;$a9b=5;$aab=6;$abb=7;$acb=8;$adb=9;$aeb=10;$afb=11;
	$a0c=1;$a1c=true;$a2c=false;$a3c=[1,2];$a4c=$obj;$a5c=1;$a6c=2;$a7c=3;$a8c=4;$a9c=5;$aac=6;$abc=7;$acc=8;$adc=9;$aec=10;$afc=11;
	$a0d=1;$a1d=true;$a2d=false;$a3d=[1,2];$a4d=$obj;$a5d=1;$a6d=2;$a7d=3;$a8d=4;$a9d=5;$aad=6;$abd=7;$acd=8;$add=9;$aed=10;$afd=11;
	$a0e=1;$a1e=true;$a2e=false;$a3e=[1,2];$a4e=$obj;$a5e=1;$a6e=2;$a7e=3;$a8e=4;$a9e=5;$aae=6;$abe=7;$ace=8;$ade=9;$aee=10;$afe=11;
	$a0f=1;$a1f=true;$a2f=false;$a3f=[1,2];$a4f=$obj;$a5f=1;$a6f=2;$a7f=3;$a8f=4;$a9f=5;$aaf=6;$abf=7;$acf=8;$adf=9;$aef=10;$aff=11;
		
	f($level+1);
}

go('f', 0);

Scheduler::RunJoinAll();

echo "the only output\n"

?>
