#include <stdio.h>
#include <stdint.h>
#include "lab4.h"

extern unsigned short globalCount;

// Implement a new pseudo random number generator
unsigned long randomInteger(int low, int high)
{
  volatile double randNum = 1.0;
  volatile int multiplier = 2743;
  volatile int addOn = 5923;
  volatile long seed;
  volatile double max = 65535 + 1.0;
  volatile unsigned long retVal = 0;
  
  seed = globalCount;

  if (low > high){
    retVal = randomInteger(high, low);
  }
  else
  {
    seed = seed * multiplier + addOn;
    randNum = seed;

    if (randNum < 0)
    {
      randNum = randNum + max;
    }
    randNum = randNum / max;
    retVal =  ((int)((high-low+1)*randNum))+low;
  }
  return retVal;
}