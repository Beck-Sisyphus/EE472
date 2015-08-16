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
extern Bool hasNewKeyboardInput;

/* 
  The queue used to send messages to the OLED task.
  Defined and initialized in main.c 
*/
extern xQueueHandle xOLEDQueue;

void vehicleComms(void* taskDataPtr)
{
    vehicleCommsStruct* dataPtr = (vehicleCommsStruct*) taskDataPtr;
    unsigned char* vehicleCommandLocal = (unsigned char*) dataPtr->vehicleCommandPtr;
    unsigned char* vehicleResponseLocal = (unsigned char*) dataPtr->vehicleResponsePtr;
    
    while(1)
    {
        // Clean up the screen
        UARTSend((unsigned char *)"\033[2J", 6);
        
        vehicleResponseLocal[0] = 'A';
        vehicleResponseLocal[1] = ' ';

        // Receive command
        if(hasNewKeyboardInput)
        {
            if ('F' == *vehicleCommandLocal || 'B' == *vehicleCommandLocal ||
                'L' == *vehicleCommandLocal || 'R' == *vehicleCommandLocal ||
                'D' == *vehicleCommandLocal || 'H' == *vehicleCommandLocal ||
                'T' == *vehicleCommandLocal || 'D' == *vehicleCommandLocal ||
                'K' == *vehicleCommandLocal || 'C' == *vehicleCommandLocal) {
                vehicleResponseLocal[0] = 'A';
                UARTSend((unsigned char *)"Valid Command:", 14);
            } else {
                vehicleResponseLocal[0] = '-';
                UARTSend((unsigned char *)"Invalid Command:", 16);
            }
            hasNewKeyboardInput = FALSE;
            UARTSend((unsigned char*)vehicleResponseLocal, 3);
        }   else {
            UARTSend((unsigned char *)"Enter Command:", 14);
        }      

        vTaskDelay(100);
    }
}