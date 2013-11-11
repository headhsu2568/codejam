#!/usr/bin/perl-w
sub FIB;
sub FIB{
	my $num=0+$n;
	if($n>2){
		$n--;
		FIB $_[0]-1,$_[1]-1;
	}
	my $str="fib($num)";
	$fib{$str}=$fib{"fib($_[0])"}+$fib{"fib($_[1])"};
	return;
}
print "Enter an integer >=0 :";
$n=<stdin>;
$fib{"fib(0)"}=0;
if($n>0){
        $fib{"fib(1)"}=1;
        if($n>1){
                FIB $n-1,$n-2;
        }
}
foreach(keys %fib){
        print "$_=> $fib{$_}\n";
}