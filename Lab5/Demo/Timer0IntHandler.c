//*****************************************************************************
//
// The interrupt handler for the Timer0B interrupt.
// Provided by periodic_16bit.c in StellarisWare examples folder
//
// Parameter: void
// Return: void
//*****************************************************************************

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include "inc/hw_gpio.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_timer.h"
#include "inc/hw_types.h"
#include "driverlib/adc.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/debug.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"
#include "drivers/rit128x96x4.h"
#include "driverlib/pwm.h"
#include "driverlib/uart.h"
#include "lab2.h"


void
Timer0IntHandler(void)
{
  //
  // Clear the timer interrupt flag.
  //
  TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

  //
  // Update the periodic interrupt counter.
  //
  cycleDone = TRUE;
}

