// #include <stdio.h>
// #include <stdlib.h>
#include "lab3.h"
#include "inc/hw_gpio.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/lm3s8962.h"
#include "driverlib/adc.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pwm.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"
#include "driverlib/uart.h"
#include "drivers/rit128x96x4.h"
#include "utils/ustdlib.h"

const int SLOW_CLOCK_RATE = 10000;
const int MINOR_CYCLE_NUM = 100; // Change the ratio of major / minor cycle

extern unsigned char vehicleCommand;
extern unsigned char vehicleResponse[3];
extern Bool hasNewKeyboardInput;
extern Bool panelDone;
extern unsigned short globalCount;
extern unsigned int* battLevelPtr;

void enableOLED()
{
    // Initialize SysClk.
    SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN |
        SYSCTL_XTAL_8MHZ);
        
    // Initialize the OLED display.
    RIT128x96x4Init(1000000);
}

void enableGPIO()
{
    // Enable GPIO C
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
    // Set pins C4, C5, C6, C7 as an output
    GPIOPinTypeGPIOOutput(GPIO_PORTC_BASE, GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|
                          GPIO_PIN_7);

    // Set select button as input with a pull up resistor 
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

    GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_1);
        
    GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_STRENGTH_2MA,
                         GPIO_PIN_TYPE_STD_WPU);
        
    
    // Set GPIO Pins D4 and D5 for input from keypad for PWM control
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    GPIOPinTypeGPIOInput(GPIO_PORTD_BASE, GPIO_PIN_4|GPIO_PIN_5);
        GPIODirModeSet(GPIO_PORTD_BASE, GPIO_PIN_4|GPIO_PIN_5, GPIO_DIR_MODE_IN);
        //GPIOPadConfigSet(GPIO_PORTD_BASE, GPIO_PIN_6|GPIO_PIN_7, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU); 
    
    
    // Enable keypad pannel done button
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    GPIOPinTypeGPIOInput(GPIO_PORTA_BASE, GPIO_PIN_4);
    GPIODirModeSet(GPIO_PORTA_BASE, GPIO_PIN_4, GPIO_DIR_MODE_IN);
    GPIOPadConfigSet(GPIO_PORTA_BASE, GPIO_PIN_4, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU); 
    GPIOPinIntEnable(GPIO_PORTA_BASE, GPIO_PIN_4);
    IntEnable(INT_GPIOA);
        
    // clear green LED      
    GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_5, 0x00);        
    // clear yellow LED     
    GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_6, 0x00);        
    // clear red LED        
    GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_7, 0x00);
        
    
    //Enable PWM
    SysCtlPWMClockSet(SYSCTL_PWMDIV_64); //SYSCTL_PWMDIV_32
    
    //Enable PWM and GPIO pins to carry signal
    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    
    //Set GPIO pins F0 and G1 as output PWM
    GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_0);

}

void enableADC() 
{
    // Enable ADC0
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    // Enable sample sequence 0 to be triggered on external
    // TODO configure proper trigger for battery
    ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_PROCESSOR, 0); //
    // Configure step 0 on sequence 3
    ADCSequenceStepConfigure(ADC0_BASE, 3, 0, ADC_CTL_CH0 | ADC_CTL_IE |
                             ADC_CTL_END);
    // Enable sequence 3
    ADCSequenceEnable(ADC0_BASE, 3);
        ADCIntEnable(ADC0_BASE, 3);

}

void enableUART()
{
    // Enable peripheral used for UART
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    // Enable processor interrupt
    IntMasterEnable();

    // Set GPIO A0 and A1 as UART pins Rx and Tx
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    // Configure the UART with BAUD rate of 115200, 8-N-1 operation.
    UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 115200, 
                        (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | 
                            UART_CONFIG_PAR_NONE));

    // Enable the UART interrupt.
    IntEnable(INT_UART0);
    UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT);

    // Remind the user to send data
    UARTSend((unsigned char *)"Enter text: ", 12); // TODO DEBUG enters fault ISR here
}

void enableTimer() {

    // Enable HW Timer
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
        
    // Configure the 32-bit periodic timer.
    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
    TimerLoadSet(TIMER0_BASE, TIMER_A, SysCtlClockGet()/SLOW_CLOCK_RATE);

    IntEnable(INT_TIMER0A);
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    //
    // Enable the timers.
    //
    TimerEnable(TIMER0_BASE, TIMER_A);
}

//*****************************************************************************
//
// Source from uart_echo.c
// 
// Send a string to the UART.
//
//*****************************************************************************
void
UARTSend(const unsigned char *pucBuffer, unsigned long ulCount)
{
    //
    // Loop while there are more characters to send.
    //
    while(ulCount--)
    {
        //
        // Write the next character to the UART.
        //
        UARTCharPut(UART0_BASE, *pucBuffer++);
    }
}

//*****************************************************************************
//
// The UART interrupt handler.
//
//*****************************************************************************
void
UARTIntHandler(void)
{
    unsigned long ulStatus;
    
    //
    // Get the interrrupt status.
    //
    ulStatus = UARTIntStatus(UART0_BASE, true);
    
    //
    // Clear the asserted interrupts.
    //
    UARTIntClear(UART0_BASE, ulStatus);

    // Receive command
    if (!hasNewKeyboardInput) 
    {
        if (UARTCharsAvail(UART0_BASE))
        {
            vehicleCommand = UARTCharGetNonBlocking(UART0_BASE);
            
            vehicleResponse[2] = vehicleCommand;
            
            hasNewKeyboardInput = TRUE;
        }
    }
}

void IntGPIOa(void)
{
    GPIOPinIntClear(GPIO_PORTA_BASE, GPIO_PIN_4);

    panelDone = TRUE;
}

//*****************************************************************************
//
// The UART interrupt handler.
//
//*****************************************************************************
void
Timer0IntHandler(void)
{
    //
    // Clear the timer interrupt.
    //
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    //
    // Update the interrupt status on the display.
    //
    IntMasterDisable();

    IntMasterEnable();
    globalCount++; 
    globalCount = globalCount % MINOR_CYCLE_NUM;
}


void 
ADCIntHandler(void) 
{
/*  unsigned int* battLevel = (unsigned int*) battLevelPtr; // Points to address of battLevelPtr[0]

    // Battery measurement
    // following interrupt:
    // delay 600us
    delay_ms(100); // TODO determine correct value for 600us
    // Below code for ADC measurement adapted from single_ended.c
    //  in IAR example file
    // Clear interrupt status flag before sampling
    ADCIntClear(ADC0_BASE, 3);
    // Trigger the ADC conversion
    ADCProcessorTrigger(ADC0_BASE, 3);

    // Wait for conversion to be completed
    while(!ADCIntStatus(ADC0_BASE, 3, false))
    {
    }

    // Clear the ADC interrupt flag
    ADCIntClear(ADC0_BASE, 3);
    // Create array to hold ADC value
    unsigned int adcReading[1] = {0}; // TODO ADC returns long or int?
    // Read ADC Value
    ADCSequenceDataGet(ADC0_BASE, 3, adcReading);

    // convert adcReading from 4.25V to 36V scale
    // If ADC returns 10bit int (0-1023), each digit ~= 0.00415V
    // Then, multiply by 8.4706 to get to 36V range
    unsigned int adcReadingConverted = adcReading[0] * 0.00415 * 8.4706;
    // move previous readings to next array slot
    for (int i = sizeof(battLevel) - 2; i > 0; --i)
    {
        battLevel[i+1] = battLevel[i];
    }
    // Add new reading to front of circular buffer
    battLevel[0] = adcReadingConverted;*/
    return;
}
