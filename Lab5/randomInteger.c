#include <stdio.h>
#include <stdint.h>
#include "lab4.h"

// Implement a new pseudo random number generator
short randomInteger(int low, int high)
{
  double randNum = 1.0;
  int multiplier = 2743;
  int addOn = 5923;
  double max = 65535 + 1.0;

  uint16_t retVal = 0;

  if (low > high)
    retVal = randomInteger(high, low);
  else
  {
    seed = seed*multiplier + addOn;
    randNum = seed;

    if (randNum < 0)
    {
      randNum = randNum + max;
    }

    randNum = randNum/max;

    retVal =  ((int)((high-low+1)*randNum))+low;
  }

  return retVal;
}