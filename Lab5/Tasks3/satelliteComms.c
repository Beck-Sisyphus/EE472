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

/* 
  The queue used to send messages to the OLED task.
  Defined and initialized in main.c 
*/
extern xQueueHandle xOLEDQueue;
extern unsigned long thrust; 

// Communication only store the command without decoding it
// Require : satellite communication data struct,
//           randomInteger function;
// Modifies: thrust command.

void satelliteComms(void* taskDataPtr)
{
    satelliteCommsDataStruct* commPtr = (satelliteCommsDataStruct*) taskDataPtr;
    unsigned short* globalCount = (unsigned short*) commPtr->globalCountPtr;
    Bool* isMajorCycle = (Bool*) commPtr->isMajorCyclePtr;
    Bool* fuelLowSignal = (Bool*)commPtr->fuelLowPtr;
    Bool* battLowSignal = (Bool*)commPtr->battLowPtr;
    unsigned short* battLevelSignal = (unsigned short*)commPtr->battLevelPtr;
    uint32_t* fuelLevelSignal = (uint32_t*)commPtr->fuelLevelPtr;
    unsigned long* thrustPtr = (unsigned long*) commPtr->thrustPtr;
    Bool* panelStateSignal = (Bool*)commPtr->panelStatePtr;
    signed int* processedImageData = (signed int*) commPtr->processedImageDataPtr;
    
    volatile unsigned long rand = 0;
    while(1)
    {    
        if (isMajorCycle)
        {
            // receive (rando) thrust commands, generate from 0 to 2^16 -1
            rand = randomInteger(0,65535);
            thrust = rand;
        }    
        vTaskDelay(100);    
    }
}