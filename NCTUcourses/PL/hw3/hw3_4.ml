fun n2c 0 f x=x

	| n2c n f x=f (n2c (n-1) f x);

fun c2n c=c (fn x=>x+1)0;

infix 7 **;

infix 6 ++;

fun (m ++ n) f x=m f(n f x);

fun (m ** n) f x=n (m f)x;
