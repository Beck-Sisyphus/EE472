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
        
    while(1) 
    {  
        //TODO add battery temp warning state 
        //use PWMGenEnable(PWM0_BASE, PWM_GEN_0); 
        //unsigned long ulPeriod = SysCtlClockGet() / 80; //run at 2Hz
        //PWMPulseWidthSet(PWM0_BASE, PWM_OUT_1, ulPeriod * 3 / 4); to start buzzing
        //PWMPulseWidthSet(PWM0_BASE, PWM_OUT_1, 0); to stop buzzing

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
                    if ((0==blinkTimer)||(2==blinkTimer)||(4==blinkTimer)||
                        (6==blinkTimer))
                        GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_6, 0xFF);
                    if ((1==blinkTimer)||(3==blinkTimer)||(5==blinkTimer)||
                        (7==blinkTimer))
                        GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_6, 0x00);
                }
                else if (*battLevel<HALF_BATT_LEVEL){
                    //TODO flash yellow 2 sec
                    if ((0==blinkTimer)||(1==blinkTimer)||(4==blinkTimer)||(5==blinkTimer))  
                        GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_6, 0xFF);
                    if ((2==blinkTimer)||(3==blinkTimer)||(6==blinkTimer)||(7==blinkTimer))  
                        GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_6, 0x00);
                }
                if ((*fuelLevel<FUEL_WARN_LEVEL)&(*fuelLevel<HALF_FUEL_LEVEL)){
                    //TODO flash red 1 sec
                    if ((0==blinkTimer)||(2==blinkTimer)||(4==blinkTimer)||(6==blinkTimer))  
                        GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_7, 0xFF);
                    if ((1==blinkTimer)||(3==blinkTimer)||(5==blinkTimer)||(7==blinkTimer))  
                        GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_7, 0x00);
                }
                else if (*fuelLevel<HALF_FUEL_LEVEL){
                    //TODO flash red 2 sec
                    if ((0==blinkTimer)||(1==blinkTimer)||(4==blinkTimer)||(5==blinkTimer))  
                        GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_7, 0xFF);
                    if ((2==blinkTimer)||(3==blinkTimer)||(6==blinkTimer)||(7==blinkTimer))  
                        GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_7, 0x00);
                }
        }
        vTaskDelay(100);
    } 
}