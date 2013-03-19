#include <stdio.h> 

main()
{
   double f=123.456;

printf ("\nf = 123.456\n\n");
printf ("%%f: %f\n", f);
printf ("%%20f:    %20f\n", f);
printf ("%%020f:   %020f\n", f);
printf ("%%020.2f: %020.2f\n", f);
printf ("%%20.2f:  %20.2f\n", f);
printf ("%%.2f: %.2f\n", f);
printf ("%%10.2e: %10.2e\n", f);
printf ("%%.2g: %.2g\n", f);
printf ("%%.4g: %.4g\n", f);
printf ("%%+f: %+f\n", f);
printf ("%%-20f: %-20f", f);
printf ("%%020f: %020f", f); 
printf ("%%*.*f: %*.*e\n", 20, 2, f);
}
	
