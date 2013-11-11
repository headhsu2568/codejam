program McCarthy
implicit none
integer :: n,f
print *, "Enter an integer >=0"
read *, n
print *, "f(", n,") =",f(n)
end program McCarthy

recursive function f(n) result (r)
implicit none
integer :: n,r
if (n>100) then
   r=n-10
else
   r=f(f(n+11))
end if
end function f

