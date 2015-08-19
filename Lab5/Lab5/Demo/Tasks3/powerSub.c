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
extern Bool panelDone;

/* 
  The queue used to send messages to the OLED task.
  Defined and initialized in main.c 
*/
extern xQueueHandle xOLEDQueue;
extern xTaskHandle solarPanelHandle;
extern xTaskHandle consoleKeyboardHandle;
extern xTaskHandle pirateHandle;
extern Bool battOverTemp;

void powerSub(void* taskDataPtr)
{
    powerSubDataStruct* dataPtr = (powerSubDataStruct*) taskDataPtr;
    unsigned int* battLevel = dataPtr->battLevelPtr; // Points to address of battLevelPtr[0]
    unsigned int* battTempArr0 = dataPtr->battTempPtr0; // Points to address of battTempPtr0[0]
    unsigned int* battTempArr1 = dataPtr->battTempPtr1; // Points to address of battTempPtr1[0]
    Bool* panelStatePtr = dataPtr->panelStatePtr;
    Bool* panelDeployPtr = dataPtr->panelDeployPtr;
    Bool* panelRetractPtr = dataPtr->panelRetractPtr;
    
    while(1)
    {
        if (!(*panelStatePtr) && panelDone)
        {
            *panelStatePtr = TRUE;
            panelDone = FALSE;
            PWMPulseWidthSet(PWM0_BASE, PWM_OUT_0, 0);
            PWMGenEnable(PWM0_BASE, PWM_GEN_0);
            vTaskSuspend(solarPanelHandle);
            vTaskSuspend(consoleKeyboardHandle);
        }
        else if ((*panelStatePtr) && panelDone)
        {
            *panelStatePtr = FALSE;
            panelDone = FALSE;
            PWMPulseWidthSet(PWM0_BASE, PWM_OUT_0, 0);
            PWMGenEnable(PWM0_BASE, PWM_GEN_0);
            vTaskSuspend(solarPanelHandle);
            vTaskSuspend(consoleKeyboardHandle);
        }
        
        //powerGeneration
        if (!(*panelStatePtr)&&((*battLevel)<=20)) {                       //else solar panel not deployed...
              //if battery less than/equal to 20%
              (*panelDeployPtr) = TRUE;              //deploy solar panel
              (*panelRetractPtr) = FALSE;
              vTaskResume(solarPanelHandle);
              vTaskResume(consoleKeyboardHandle);
        } 
        if ((*panelStatePtr)&&((*battLevel)>95)) {                          //if solar panel is deployed...
              //if battery greater than 95%
              (*panelDeployPtr) = FALSE;
              (*panelRetractPtr)=TRUE;               //retract solar panel
              vTaskResume(solarPanelHandle);
              vTaskResume(consoleKeyboardHandle);
        }
        
        static unsigned long adc0Reading[8] = {0};

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
        ADCSequenceDataGet(ADC0_BASE, 0, adc0Reading);

        // convert adcReading from 3.0V to 36V scale
        // If ADC returns 10bit int (0-1023), each digit ~= 0.00293V
        // Then, multiply by 0.098 to convert to percentage of 36V
        unsigned int adcReadingConverted = (int) (adc0Reading[0] * 0.098);
        // move previous readings to next array slot
        for (int i = 15; i > 0; i--)
        {
            battLevel[i] = battLevel[i-1];
            battTempArr0[i] = battTempArr0[i-1];
            battTempArr1[i] = battTempArr1[i-1];
        }
        // Add new reading to front of circular buffer
        battLevel[0] = adcReadingConverted;


        // TODO Add over-temp flag for warning alarm
        // Start of ADC1
        //
        static unsigned long adc1Reading[4] = {0};

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
        ADCSequenceDataGet(ADC0_BASE, 1, adc1Reading);

        // 
        // Start of ADC2
        //
        static unsigned long adc2Reading[4] = {0};


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
        ADCSequenceDataGet(ADC0_BASE, 2, adc2Reading);
        
        if(adc2Reading[2]>300){
             vTaskResume(pirateHandle);
        }

        // Convert raw temp readings to temperatures in Celsius
        int temp0 = (int) (adc1Reading[1] * 32 + 33);
        int temp1 = (int) (adc1Reading[2] * 32 + 33);                           //Changed to copy channel 1 to free up ADCs
        // Store temperature readings in arrays
        // TODO stored temps specified to be in millivolts...
        battTempArr0[0] = temp0;
        battTempArr1[0] = temp1;

        // Check battery overtemp condition: if current temp reading 
        //  exceeds largest of previous 2 values by more than 20%
        if ((temp0 > 1.2 * max(battTempArr0[1], battTempArr0[2])) ||
            (temp1 > 1.2 * max(battTempArr1[1], battTempArr1[2])))
            battOverTemp = TRUE;
        
        vTaskDelay(100);
    }
}