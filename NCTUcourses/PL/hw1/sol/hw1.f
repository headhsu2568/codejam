      program McCarthy
      implicit none
      integer :: n,f
      print *, "Enter an integer >=0"
      read *, n
      print *, "f(", n,") =",f(n)
      end program McCarthy

      function f(n)
      implicit none
      integer :: n,f,fs
      fs=1
      do while (fs>0)
         if (n>100) then
             n=n-10
             fs=fs-1
         else
             n=n+11
             fs=fs+1
         end if
      end do
      f=n
      end function f

