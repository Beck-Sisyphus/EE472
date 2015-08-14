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
extern const unsigned short MAX_BATT_LEVEL;
extern const unsigned short HALF_BATT_LEVEL;
extern const unsigned short BATT_WARN_LEVEL;
extern const uint32_t MAX_FUEL_LEVEL;
extern const uint32_t HALF_FUEL_LEVEL;
extern const uint32_t FUEL_WARN_LEVEL;
extern const unsigned short TASK_QUEUE_LENGTH;
extern unsigned short globalCount;
extern unsigned short blinkTimer;
extern uint32_t fuelLevellll;
extern Bool panelDone;
extern Bool hasNewKeyboardInput;
extern unsigned int* battLevelPtr;

/* 
  The queue used to send messages to the OLED task.
  Defined and initialized in main.c 
*/
extern xQueueHandle xOLEDQueue;

// local variable used in functions
const int fuelBuringRatio = 2000; // Set as a large number in demo
const int sysDelayLength = 1000000;


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
    unsigned short* powerConsumptionSignal = (unsigned short*)commPtr->powerConsumptionPtr;
    unsigned short* powerGenerationSignal = (unsigned short*)commPtr->powerGenerationPtr;
    Bool* panelStateSignal = (Bool*)commPtr->panelStatePtr; 
    
    while(1)
    {    
        
        if (isMajorCycle)
        {
            // receive (rando) thrust commands, generate from 0 to 2^16 -1
            uint16_t thrustCommand = randomInteger(0,65535);
            thrustCommand = thrustCommand % 65535;
            // uint16_t thrustCommand = 0x0FF1;
            *(uint16_t*)(commPtr->thrustPtr) = thrustCommand;
        }    
        vTaskDelay(100);    
    }
    return;
}