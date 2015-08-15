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

void solarPanelControl(void* taskDataPtr)
{
    solarPanelStruct* solarPanelPtr = (solarPanelStruct*) taskDataPtr;
    Bool* isMajorCycle = (Bool*) solarPanelPtr->isMajorCyclePtr;
    Bool* panelState = (Bool*) solarPanelPtr->panelStatePtr;
    Bool* panelDeploy = (Bool*) solarPanelPtr->panelDeployPtr;
    Bool* panelRetract = (Bool*) solarPanelPtr->panelRetractPtr;
    Bool* panelMotorSpeedUp = (Bool*) solarPanelPtr->panelMotorSpeedUpPtr;
    Bool* panelMotorSpeedDown = (Bool*) solarPanelPtr->panelMotorSpeedDownPtr;
    unsigned short* globalCount = (unsigned short*) solarPanelPtr->globalCountPtr;
    
    while(1)
    {
        // Compute the PWM period based on the system clock.
        // Base clock 8MHz, want 2Hz for panel motor ( / 4000000)
        unsigned long ulPeriod = SysCtlClockGet() / 80; //run at 2Hz
        static unsigned long dutyCycle = 5;
        
        // Read keypad input and adjust duty cycle based on keypress
        if (GPIOPinRead(GPIO_PORTB_BASE, GPIO_PIN_3)){
            dutyCycle += 5;
        }
        if (GPIOPinRead(GPIO_PORTB_BASE, GPIO_PIN_2)){
            dutyCycle -= 5;
        }
        dutyCycle = dutyCycle % 100;
        
        PWMPulseWidthSet(PWM0_BASE, PWM_OUT_0, ulPeriod * dutyCycle / 100);

        //Enable the PWM generator.
        if(panelDeploy||panelRetract){
            PWMGenEnable(PWM0_BASE, PWM_GEN_0);
        }
        
        else if(panelDone){
            PWMPulseWidthSet(PWM0_BASE, PWM_OUT_0, 0);
            PWMGenEnable(PWM0_BASE, PWM_GEN_0);
        }    
        vTaskDelay(100);
    }
}