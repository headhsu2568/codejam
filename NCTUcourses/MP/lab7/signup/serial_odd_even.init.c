#include <stdio.h>
#include <stdlib.h>

//#define DEBUG
const int RMAX = 1000000000;

void Usage (char *prog_name);
void Get_args (int argc, char *argv[], int *n_p, char *g_i_p);
void Generate_list (int a[], int n);
void Print_list (int a[], int n, char *title);
void Read_list (int a[], int n);
void Odd_even_sort (int a[], int n);
void Odd_even_iter (int a[], int n, int phase);
void Swap (int *x_p, int *y_p);

/*-----------------------------------------------------------------*/
/*
   int
   main (int argc, char *argv[])
   {
   int n=16;
   int a[] = {612, 765, 123, 1, 839, 146, 243, 32, 989, 8, 177, 94, 26,83, 15, 4};

   Print_list (a, n, "Before sort");

   Odd_even_sort (a, n);

   Print_list (a, n, "After sort");

   return 0;
   }
   */

/*-----------------------------------------------------------------
 * Function:  Print_list
 * Purpose:   Print the elements in the list
 * In args:   a, n
 */
void Print_list (int a[], int n, char *title)
{
    int i;

    printf ("%s:\n", title);
    for (i = 0; i < n; i++)
        printf ("0x%08x\n", a[i]);
    printf ("\n");
}				/* Print_list */


/*-----------------------------------------------------------------
 * Function:     Odd_even_sort
 * Purpose:      Sort list using odd-even transposition sort
 * In args:      n
 * In/out args:  a
 */
void Odd_even_sort (int a[], int n)
{
    int phase;
#  ifdef DEBUG
    char title[100];
#  endif

    for (phase = 0; phase < n; phase++)
    {
        Odd_even_iter (a, n, phase);
#     ifdef DEBUG
        sprintf (title, "After phase %d", phase);
        Print_list (a, n, title);
#     endif
    }
}				/* Odd_even_sort */

/*-----------------------------------------------------------------
 * Function:    Odd_even_iter
 * Purpose:     Execute one iteration of odd-even transposition sort
 * In args:     n, phase
 * In/out args: a
 */

void Odd_even_iter (int a[], int n, int phase)
{
    int i, left, right;

    if (phase % 2 == 0)
    {
        for (i = 1; i < n; i += 2)
        {
            left = i - 1;
            if (a[left] > a[i])
                Swap (&a[left], &a[i]);
        }
    }
    else
    {
        for (i = 1; i < n - 1; i += 2)
        {
            right = i + 1;
            if (a[i] > a[right])
                Swap (&a[i], &a[right]);
        }
    }
}				/* Odd_even_iter */

/*-----------------------------------------------------------------
 * Function:     Swap
 * Purpose:      Swap contents of x_p and y_p
 * In/out args:  x_p, y_p
 */

void Swap (int *x_p, int *y_p)
{
    int temp = *x_p;
    *x_p = *y_p;
    *y_p = temp;
}				/* Swap */
