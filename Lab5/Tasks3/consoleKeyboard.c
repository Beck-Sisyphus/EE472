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

void consoleKeyboard(void* taskDataPtr)
{
    keyboardDataStruct* keyboardData = (keyboardDataStruct*) taskDataPtr;
    Bool* panelMotorSpeedUp = (Bool*) keyboardData->panelMotorSpeedUpPtr;
    Bool* panelMotorSpeedDown = (Bool*) keyboardData->panelMotorSpeedDownPtr;
  
    while(1)
    {
        // Read keypad input and adjust duty cycle based on keypress
        if (GPIOPinRead(GPIO_PORTB_BASE, GPIO_PIN_3)){
            *panelMotorSpeedUp = TRUE;
        }
        else {
            *panelMotorSpeedUp = FALSE;
        }
        
        if (GPIOPinRead(GPIO_PORTB_BASE, GPIO_PIN_2)){
            *panelMotorSpeedDown = TRUE;
        }
        else {
            *panelMotorSpeedDown = FALSE;
        }
        vTaskDelay(100);
    }
}