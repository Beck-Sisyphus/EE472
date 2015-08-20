#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "lab4.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/lm3s8962.h"
#include "driverlib/adc.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/pwm.h"
#include "driverlib/sysctl.h"
#include "lcd_message.h" // OLED
#include "rit128x96x4.h" // OLED

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

void hardwareTimer(void)
{
  while{
    // The Timer0 peripheral must be enabled for use.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    
    // Configure Timer0 as a 16-bit periodic timer.
    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
    
    // Set the Timer0 load value to 100ms.
    TimerLoadSet(TIMER0_BASE, TIMER_A, SysCtlClockGet() / 10);
    
    // Enable processor interrupts.
    IntMasterEnable();
    
    // Enable the Timer0 interrupt on the processor
    IntEnable(INT_TIMER0A);

    // Configure the Timer0 interrupt for timer timeout.
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    // Enable Timer0.
    TimerEnable(TIMER0_BASE, TIMER_A);
    vTaskDelay(100);
  }
}