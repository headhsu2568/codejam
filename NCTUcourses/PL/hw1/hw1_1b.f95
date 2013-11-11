program hw1_2
        implicit none
        integer :: n,fact
        print *,"Enter an integer :"
        read *,n
        print *,"f(",n,")=",fact(n)
end![program[hw1_2]]
recursive function fact(n)result(r)
        implicit none
        integer :: n,r
        if(n>100)then
                r=n-10
        else
                n=fact(n+11)
                r=fact(n)
        end if
end![function[fact]]