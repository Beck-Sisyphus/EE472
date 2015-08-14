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

void powerSub(void* taskDataPtr)
{
  
    powerSubDataStruct* dataPtr = (powerSubDataStruct*) taskDataPtr;
    unsigned int* battLevel = (unsigned int*) dataPtr->battLevelPtr; // Points to address of battLevelPtr[0]
    unsigned short* powerConsumption = (unsigned short*) dataPtr->powerConsumptionPtr;
    unsigned short* powerGeneration = (unsigned short*) dataPtr->powerGenerationPtr;
    Bool* panelState = (Bool*) dataPtr->panelStatePtr;
    Bool* panelDeploy = (Bool*) dataPtr->panelDeployPtr;
    Bool* panelRetract = (Bool*) dataPtr->panelRetractPtr;
    
    while(1)
    {
      /*
        //TODO FIX ME!!!
        if (!(*panelState) && panelDone)
        {
            *panelState = TRUE;
            panelDone = FALSE;
        }
        else if ((*panelState) && panelDone)
        {
            *panelState = FALSE;
            panelDone = FALSE;
        }
        
        //powerGeneration
        if (!(panelState)&&((*battLevel)<=20)) {                       //else solar panel not deployed...
              //if battery less than/equal to 20%
              xTaskResume(solarPanelHandle);
              xTaskResume(consoleKeyboardHandle);
              (*panelDeploy) = TRUE;              //deploy solar panel
              (*panelRetract) = FALSE;
        } 
        if (panelState&&((*battLevel)>95)) {                          //if solar panel is deployed...
              //if battery greater than 95%
              xTaskResume(solarPanelHandle);
              xTaskResume(consoleKeyboardHandle);
              (*panelDeploy) = FALSE;
              (*panelRetract)=TRUE;               //retract solar panel
        }
        */
        static unsigned long adcReading[8] = {0};

        // Below code adapted from temperature_sensor.c in IAR examples/peripherals
        // Clear the interrupt status flag.  This is done to make sure the
        // interrupt flag is cleared before we sample.
        ADCIntClear(ADC0_BASE, 0);

        // Trigger the ADC conversion.
        ADCProcessorTrigger(ADC0_BASE, 0);

        // Wait for conversion to be completed.
        while(!ADCIntStatus(ADC0_BASE, 0, false))
        {
        }

        // Clear the ADC interrupt flag.
        ADCIntClear(ADC0_BASE, 0);

        // Read ADC Value from ADC0.
        ADCSequenceDataGet(ADC0_BASE, 0, adcReading);

        // convert adcReading from 3.0V to 36V scale
        // If ADC returns 10bit int (0-1023), each digit ~= 0.00293V
        // Then, multiply by 0.098 to convert to percentage of 36V
        unsigned int adcReadingConverted = (int) (adcReading[0] * 0.098);
        // move previous readings to next array slot
        for (int i = 0; i < 15; ++i)
        {
            battLevel[i+1] = battLevel[i];
        }
        // Add new reading to front of circular buffer
        battLevel[0] = adcReadingConverted;


        // TODO Add over-temp flag for warning alarm
        // Start of ADC1
        //
        static unsigned long ADC1Reading[4] = {0};

        // interrupt flag is cleared before we sample.
        ADCIntClear(ADC0_BASE, 1);

        // Trigger the ADC conversion.
        ADCProcessorTrigger(ADC0_BASE, 1);

        // Wait for conversion to be completed.
        while(!ADCIntStatus(ADC0_BASE, 1, false))
        {
        }

        // Clear the ADC interrupt flag.
        ADCIntClear(ADC0_BASE, 1);

        // Read ADC Value.
        ADCSequenceDataGet(ADC0_BASE, 1, ADC1Reading);

        // 
        // Start of ADC2
        //
        static unsigned long ADC2Reading[4] = {0};


        // interrupt flag is cleared before we sample.
        ADCIntClear(ADC0_BASE, 2);

        // Trigger the ADC conversion.
        ADCProcessorTrigger(ADC0_BASE, 2);

        // Wait for conversion to be completed.
        while(!ADCIntStatus(ADC0_BASE, 2, false))
        {
        }

        // Clear the ADC interrupt flag.
        ADCIntClear(ADC0_BASE, 2);

        // Read ADC Value.
        ADCSequenceDataGet(ADC0_BASE, 2, ADC2Reading);

        vTaskDelay(100);
    }
}