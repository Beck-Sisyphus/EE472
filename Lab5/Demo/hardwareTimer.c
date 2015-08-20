//*****************************************************************************
//
// Description: Configure Timer0B as a 16-bit periodic counter with an interrupt
// every 100 ms.
//
//*****************************************************************************

#include <stdlib.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"
#include "lab2.h"

void
hardwareTimer(void)
{
  //
  // The Timer0 peripheral must be enabled for use.
  //
  SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);

  //
  // Configure Timer0 as a 16-bit periodic timer.
  //
  TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);

  //
  // Set the Timer0 load value to 100ms.
  //
  TimerLoadSet(TIMER0_BASE, TIMER_A, SysCtlClockGet() / 10);

  //
  // Enable processor interrupts.
  //
  IntMasterEnable();
    
  //
  // Enable the Timer0 interrupt on the processor (NVIC).
  //
  IntEnable(INT_TIMER0A);

  //
  // Configure the Timer0 interrupt for timer timeout.
  //
  TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

  //
  // Enable Timer0.
  //
  TimerEnable(TIMER0_BASE, TIMER_A);
}

