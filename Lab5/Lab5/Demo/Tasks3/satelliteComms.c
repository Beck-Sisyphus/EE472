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
extern char systemInfo[ 500 ];		
extern Bool fuelLow;		
extern Bool battLow;		
extern Bool panelState;		
extern uint32_t fuelLevel;		
extern unsigned short powerConsumption;		
extern unsigned int batteryLevelArray[16];		
extern unsigned int battTempArray0[16];		
extern unsigned long transportDistance;

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
    char* systemInfoPtr = (char*)commPtr->systemInfoPtr;
    unsigned short* battTempSignal = (unsigned short*)commPtr->battTempPtr0;
    unsigned long* transportDistancePtr = (unsigned long*)commPtr->transportDistancePtr;
    
    volatile unsigned long rand = 0;
    while(1)
    {    
        if (isMajorCycle)
        {
            // receive (rando) thrust commands, generate from 0 to 2^16 -1
            rand = randomInteger(0,65535);
            thrust = rand;
        }
		snprintf(&systemInfo[0],  44, 		
                 "<tr><td>Fuel Low</td><td> %s </td></tr>", 		
                 "FALSE");		
        //                 fuelLow ? "FALSE" : "TRUE",		
        snprintf(&systemInfo[44], 46, 		
                 "<tr><td>Battery Low</td><td> %s </td></tr>", 		
                 "FALSE");		
        //                 battLow ? "FALSE" : "TRUE",		
        snprintf(&systemInfo[90], 55, 		
                 "<tr><td>Solar Panel State</td><td> %s </td></tr>", 		
                 "RETRACTED");		
        //                 panelState ? "RETRACTED" : "DEPLOYED",		
        snprintf(&systemInfo[145], 50, 		
                 "<tr><td>Fuel Level</td><td> %u </td></tr>", 		
                 fuelLevel);			
        snprintf(&systemInfo[245], 50, 		
                 "<tr><td>Battery Level</td><td> %hu </td></tr>", 		
                 batteryLevelArray[0]);		
        snprintf(&systemInfo[295], 50, 		
                 "<tr><td>Battery Temperature</td><td> %hu </td></tr>",		
                 battTempArray0[0]);		
        snprintf(&systemInfo[345], 55, 		
                 "<tr><td>Transport Distance</td><td> %lu </td></tr>", 		
                 transportDistance);		
        snprintf(&systemInfo[400], 70, 		
                 "<tr><td>Image Data</td><td> %s </td></tr>", 		
                 "None");		
        vTaskDelay(100);    
    }
}