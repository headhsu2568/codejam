fun palindrome stri= stri=

	let fun reverse str=

		let fun rev 0 y = y 
		|rev x y = rev (x div 10) (y*10+x mod 10)

		in rev str 0 end

	in reverse stri end;
