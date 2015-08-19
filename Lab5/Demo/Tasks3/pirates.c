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
double converted;

// Control the major or minor cycle in main function
void pirates(void* taskDataPtr)
{
    pirateDataStruct* dataPtr = (pirateDataStruct*) taskDataPtr;
    unsigned short* pirateProximityPtr = dataPtr->pirateProximity;
    while (1){	 
        // Start of ADC2                                                    //removed to free up ADC lines
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
        converted = (double) (1023 - adc2Reading[2]) * (200.0 / 1023) * 2;
        *pirateProximityPtr = (unsigned short) converted;
        //disable phasor and photon
        GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_5, 0x00);
        GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_6, 0x00);
        if(*pirateProximityPtr>100){
          //disable phasor and photon
          GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_5, 0x00);
          GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_6, 0x00);
          //xTaskSuspend(piratesHandle);
        }
        else if(*pirateProximityPtr<30){
          //fire phasors fault pin
          GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_6, 0xFF);
        }
        if(*pirateProximityPtr<5){
          //fire photons PD5
          GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_5, 0xFF);
        }
        vTaskDelay(100);
    }
}