fun msort []=[]

	| msort (target as _::[])=target

	| msort target=

		let val (leftside,rightside)=

			let fun cut (x::xs,l,r)=cut (xs,r,x::l)

				| cut ([],l,r)=(l,r)

			in cut(target,[],[])
 end

		val sl=msort leftside

		val sr=msort rightside

		val m=

			let fun merge(out,x as (l::ls),y as (r::rs))=

				if l<r then merge(l::out,ls,y)

					else merge(r::out,x,rs)

				| merge(out,l::ls,[])=merge(l::out,ls,[])

				| merge(out,[],r::rs)=merge(r::out,[],rs)

				| merge(out,[],[])=out

			in merge([],sl,sr)
 end

		in rev m
 end;
