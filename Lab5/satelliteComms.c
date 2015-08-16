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

/* 
  The queue used to send messages to the OLED task.
  Defined and initialized in main.c 
*/
extern xQueueHandle xOLEDQueue;

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
    unsigned short* command = (unsigned short*)(commPtr->thrustPtr);
    unsigned short* powerConsumptionSignal = (unsigned short*)commPtr->powerConsumptionPtr;
    unsigned short* powerGenerationSignal = (unsigned short*)commPtr->powerGenerationPtr;
    Bool* panelStateSignal = (Bool*)commPtr->panelStatePtr;    
    while(1)
    {    
        if (isMajorCycle)
        {
            // receive (rando) thrust commands, generate from 0 to 2^16 -1
            unsigned short thrustCommand = randomInteger(0,65535);
            thrustCommand = thrustCommand % 65535;
            *command = thrustCommand;
        }    
        vTaskDelay(100);    
    }
}