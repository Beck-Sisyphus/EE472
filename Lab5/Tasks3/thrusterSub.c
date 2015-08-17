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
extern const uint32_t MAX_FUEL_LEVEL;
extern unsigned short globalCount;
extern uint32_t fuelLevellll;

/* 
  The queue used to send messages to the OLED task.
  Defined and initialized in main.c 
*/
extern xQueueHandle xOLEDQueue;

// local variable used in functions
const int fuelBuringRatio = 2000; // Set as a large number in demo

// Require : the minor clock running at 1 second per cycle, 
//          and thruster sub data struct; 
// Modifies: global constant fuelLevel, taking in count of duration.
void thrusterSub(void* taskDataPtr)
{
    thrusterSubDataStruct* thrustCommandPtr = (thrusterSubDataStruct*) taskDataPtr; 
    // unsigned short* globalCount = (unsigned short*) thrustCommandPtr->globalCountPtr;
    Bool* isMajorCycle = (Bool*) thrustCommandPtr->isMajorCyclePtr;
    uint32_t* fuelPtr = (uint32_t*) thrustCommandPtr->fuelLevelPtr;
    unsigned long* command = (unsigned long*)(thrustCommandPtr->thrustPtr);
    static unsigned short left = 0;
    static unsigned short right = 0;
    static unsigned short up = 0;
    static unsigned short down = 0;
    static unsigned short magnitude = 0;
    static unsigned short duration = 0;
    
    while(1) 
    {
        // Only updates the magnitude when the duration is positive, otherwise no change on magnitude
        if (*isMajorCycle)
        {
            
            // get duration, unsigned char pointer moves every 8 bits
            // so it can separate first 8 bits and last 8 bits
            duration = *((unsigned char*)&command + 1);
            if (duration) { magnitude = ((*command) >> 4) & 0x000F; }
            if (*command & 0x0001) 
            { 
              left  = 1; 
            } 
            else 
            { 
              left  = 0;
            }
            if (*command & 0x0002) { right = 1; } else { right = 0;}
            if (*command & 0x0004) { up    = 1; } else { up    = 0;}
            if (*command & 0x0008) { down  = 1; } else { down  = 0;}
        }

        // If the new command ask use a 
        if (duration)
        {
            *fuelPtr -= magnitude * fuelBuringRatio;
            duration -= 1;
        }

        // When the fuel level goes below 0, the unsigned int when to really big
        if (*fuelPtr > MAX_FUEL_LEVEL)
        {
            *fuelPtr = 0;
        }
            
        // Convert inner counter to 100 scale for OLED display
        fuelLevellll = ((*fuelPtr) * 100) / MAX_FUEL_LEVEL;  

        vTaskDelay(100);      
    }
}