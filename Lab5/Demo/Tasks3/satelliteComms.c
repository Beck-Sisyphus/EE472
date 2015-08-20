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

extern Bool fuelLow;
extern Bool battLow;
extern Bool panelState;
extern uint32_t fuelLevel;
extern unsigned int batteryLevelArray[16];
extern unsigned int battTempArray0[16];
extern unsigned long transportDistance;
extern double imageFrequency;

extern char systemInfo[ 430 ];
extern char specificInfo[ 100 ];

extern char commandResponse[4];
extern char mResponse[40];
extern Bool newCommand;
extern xTaskHandle commandHandle;

// Communication only store the command without decoding it
// Require : satellite communication data struct,
//           randomInteger function;
// Modifies: thrust command.


void satelliteComms(void* taskDataPtr)
{
    satelliteCommsDataStruct* commPtr = (satelliteCommsDataStruct*) taskDataPtr;
    unsigned short* globalCount = (unsigned short*) commPtr->globalCountPtr;
    Bool* isMajorCycle = (Bool*) commPtr->isMajorCyclePtr;
    unsigned long* thrustPtr = (unsigned long*) commPtr->thrustPtr;
	
    // fields to be displayed
    Bool* fuelLowSignal = (Bool*)commPtr->fuelLowPtr;
    Bool* battLowSignal = (Bool*)commPtr->battLowPtr;
    Bool* panelStateSignal = (Bool*)commPtr->panelStatePtr;
    uint32_t* fuelLevelSignal = (uint32_t*)commPtr->fuelLevelPtr;
    unsigned short* battLevelSignal = (unsigned short*)commPtr->battLevelPtr;
    unsigned short* battTempSignal = (unsigned short*)commPtr->battTempPtr0;
    unsigned long* transportDistancePtr = (unsigned long*)commPtr->transportDistancePtr;
    char* systemInfoPtr = (char*)commPtr->systemInfoPtr;	

    
    volatile unsigned long rand = 0;
    while(1)
    {    
        if (newCommand)
        {
            vTaskResume(commandHandle);
        }

        newCommand = FALSE;

        snprintf(systemInfo,  430, 
                 "<tr><td>Fuel Low</td><td> %s </td></tr>\
<tr><td>Battery Low</td><td> %s </td></tr>\
<tr><td>Solar Panel State</td><td> %s </td></tr>\
<tr><td>Fuel Level</td><td> %u </td></tr>\
<tr><td>Battery Level</td><td> %hu </td></tr>\
<tr><td>Battery Temperature</td><td> %hu </td></tr>\
<tr><td>Transport Distance</td><td> %lu </td></tr>\
<tr><td>Image Frequency</td><td> %d </td></tr>", 
                 fuelLow ? "TRUE" : "FALSE",
                 battLow ? "TRUE" : "FALSE",
                 panelState ? "DEPLOYED" : "RETRACTED",
                 fuelLevel,
                 batteryLevelArray[0],
                 battTempArray0[0],
                 transportDistance,
                 (int)imageFrequency
                 );

        
        snprintf(specificInfo, 100,
        "<tr><td>%s</td></tr><tr><td>%s</td></tr>",
        commandResponse,
        mResponse
        );
        

        vTaskDelay(100);    
    }
}
