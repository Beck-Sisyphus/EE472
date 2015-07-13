#include <stdio.h>
#include "kernel3.h"

void get (void* aNumber)				//  perform input operation
{
	printf ("Enter a number: 0..9 ");
	*(int*) aNumber  = getchar();
	getchar();							//  discard cr
	*(int*) aNumber -= '0';  			//  convert to decimal from ascii
	return;
}

void increment (void* aNumber)			//  perform computation
{
	int* aPtr = (int*) aNumber;
	(*aPtr)++;
	return;
}

void display (void* aNumber)			// perform output operation
{
	printf ("The result is: %d\n", *(int*)aNumber);
	return;
}