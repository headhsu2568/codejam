        program PLhw1
                implicit none
                integer :: n,fact
                print *,"Enter an integer :"
                read *,n
                print *,"f(",n,")=",fact(n)
        end![program[PLhw1]]
        function fact(n)
                implicit none
                integer :: n,fact,level
		level=0
                fact=n
                if(fact<=100)then
                        do while(fact<=100)
				do while(fact<=100)
					level=level+1
                                	fact=fact+11
				end do
				if(fact>100)then
					do while(level>0)
						level=level-1
						fact=fact-10
					end do
				end if
                        end do
			fact=fact-10
		else
			fact=fact-10
		end if
        end![function[fact]]
