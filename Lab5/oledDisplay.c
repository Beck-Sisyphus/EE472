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

void oledDisplay(void* taskDataPtr)
{
    oledDisplayDataStruct* dataPtr = (oledDisplayDataStruct*) taskDataPtr;
        
    unsigned int* battLevel = (unsigned int*) dataPtr->battLevelPtr;
    uint32_t* fuelLevelPtr2 = (uint32_t*) dataPtr->fuelLevelPtr;
    unsigned short* powerConsumption = (unsigned short*) dataPtr->powerConsumptionPtr;
    Bool* panelState = (Bool*) dataPtr->panelStatePtr;
    Bool* fuelLow = (Bool*) dataPtr->fuelLowPtr;
    Bool* battLow = (Bool*) dataPtr->battLowPtr;
    unsigned int* battTemp0 = (unsigned int*) dataPtr->battTempPtr0;
    unsigned int* battTemp1 = (unsigned int*) dataPtr->battTempPtr1;
    unsigned int* distance = (unsigned int*) dataPtr->transportDistancePtr;
  
    xOLEDMessage xMsgBlank;
    xOLEDMessage xMsgPanelState;
    xOLEDMessage xMsgBattLev;
    xOLEDMessage xMsgBattTemp;
    xOLEDMessage xMsgFuelLev;
    xOLEDMessage xMsgDist;
    xOLEDMessage xMsgFuelLow;
    xOLEDMessage xMsgBattLow;
    
    while(1) 
    {
        // Push select button to change to annunciation mode on OLED
        long buttonRead = 2;

        // Pushed = 0, released = 2 from our measurement
        buttonRead = GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_1);

        xMsgBlank.pcMessage = "                ";
     
        if (2 == buttonRead)    // Status display mode
        {
          // Display panel state.
          if (1== *panelState)
          {
              xMsgPanelState.pcMessage = "Panel: Deployed";
          }
          else
          {
              xMsgPanelState.pcMessage = "Panel: Retracted  ";
          }
          xQueueSend( xOLEDQueue, &xMsgPanelState, 0 );
          
          // Display battery level
          char battBuffer [18];
          snprintf(battBuffer, 18, "Battery: %d    ", *battLevel);
          xMsgBattLev.pcMessage = battBuffer;
          xQueueSend( xOLEDQueue, &xMsgBattLev, 0 );

          // Display battery temperature
          char tempBuffer [20];
          snprintf(tempBuffer, 20, "Battery Temp: %d  ", *battTemp0);
          xMsgBattTemp.pcMessage = tempBuffer;
          xQueueSend( xOLEDQueue, &xMsgBattTemp, 0 );

          // Display fuel level
          char fuelBuffer [12];
          snprintf(fuelBuffer, 12, "Fuel: %d", fuelLevellll);
          xMsgFuelLev.pcMessage = fuelBuffer;
          xQueueSend( xOLEDQueue, &xMsgFuelLev, 0 );

          // Display transport distance ptr
          char transBuffer [16];
          snprintf(transBuffer, 16, "Distance: %d", *distance);
          xMsgDist.pcMessage = transBuffer;
          xQueueSend( xOLEDQueue, &xMsgDist, 0 );

          xQueueSend( xOLEDQueue, &xMsgBlank, 0 );
          xQueueSend( xOLEDQueue, &xMsgBlank, 0 );
          xQueueSend( xOLEDQueue, &xMsgBlank, 0 );
          xQueueSend( xOLEDQueue, &xMsgBlank, 0 );
            
        } 
        else if (0 == buttonRead) // Annunciation mode
        {
            //RIT128x96x4Clear();
            
            // Display fuel low flag.
            if (1 == *fuelLow)
            {
                xMsgFuelLow.pcMessage = "FUEL LOW!    ";
            }
            else
            {
                xMsgFuelLow.pcMessage = "Fuel Good :)    ";
            }
            xQueueSend( xOLEDQueue, &xMsgFuelLow, 0 );


            // Display battery low flag.
            if (1 == *battLow)
            {
                xMsgBattLow.pcMessage = "BATTERY LOW!  ";
            }
            else
            {
                xMsgBattLow.pcMessage = "Battery Good :)  ";
            }
            xQueueSend( xOLEDQueue, &xMsgBattLow, 0 );

            xQueueSend( xOLEDQueue, &xMsgBlank, 0 );
            xQueueSend( xOLEDQueue, &xMsgBlank, 0 );
            xQueueSend( xOLEDQueue, &xMsgBlank, 0 );
            xQueueSend( xOLEDQueue, &xMsgBlank, 0 );
            xQueueSend( xOLEDQueue, &xMsgBlank, 0 );
            xQueueSend( xOLEDQueue, &xMsgBlank, 0 );
            xQueueSend( xOLEDQueue, &xMsgBlank, 0 );
        }        
        vTaskDelay(100);
    }
}