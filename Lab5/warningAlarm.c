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
extern const unsigned short HALF_BATT_LEVEL;
extern const unsigned short BATT_WARN_LEVEL;
extern const uint32_t MAX_FUEL_LEVEL;
extern const uint32_t FUEL_WARN_LEVEL;
extern unsigned short globalCount;
extern unsigned short blinkTimer8;
extern unsigned short blinkTimer10;
extern unsigned short tempAlarm;

/* 
  The queue used to send messages to the OLED task.
  Defined and initialized in main.c 
*/
extern xQueueHandle xOLEDQueue;

// Require : warning alarm data struct;
// Modifies: fuelLow pointer, battLow pointer.
void warningAlarm(void* taskDataPtr)
{
    warningAlarmDataStruct* dataPtr = (warningAlarmDataStruct*) taskDataPtr;
    Bool* fuelLow = (Bool*)dataPtr->fuelLowPtr;
    Bool* battLow = (Bool*)dataPtr->battLowPtr;
    unsigned int* battLevel = (unsigned int*)dataPtr->battLevelPtr;
    uint32_t* fuelLevel = (uint32_t*)dataPtr->fuelLevelPtr;
    unsigned short* globalCount = (unsigned short*) dataPtr->globalCountPtr;
    Bool* isMajorCycle = (Bool*) dataPtr->isMajorCyclePtr;
    Bool* battOverTemp = (Bool*) dataPtr->battOverTempPtr;
        
    while(1) 
    {  
        if (*fuelLevel < FUEL_WARN_LEVEL) { *fuelLow = TRUE; } else { *fuelLow = FALSE; }
        if (*battLevel < BATT_WARN_LEVEL) { *battLow = TRUE; } else { *battLow = FALSE; }
        
        if (((*battLevel)>HALF_BATT_LEVEL)&&((*fuelLevel)>HALF_FUEL_LEVEL)){
            //display solid green LED
            GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_5, 0xFF);
            //clear yellow LED
            GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_6, 0x00);
            //clear red LED
            GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_7, 0x00);
        }
        else{
            //clear solid green LED
            GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_5, 0x00);
            //SPEC CHANGE!!! YELLOW is BATT, RED is FUEL
                if ((*battLevel<BATT_WARN_LEVEL)&&(*battLevel<HALF_BATT_LEVEL)){
                    //TODO flash yellow 1 sec
                    if ((0==blinkTimer8)||(2==blinkTimer8)||(4==blinkTimer8)||
                        (6==blinkTimer8))
                        GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_6, 0xFF);
                    if ((1==blinkTimer8)||(3==blinkTimer8)||(5==blinkTimer8)||
                        (7==blinkTimer8))
                        GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_6, 0x00);
                }
                else if (*battLevel<HALF_BATT_LEVEL){
                    //TODO flash yellow 2 sec
                    if ((0==blinkTimer8)||(1==blinkTimer8)||(4==blinkTimer8)||(5==blinkTimer8))  
                        GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_6, 0xFF);
                    if ((2==blinkTimer8)||(3==blinkTimer8)||(6==blinkTimer8)||(7==blinkTimer8))  
                        GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_6, 0x00);
                }
                if ((*fuelLevel<FUEL_WARN_LEVEL)&(*fuelLevel<HALF_FUEL_LEVEL)){
                    //TODO flash red 1 sec
                    if ((0==blinkTimer8)||(2==blinkTimer8)||(4==blinkTimer8)||(6==blinkTimer8))  
                        GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_7, 0xFF);
                    if ((1==blinkTimer8)||(3==blinkTimer8)||(5==blinkTimer8)||(7==blinkTimer8))  
                        GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_7, 0x00);
                }
                else if (*fuelLevel<HALF_FUEL_LEVEL){
                    //TODO flash red 2 sec
                    if ((0==blinkTimer8)||(1==blinkTimer8)||(4==blinkTimer8)||(5==blinkTimer8))  
                        GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_7, 0xFF);
                    if ((2==blinkTimer8)||(3==blinkTimer8)||(6==blinkTimer8)||(7==blinkTimer8))  
                        GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_7, 0x00);
                }
        }

        // Part 4 audible alarm if battery is over temperature
        if (*battOverTemp)
        {
            //Start Alarm
            unsigned long ulPeriod = SysCtlClockGet() / 80; //run at 2Hz
            SysCtlPWMClockSet(SYSCTL_PWMDIV_64); //SYSCTL_PWMDIV_32
        
            //Enable PWM and GPIO pins to carry signal
            SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);
            SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOG);
            GPIOPinTypePWM(GPIO_PORTG_BASE, GPIO_PIN_1);
            PWMGenPeriodSet(PWM0_BASE, PWM_GEN_0, ulPeriod);
            PWMPulseWidthSet(PWM0_BASE, PWM_OUT_1, ulPeriod * 3 / 4);
            PWMGenConfigure(PWM0_BASE, PWM_GEN_0,
                        PWM_GEN_MODE_UP_DOWN | PWM_GEN_MODE_NO_SYNC);
            PWMOutputState(PWM0_BASE, PWM_OUT_0_BIT | PWM_OUT_1_BIT, true);
            PWMGenEnable(PWM0_BASE, PWM_GEN_0);

            // TODO button to acknowledge alarm & reset tempAlarm to 0                            DO THIS!!!!


            // tempAlarm has been unacknowledged for >15s
            if (tempAlarm >= 15) 
            {
                // Prevent tempAlarm from overflowing
                tempAlarm = 15;

                // Flash red and yellow LEDs for 5s at 10Hz
                if (6 > blinkTimer10)
                {
                    // Red
                    GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_7, 0xFF);
                    vTaskDelay(50); // Delay 50ms
                    GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_7, 0x00);

                    // Yellow
                    GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_6, 0xFF);
                    vTaskDelay(50); // Delay 50ms
                    GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_6, 0x00);
                }

                // Hold red and yellow LEDs solid for 5s
                if (6==blinkTimer10)  
                {
                    // Red
                    GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_7, 0xFF);
                    // Yellow
                    GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_6, 0xFF);
                }
            }
        }
        vTaskDelay(100);
    } 
}