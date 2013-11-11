datatype 'a stack = Empty | Push of 'a*'a stack
fun makestk []=Empty
        | makestk (x::xs)=
                let fun pushin x Empty=Push(x,Empty)
                        | pushin x (Push(y,stk))=Push(x,Push(y,stk))
                in pushin x (makestk xs) end;
fun pop (Push(value,next))=(value,next);
fun result Empty =0
        | result target=
                let fun count(Empty,temp)=hd temp
                | count(tar,temp)=
                        let
                                val two=pop tar
                                val tmp=if ord (#1two)>47 then ord (#1two)-48::temp
                                        else if ord (#1two)=43 then hd (tl temp)+hd temp::tl (tl temp)
                                        else if ord (#1two)=45 then hd (tl temp)-hd temp::tl (tl temp)
                                        else if ord (#1two)=42 then hd (tl temp)*hd temp::tl (tl temp)
                                        else if ord (#1two)=47 then hd (tl temp) div (hd temp)::tl (tl temp)
                                        else hd (tl temp) mod (hd temp)::tl (tl temp)
                        in if(ord (#1two)>47) then count(#2two,tmp)
                                else count(#2two,tmp) end
                in count(target,[]) end;
val eval =result o makestk o explode;