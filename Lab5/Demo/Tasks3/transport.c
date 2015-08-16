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
extern unsigned long pulsecount;
extern unsigned long* transportTimeArray;
extern unsigned short transportTimeElement;

// Control the major or minor cycle in main function
void transport(void* taskDataPtr)
{
    transportDataStruct* dataPtr = (transportDataStruct*) taskDataPtr;
    unsigned long period;
    double transportDistance;
    while (1){
    	/*              IN ISR
    	transportTimeElement = (transportTimeElement + 1) % 2;
    	transportTimeArray[transportTimeElement] = xTaskGetTickCountFromISR();
        */
    	period = abs(transportTimeArray[0]-transportTimeArray[1]);
    	transportDistance = 1 / period;
        vTaskDelay(100);
    }
}