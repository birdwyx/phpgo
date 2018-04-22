--TEST--
Go closure with 1024 local variables

--FILE--
<?php
use \Go\Scheduler;

function subtc($seq){
    echo "SUB-TC: #$seq\n";
}

go(function(){
		class TestClass{
		};
		
		$obj = new TestClass();
		
		subtc(1);
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

	    $b00=1;$b10=true;$b20=false;$b30=[1,2];$b40=$obj;$b50=1;$b60=2;$b70=3;$b80=4;$b90=5;$ba0=6;$bb0=7;$bc0=8;$bd0=9;$be0=10;$bf0=11;
	    $b01=1;$b11=true;$b21=false;$b31=[1,2];$b41=$obj;$b51=1;$b61=2;$b71=3;$b81=4;$b91=5;$ba1=6;$bb1=7;$bc1=8;$bd1=9;$be1=10;$bf1=11;
	    $b02=1;$b12=true;$b22=false;$b32=[1,2];$b42=$obj;$b52=1;$b62=2;$b72=3;$b82=4;$b92=5;$ba2=6;$bb2=7;$bc2=8;$bd2=9;$be2=10;$bf2=11;
	    $b03=1;$b13=true;$b23=false;$b33=[1,2];$b43=$obj;$b53=1;$b63=2;$b73=3;$b83=4;$b93=5;$ba3=6;$bb3=7;$bc3=8;$bd3=9;$be3=10;$bf3=11;
	    $b04=1;$b14=true;$b24=false;$b34=[1,2];$b44=$obj;$b54=1;$b64=2;$b74=3;$b84=4;$b94=5;$ba4=6;$bb4=7;$bc4=8;$bd4=9;$be4=10;$bf4=11;
	    $b05=1;$b15=true;$b25=false;$b35=[1,2];$b45=$obj;$b55=1;$b65=2;$b75=3;$b85=4;$b95=5;$ba5=6;$bb5=7;$bc5=8;$bd5=9;$be5=10;$bf5=11;
	    $b06=1;$b16=true;$b26=false;$b36=[1,2];$b46=$obj;$b56=1;$b66=2;$b76=3;$b86=4;$b96=5;$ba6=6;$bb6=7;$bc6=8;$bd6=9;$be6=10;$bf6=11;
	    $b07=1;$b17=true;$b27=false;$b37=[1,2];$b47=$obj;$b57=1;$b67=2;$b77=3;$b87=4;$b97=5;$ba7=6;$bb7=7;$bc7=8;$bd7=9;$be7=10;$bf7=11;
	    $b08=1;$b18=true;$b28=false;$b38=[1,2];$b48=$obj;$b58=1;$b68=2;$b78=3;$b88=4;$b98=5;$ba8=6;$bb8=7;$bc8=8;$bd8=9;$be8=10;$bf8=11;
	    $b09=1;$b19=true;$b29=false;$b39=[1,2];$b49=$obj;$b59=1;$b69=2;$b79=3;$b89=4;$b99=5;$ba9=6;$bb9=7;$bc9=8;$bd9=9;$be9=10;$bf9=11;
	    $b0a=1;$b1a=true;$b2a=false;$b3a=[1,2];$b4a=$obj;$b5a=1;$b6a=2;$b7a=3;$b8a=4;$b9a=5;$baa=6;$bba=7;$bca=8;$bda=9;$bea=10;$bfa=11;
	    $b0b=1;$b1b=true;$b2b=false;$b3b=[1,2];$b4b=$obj;$b5b=1;$b6b=2;$b7b=3;$b8b=4;$b9b=5;$bab=6;$bbb=7;$bcb=8;$bdb=9;$beb=10;$bfb=11;
	    $b0c=1;$b1c=true;$b2c=false;$b3c=[1,2];$b4c=$obj;$b5c=1;$b6c=2;$b7c=3;$b8c=4;$b9c=5;$bac=6;$bbc=7;$bcc=8;$bdc=9;$bec=10;$bfc=11;
	    $b0d=1;$b1d=true;$b2d=false;$b3d=[1,2];$b4d=$obj;$b5d=1;$b6d=2;$b7d=3;$b8d=4;$b9d=5;$bad=6;$bbd=7;$bcd=8;$bdd=9;$bed=10;$bfd=11;
	    $b0e=1;$b1e=true;$b2e=false;$b3e=[1,2];$b4e=$obj;$b5e=1;$b6e=2;$b7e=3;$b8e=4;$b9e=5;$bae=6;$bbe=7;$bce=8;$bde=9;$bee=10;$bfe=11;
	    $b0f=1;$b1f=true;$b2f=false;$b3f=[1,2];$b4f=$obj;$b5f=1;$b6f=2;$b7f=3;$b8f=4;$b9f=5;$baf=6;$bbf=7;$bcf=8;$bdf=9;$bef=10;$bff=11;

	    $c00=1;$c10=true;$c20=false;$c30=[1,2];$c40=$obj;$c50=1;$c60=2;$c70=3;$c80=4;$c90=5;$ca0=6;$cb0=7;$cc0=8;$cd0=9;$ce0=10;$cf0=11;
	    $c01=1;$c11=true;$c21=false;$c31=[1,2];$c41=$obj;$c51=1;$c61=2;$c71=3;$c81=4;$c91=5;$ca1=6;$cb1=7;$cc1=8;$cd1=9;$ce1=10;$cf1=11;
	    $c02=1;$c12=true;$c22=false;$c32=[1,2];$c42=$obj;$c52=1;$c62=2;$c72=3;$c82=4;$c92=5;$ca2=6;$cb2=7;$cc2=8;$cd2=9;$ce2=10;$cf2=11;
	    $c03=1;$c13=true;$c23=false;$c33=[1,2];$c43=$obj;$c53=1;$c63=2;$c73=3;$c83=4;$c93=5;$ca3=6;$cb3=7;$cc3=8;$cd3=9;$ce3=10;$cf3=11;
	    $c04=1;$c14=true;$c24=false;$c34=[1,2];$c44=$obj;$c54=1;$c64=2;$c74=3;$c84=4;$c94=5;$ca4=6;$cb4=7;$cc4=8;$cd4=9;$ce4=10;$cf4=11;
	    $c05=1;$c15=true;$c25=false;$c35=[1,2];$c45=$obj;$c55=1;$c65=2;$c75=3;$c85=4;$c95=5;$ca5=6;$cb5=7;$cc5=8;$cd5=9;$ce5=10;$cf5=11;
	    $c06=1;$c16=true;$c26=false;$c36=[1,2];$c46=$obj;$c56=1;$c66=2;$c76=3;$c86=4;$c96=5;$ca6=6;$cb6=7;$cc6=8;$cd6=9;$ce6=10;$cf6=11;
	    $c07=1;$c17=true;$c27=false;$c37=[1,2];$c47=$obj;$c57=1;$c67=2;$c77=3;$c87=4;$c97=5;$ca7=6;$cb7=7;$cc7=8;$cd7=9;$ce7=10;$cf7=11;
	    $c08=1;$c18=true;$c28=false;$c38=[1,2];$c48=$obj;$c58=1;$c68=2;$c78=3;$c88=4;$c98=5;$ca8=6;$cb8=7;$cc8=8;$cd8=9;$ce8=10;$cf8=11;
	    $c09=1;$c19=true;$c29=false;$c39=[1,2];$c49=$obj;$c59=1;$c69=2;$c79=3;$c89=4;$c99=5;$ca9=6;$cb9=7;$cc9=8;$cd9=9;$ce9=10;$cf9=11;
	    $c0a=1;$c1a=true;$c2a=false;$c3a=[1,2];$c4a=$obj;$c5a=1;$c6a=2;$c7a=3;$c8a=4;$c9a=5;$caa=6;$cba=7;$cca=8;$cda=9;$cea=10;$cfa=11;
	    $c0b=1;$c1b=true;$c2b=false;$c3b=[1,2];$c4b=$obj;$c5b=1;$c6b=2;$c7b=3;$c8b=4;$c9b=5;$cab=6;$cbb=7;$ccb=8;$cdb=9;$ceb=10;$cfb=11;
	    $c0c=1;$c1c=true;$c2c=false;$c3c=[1,2];$c4c=$obj;$c5c=1;$c6c=2;$c7c=3;$c8c=4;$c9c=5;$cac=6;$cbc=7;$ccc=8;$cdc=9;$cec=10;$cfc=11;
	    $c0d=1;$c1d=true;$c2d=false;$c3d=[1,2];$c4d=$obj;$c5d=1;$c6d=2;$c7d=3;$c8d=4;$c9d=5;$cad=6;$cbd=7;$ccd=8;$cdd=9;$ced=10;$cfd=11;
	    $c0e=1;$c1e=true;$c2e=false;$c3e=[1,2];$c4e=$obj;$c5e=1;$c6e=2;$c7e=3;$c8e=4;$c9e=5;$cae=6;$cbe=7;$cce=8;$cde=9;$cee=10;$cfe=11;
	    $c0f=1;$c1f=true;$c2f=false;$c3f=[1,2];$c4f=$obj;$c5f=1;$c6f=2;$c7f=3;$c8f=4;$c9f=5;$caf=6;$cbf=7;$ccf=8;$cdf=9;$cef=10;$cff=11;

	    $d00=1;$d10=true;$d20=false;$d30=[1,2];$d40=$obj;$d50=1;$d60=2;$d70=3;$d80=4;$d90=5;$da0=6;$db0=7;$dc0=8;$dd0=9;$de0=10;$df0=11;
	    $d01=1;$d11=true;$d21=false;$d31=[1,2];$d41=$obj;$d51=1;$d61=2;$d71=3;$d81=4;$d91=5;$da1=6;$db1=7;$dc1=8;$dd1=9;$de1=10;$df1=11;
	    $d02=1;$d12=true;$d22=false;$d32=[1,2];$d42=$obj;$d52=1;$d62=2;$d72=3;$d82=4;$d92=5;$da2=6;$db2=7;$dc2=8;$dd2=9;$de2=10;$df2=11;
	    $d03=1;$d13=true;$d23=false;$d33=[1,2];$d43=$obj;$d53=1;$d63=2;$d73=3;$d83=4;$d93=5;$da3=6;$db3=7;$dc3=8;$dd3=9;$de3=10;$df3=11;
	    $d04=1;$d14=true;$d24=false;$d34=[1,2];$d44=$obj;$d54=1;$d64=2;$d74=3;$d84=4;$d94=5;$da4=6;$db4=7;$dc4=8;$dd4=9;$de4=10;$df4=11;
	    $d05=1;$d15=true;$d25=false;$d35=[1,2];$d45=$obj;$d55=1;$d65=2;$d75=3;$d85=4;$d95=5;$da5=6;$db5=7;$dc5=8;$dd5=9;$de5=10;$df5=11;
	    $d06=1;$d16=true;$d26=false;$d36=[1,2];$d46=$obj;$d56=1;$d66=2;$d76=3;$d86=4;$d96=5;$da6=6;$db6=7;$dc6=8;$dd6=9;$de6=10;$df6=11;
	    $d07=1;$d17=true;$d27=false;$d37=[1,2];$d47=$obj;$d57=1;$d67=2;$d77=3;$d87=4;$d97=5;$da7=6;$db7=7;$dc7=8;$dd7=9;$de7=10;$df7=11;
	    $d08=1;$d18=true;$d28=false;$d38=[1,2];$d48=$obj;$d58=1;$d68=2;$d78=3;$d88=4;$d98=5;$da8=6;$db8=7;$dc8=8;$dd8=9;$de8=10;$df8=11;
	    $d09=1;$d19=true;$d29=false;$d39=[1,2];$d49=$obj;$d59=1;$d69=2;$d79=3;$d89=4;$d99=5;$da9=6;$db9=7;$dc9=8;$dd9=9;$de9=10;$df9=11;
	    $d0a=1;$d1a=true;$d2a=false;$d3a=[1,2];$d4a=$obj;$d5a=1;$d6a=2;$d7a=3;$d8a=4;$d9a=5;$daa=6;$dba=7;$dca=8;$dda=9;$dea=10;$dfa=11;
	    $d0b=1;$d1b=true;$d2b=false;$d3b=[1,2];$d4b=$obj;$d5b=1;$d6b=2;$d7b=3;$d8b=4;$d9b=5;$dab=6;$dbb=7;$dcb=8;$ddb=9;$deb=10;$dfb=11;
	    $d0c=1;$d1c=true;$d2c=false;$d3c=[1,2];$d4c=$obj;$d5c=1;$d6c=2;$d7c=3;$d8c=4;$d9c=5;$dac=6;$dbc=7;$dcc=8;$ddc=9;$dec=10;$dfc=11;
	    $d0d=1;$d1d=true;$d2d=false;$d3d=[1,2];$d4d=$obj;$d5d=1;$d6d=2;$d7d=3;$d8d=4;$d9d=5;$dad=6;$dbd=7;$dcd=8;$ddd=9;$ded=10;$dfd=11;
	    $d0e=1;$d1e=true;$d2e=false;$d3e=[1,2];$d4e=$obj;$d5e=1;$d6e=2;$d7e=3;$d8e=4;$d9e=5;$dae=6;$dbe=7;$dce=8;$dde=9;$dee=10;$dfe=11;
	    $d0f=1;$d1f=true;$d2f=false;$d3f=[1,2];$d4f=$obj;$d5f=1;$d6f=2;$d7f=3;$d8f=4;$d9f=5;$daf=6;$dbf=7;$dcf=8;$ddf=9;$def=10;$dff=11;
//<--1024 vars
	
		var_dump($a35);
		var_dump($a4f);
		var_dump($a00);
		var_dump($a20);
		var_dump($a1b);
		echo $aff . PHP_EOL;
		
		var_dump($b35);
		var_dump($b4f);
		var_dump($b00);
		var_dump($b20);
		var_dump($b1b);
		echo $bff . PHP_EOL;
		
		var_dump($c35);
		var_dump($c4f);
		var_dump($c00);
		var_dump($c20);
		var_dump($c1b);
		echo $cff . PHP_EOL;
		
		var_dump($d35);
		var_dump($d4f);
		var_dump($d00);
		var_dump($d20);
		var_dump($d1b);
		echo $dff . PHP_EOL;
	}
);

Scheduler::join();

?>
--EXPECT--
SUB-TC: #1
array(2) {
  [0]=>
  int(1)
  [1]=>
  int(2)
}
object(TestClass)#2 (0) {
}
int(1)
bool(false)
bool(true)
11
array(2) {
  [0]=>
  int(1)
  [1]=>
  int(2)
}
object(TestClass)#2 (0) {
}
int(1)
bool(false)
bool(true)
11
array(2) {
  [0]=>
  int(1)
  [1]=>
  int(2)
}
object(TestClass)#2 (0) {
}
int(1)
bool(false)
bool(true)
11
array(2) {
  [0]=>
  int(1)
  [1]=>
  int(2)
}
object(TestClass)#2 (0) {
}
int(1)
bool(false)
bool(true)
11

