#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "lab4.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/adc.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/pwm.h"
#include "driverlib/sysctl.h"
#include "lcd_message.h" // messages to OLED
// FFT
#include "optfft.h"

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

// Bring into scope handle for image cap task
extern xTaskHandle imageCaptureHandle;
// TODO DEBUGGING send frequency to OLED
extern xQueueHandle xOLEDQueue;

#define NUM_SAMPLES 256
// Sample rate in Hz
#define SAMPLE_FREQ 36600

double
computeFrequency(signed int);

void
imageCapture(void* taskDataPtr)
{
	imageCaptureDataStruct* dataPtr = (imageCaptureDataStruct*) taskDataPtr;
	//unsigned int* rawDataPtr = dataPtr->rawDataPtr;
	//signed int* realData = dataPtr->processedDataPtr;
	signed int realData[256] = {0};
	double* frequency = dataPtr->frequencyPtr;

	while(1)
	{
		// Disable interrupts during FFT reading
		IntMasterDisable();

		// Compute FFT
		signed int imaginaryData[256] = {0};

		for (int i = 0; i < NUM_SAMPLES - 1; i++)
		{
			unsigned int adcReading[1] = {0};

			// Collect input image data
		    // Below code adapted from temperature_sensor.c in IAR examples/peripherals
		    // Clear the interrupt status flag.  This is done to make sure the
		    // interrupt flag is cleared before we sample.
		    ADCIntClear(ADC0_BASE, 3);

		    // Trigger the ADC conversion.
		    ADCProcessorTrigger(ADC0_BASE, 3);

		    // Wait for conversion to be completed.
		    while(!ADCIntStatus(ADC0_BASE, 3, false))
		    {
		    }

		    // Clear the ADC interrupt flag.
		    ADCIntClear(ADC0_BASE, 3);

		    // Read ADC Value from ADC0.
		    ADCSequenceDataGet(ADC0_BASE, 3, adcReading);

		    // convert adcReading from 0, 1023 to -31, 32
		    // 1023 / 16 = 63 -> 0 to 63; 63 - 31 = 32, 0 - 31 = -31
		    signed int adcReadingConverted = ((signed int)adcReading[0]) / 16 - 31;

		    realData[i] = adcReadingConverted;
		    
		    // TODO Delay 500us between readings; DELAY MUST BE CONSISTENT
		    // The function delay (in cycles) = 3 * parameter
		    //SysCtlDelay(SysCtlClockGet() / 12);
		}

		// Range of samples is -31 to 32
		signed int maxIndex = optfft(realData, imaginaryData);

		// Get frequency of signal
		*frequency = (SAMPLE_FREQ * maxIndex) / SAMPLE_FREQ; // TODO do anything with frequency or compute in oled display funct?

        // Display computed image frequency
        char freqBuffer [24];
        xOLEDMessage xMsgFreq;
        unsigned int freq = (unsigned int) *frequency;
        // int nDigits = floor(log10(abs(freq))) + 1;
        // unsigned int freqInt = freq;
        // unsigned int freqDec = freq * pow(10, nDigits) - freqInt * pow(10, nDigits);
        snprintf(freqBuffer, 24, "Image Freq: %d      ", freq);
        xMsgFreq.pcMessage = freqBuffer;
        xQueueSend( xOLEDQueue, &xMsgFreq, 0 );

		IntMasterEnable();

		vTaskDelay(100);
	}
}

double
computeFrequency(signed int maxIndex)
{
	double frequency = SAMPLE_FREQ * maxIndex / NUM_SAMPLES;
	return frequency;
}
