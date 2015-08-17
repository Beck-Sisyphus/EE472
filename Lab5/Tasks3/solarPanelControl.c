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

void solarPanelControl(void* taskDataPtr)
{
    solarPanelStruct* solarPanelPtr = (solarPanelStruct*) taskDataPtr;
    Bool* isMajorCycle = (Bool*) solarPanelPtr->isMajorCyclePtr;
    Bool* panelState = (Bool*) solarPanelPtr->panelStatePtr;
    Bool* panelDeploy = (Bool*) solarPanelPtr->panelDeployPtr;
    Bool* panelRetract = (Bool*) solarPanelPtr->panelRetractPtr;
    Bool* panelMotorSpeedUp = (Bool*) solarPanelPtr->panelMotorSpeedUpPtr;
    Bool* panelMotorSpeedDown = (Bool*) solarPanelPtr->panelMotorSpeedDownPtr;
    
    while(1)
    {
        // Compute the PWM period based on the system clock.
        // Base clock 8MHz, want 2Hz for panel motor ( / 4000000)
        unsigned long ulPeriod = SysCtlClockGet() / 80; //run at 2Hz
        static unsigned long dutyCycle = 5;
        
        // Read keypad input and adjust duty cycle based on keypress
        if (*panelMotorSpeedUp){
            dutyCycle += 5;
            *panelMotorSpeedUp = FALSE;
        }
        if (*panelMotorSpeedDown){
            dutyCycle -= 5;
            *panelMotorSpeedDown = FALSE;
        }
        dutyCycle = dutyCycle % 100;
        
        SysCtlPWMClockSet(SYSCTL_PWMDIV_64); //SYSCTL_PWMDIV_32
    
        //Enable PWM and GPIO pins to carry signal
        SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);
        SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
        
        //Set GPIO pins F0 and G1 as output PWM
        GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_0);
        
        PWMGenPeriodSet(PWM0_BASE, PWM_GEN_0, ulPeriod);
        PWMPulseWidthSet(PWM0_BASE, PWM_OUT_0, ulPeriod);
        PWMGenConfigure(PWM0_BASE, PWM_GEN_0,
                    PWM_GEN_MODE_UP_DOWN | PWM_GEN_MODE_NO_SYNC);
        PWMOutputState(PWM0_BASE, PWM_OUT_0_BIT | PWM_OUT_1_BIT, true);

        //Enable the PWM generator.
        if(*panelDeploy||*panelRetract){
            PWMPulseWidthSet(PWM0_BASE, PWM_OUT_0, ulPeriod * dutyCycle / 100);
            PWMGenEnable(PWM0_BASE, PWM_GEN_0);
        }
        
        else if(panelDone){
            PWMPulseWidthSet(PWM0_BASE, PWM_OUT_0, 0);
            PWMGenEnable(PWM0_BASE, PWM_GEN_0);
        }    
        
        vTaskDelay(100);
    }
}