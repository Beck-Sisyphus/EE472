#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
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

extern unsigned long thrust; 
extern char remoteCommand[ 100 ];
extern char commandResponse[4];
extern char mResponse[40];
extern Bool mRequestValid;
extern xTaskHandle pirateHandle;
extern xTaskHandle imageCaptureHandle;
extern xTaskHandle transportHandle;
extern xTaskHandle commandHandle;

extern Bool fuelLow;
extern Bool battLow;
extern Bool panelState;
extern uint32_t fuelLevel;
extern unsigned int batteryLevelArray[16];
extern unsigned int battTempArray0[16];
extern unsigned long transportDistance;
extern double imageFrequency;

void command(void* taskDataPtr) {
	static unsigned short OLEDon = 1; // Assign it to 0x00
	while (1) {
		commandDataStruct* commPtr = (commandDataStruct*) taskDataPtr;
		// char* remoteCommandPtr = commPtr->remoteCommandPtr;
		if (remoteCommand!= NULL) {
			if (remoteCommand[0] == 'S' && remoteCommand[1] == '\0')
			{
				vTaskResume(pirateHandle);
				vTaskResume(imageCaptureHandle);
				vTaskResume(transportHandle);
				snprintf(commandResponse, 4, "A S");
			} 
			else if (remoteCommand[0] == 'P' && remoteCommand[1] == '\0')
			{
				vTaskSuspend(pirateHandle);
				vTaskSuspend(imageCaptureHandle);
				vTaskSuspend(transportHandle);
				snprintf(commandResponse, 4, "A P");
			} 
			else if (remoteCommand[0] == 'D' && remoteCommand[1] == '\0')
			{
				if (OLEDon)
				{
					RIT128x96x4DisplayOff();
				} else {
					RIT128x96x4DisplayOn();
				}
				OLEDon = ~OLEDon;
				snprintf(commandResponse, 4, "A D");
			} 
			else if (remoteCommand[0] == 'T')
			{
				char * pEnd;
				thrust = strtol(&remoteCommand[1], &pEnd,10);
				snprintf(commandResponse, 4, "A T");
			}
			else if (remoteCommand[0] == 'M')
			{
				// <ol>
				// 	<li>Fuel Low</li>
				// 	<li>Battery Low</li>
				// 	<li>Solar Panel State</li>
				// 	<li>Fuel Level</li>
				// 	<li>Battery Level</li>
				// 	<li>Battery Temperature</li>
				// 	<li>Transport Distance</li>
				// 	<li>Image Data</li>
				// </ol>
				snprintf(commandResponse, 4, "M");

				char *p1, *p2, *p3, *p4, *p5;
				p1 = strstr( remoteCommand, "Fuel" );
				p2 = strstr( remoteCommand, "Battery" );
				p3 = strstr( remoteCommand, "Solar" );
				p4 = strstr( remoteCommand, "Transport" );
				p5 = strstr( remoteCommand, "Image" );
				if (p1 != NULL)
				{
					char *ptr1, *ptr2;
					ptr1 = strstr( remoteCommand, "Fuel Low");
					ptr2 = strstr( remoteCommand, "Fuel Lev");
					if (ptr1 != NULL)
					{
						snprintf(mResponse, 40, 
							"Fuel Low is %s",
							fuelLow ? "TRUE" : "FALSE");
					}
					else if (ptr2 != NULL)
					{
						snprintf(mResponse, 40, 
							"Fuel Level is %u",
							fuelLevel);
					}

				} 
				else if (p2 != NULL)
				{
					char *ptr1, *ptr2, *ptr3;
					ptr1 = strstr( remoteCommand, "Battery Low");
					ptr2 = strstr( remoteCommand, "Battery Lev");
					ptr3 = strstr( remoteCommand, "Battery Temp");
					if (ptr1 != NULL)
					{
						snprintf(mResponse, 40, 
							"Battery Low is %s",
							battLow ? "TRUE" : "FALSE");
					}
					else if (ptr2 != NULL)
					{
						snprintf(mResponse, 40, 
							"Battery Level is %hu",
							batteryLevelArray[0]);
					}
					else if (ptr3 != NULL)
					{
						snprintf(mResponse, 40, 
							"Battery Temperature is %hu",
							battTempArray0[0]);
					}
				}
				else if (p3 != NULL)
				{
					snprintf(mResponse, 40, 
						"Solar Panel State is %s",
						panelState ? "DEPLOYED" : "RETRACTED");
				}
				else if (p4 != NULL)
				{
					snprintf(mResponse, 40, 
						"Transport Distance is %lu",
						transportDistance);
				}
				else if (p5 != NULL)
				{
					snprintf(mResponse, 40, 
						"Image Frequency is %d",
						(int)imageFrequency);
				}
				else {
					snprintf(mResponse, 40,
						"Invalid Request, %s",
						remoteCommand);
				}
			}
			else {
				snprintf(commandResponse, 4, "E");
			}
		}
		vTaskSuspend(commandHandle);
	}
}