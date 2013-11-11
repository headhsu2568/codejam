sub fib

{
  
   if ($h{"fib($_[0])"}==undef) { 
      if ($_[0]<=1) { $h{"fib($_[0])"} = $_[0]; }
      else { $h{"fib($_[0])"} = fib($_[0]-1)+fib($_[0]-2); }
   } 
   return $h{"fib($_[0])"};
}

print "Enter an integer>=0: ";
$n=<stdin>*1;

print "The $n","th Fibonacci number is ",fib($n),".\n";
print "Below is the hash table created during the computation.\n";
while (($key,$value)=each %h) { 
   
   print "$key => $value\n"; 

}

