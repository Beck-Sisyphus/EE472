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


// Constants defined in main
extern unsigned short globalCount;
extern unsigned short blinkTimer8;
extern unsigned short blinkTimer10;
extern unsigned short tempAlarm;

/* 
  The queue used to send messages to the OLED task.
  Defined and initialized in main.c 
*/
extern xQueueHandle xOLEDQueue;

const int sysDelayLength = 1000000;

// Control the major or minor cycle in main function
void schedule(void* taskDataPtr)
{
    scheduleDataStruct* dataPtr = (scheduleDataStruct*) taskDataPtr;
    Bool* isMajorCycle = (Bool*) dataPtr->isMajorCyclePtr;
    while(1){
        //Execute a Major Cycle when the count is zero.
        if (0  == globalCount) 
        {
            *isMajorCycle = TRUE;
        } 
        else 
        {
            *isMajorCycle = FALSE; 
        }
        SysCtlDelay(sysDelayLength);

        blinkTimer8 = (blinkTimer8 + 1) % 8;
        blinkTimer10 = (blinkTimer10 + 1) % 10;
        tempAlarm = (tempAlarm + 1);
        vTaskDelay(100);
    }
}