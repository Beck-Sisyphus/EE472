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
// FFT
#include "optfft.h"

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

// Bring image frequency global variable into scope
extern double imageFrequency;
extern xTaskHandle imageCaptureHandle;

#define NUM_SAMPLES 256
// Sample rate in Hz
#define SAMPLE_FREQ 34500

double
computeFrequency(signed int);

void
imageCapture(void* taskDataPtr)
{
	imageCaptureDataStruct* dataPtr = (imageCaptureDataStruct*) taskDataPtr;
	signed int* realData = dataPtr->processedDataPtr;
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
		}

		// Range of samples is -31 to 32
		signed int maxIndex = optfft(realData, imaginaryData);

		// Get frequency of signal
		*frequency = (SAMPLE_FREQ * maxIndex) / NUM_SAMPLES;

		// Update global variable
        imageFrequency = *frequency;

        // Re-enable interrupts
        IntMasterEnable();
        vTaskSuspend(imageCaptureHandle);
        vTaskDelay(100);
	}
}

double
computeFrequency(signed int maxIndex)
{
	double frequency = SAMPLE_FREQ * maxIndex / NUM_SAMPLES;
	return frequency;
}
