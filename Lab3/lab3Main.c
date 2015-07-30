// Include Statements
#include <stdlib.h>
#include <stdint.h>
#include "lab3.h"
#include "inc/hw_gpio.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "driverlib/adc.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "drivers/rit128x96x4.h"

// Define some Constants
const unsigned short MAX_BATT_LEVEL = 100;
const unsigned short HALF_BATT_LEVEL = 50;
const unsigned short BATT_WARN_LEVEL = 10;
// 6 month has x = 6*30*24*60*60 seconds, and 100% has y = x/20 seconds, 
// since the magnitude command has 4 bit, aka the consumption ranks from 0 to 15, 
// setting a total fuel level as 15 * y helps to calculate the fuel easier
// Assuming the minor cycle is running at 1 second per cycle rate.
const uint32_t MAX_FUEL_LEVEL = 11664000;
const uint32_t HALF_FUEL_LEVEL = 5832000; // at 50% level
const uint32_t FUEL_WARN_LEVEL = 1166400; // below 10% is warnning level
const unsigned short TASK_QUEUE_LENGTH = 6;

// Define Global Variables storing status data
unsigned int* battLevelPtr;
uint32_t fuelLevel;
unsigned short powerConsumption;
unsigned short powerGeneration;
Bool panelState;
Bool panelDeploy;
Bool panelRetract;
Bool panelMotorSpeedUp;
Bool panelMotorSpeedDown;
// 16bit encoded thrust command [15:8]Duration,[7:4]Magnitude,[3:0]Direction
uint16_t thrust; 
Bool fuelLow;
Bool battLow;
Bool isMajorCycle;

// Added for lab 3
char vehicleCommand;
char vehicleResponse;
// If true, task should be inserted into task queue if not already present
// If false, task should be removed from task queue if present
// Keypad task is also tied
Bool panelAndKeypadTask;

// Global variable created for passing data through different function safer
unsigned short globalCount;
unsigned short blinkTimer;
uint32_t fuelLevellll;

int main()
{
	enableOLED();
	enableGPIO();
	enableADC();
	enableUART();
	initializeGlobalVariables();

	// Define Data Structs
	powerSubDataStruct powerSubData           	= {&panelState, &panelDeploy, &panelRetract, &battLevelPtr, &powerConsumption, &powerGeneration};
	solarPanelStruct solarPanelData           	= {&panelState, &panelDeploy, &panelRetract, &panelMotorSpeedUp, &panelMotorSpeedDown, &globalCount, &isMajorCycle};
	satelliteCommsDataStruct satelliteCommsData	= {&fuelLow, &battLow, &panelState, &battLevelPtr, &fuelLevel, &powerConsumption, &powerGeneration, &thrust, &globalCount, &isMajorCycle};
	thrusterSubDataStruct thrusterSubData     	= {&thrust, &fuelLevel, &globalCount, &isMajorCycle};
	vehicleCommsStruct vehicleCommsData       	= {&vehicleCommand, &vehicleResponse, &globalCount, &isMajorCycle};
	oledDisplayDataStruct oledDisplayData     	= {&fuelLow, &battLow, &panelState, &battLevelPtr, &fuelLevel, &powerConsumption, &powerGeneration, &globalCount, &isMajorCycle};
	keyboardDataStruct keyboardData           	= {&panelMotorSpeedUp, &panelMotorSpeedDown};
	warningAlarmDataStruct warningAlarmData   	= {&fuelLow, &battLow, &battLevelPtr, &fuelLevel, &globalCount, &isMajorCycle};
	scheduleDataStruct scheduleData           	= {&globalCount, &isMajorCycle};

	// Define TCBs
	TCB powerSubTCB;
	TCB solarPanelTCB;
	TCB satelliteCommsTCB;
	TCB thrusterSubTCB;
	TCB vehicleCommsTCB;
	TCB oledDisplayTCB;
	TCB keyboardDataTCB;
	TCB warningAlarmTCB;
	TCB* TCBPtr; //ptr to active TCB

	// Populate TCBs
	powerSubTCB.taskDataPtr = (void*)&powerSubData;
	powerSubTCB.taskPtr = powerSub;
	powerSubTCB.next = NULL;
	powerSubTCB.prev = NULL;

	solarPanelTCB.taskDataPtr = (void*)&solarPanelData;
	solarPanelTCB.taskPtr = solarPanelControl;
	solarPanelTCB.next = NULL;
	solarPanelTCB.prev = NULL;

	satelliteCommsTCB.taskDataPtr = (void*)&satelliteCommsData;
	satelliteCommsTCB.taskPtr = satelliteComms;
	satelliteCommsTCB.next = NULL;
	satelliteCommsTCB.prev = NULL;

	thrusterSubTCB.taskDataPtr = (void*)&thrusterSubData;
	thrusterSubTCB.taskPtr = thrusterSub;
	thrusterSubTCB.next = NULL;
	thrusterSubTCB.prev = NULL;

	vehicleCommsTCB.taskDataPtr = (void*)&vehicleCommsData;
	vehicleCommsTCB.taskPtr = vehicleComms;
	vehicleCommsTCB.next = NULL;
	vehicleCommsTCB.prev = NULL;

	oledDisplayTCB.taskDataPtr = (void*)&oledDisplayData;
	oledDisplayTCB.taskPtr = oledDisplay;
	oledDisplayTCB.next = NULL;
	oledDisplayTCB.prev = NULL;

	keyboardDataTCB.taskDataPtr = (void*)&keyboardData;
	keyboardDataTCB.taskPtr = consoleKeyboard;
	keyboardDataTCB.next = NULL;
	keyboardDataTCB.prev = NULL;

	warningAlarmTCB.taskDataPtr = (void*)&warningAlarmData;
	warningAlarmTCB.taskPtr = warningAlarm;
	warningAlarmTCB.next = NULL;
	warningAlarmTCB.prev = NULL;


	// Task Queue head and tail pointers
	TCB* taskQueueHead = NULL;
	TCB* taskQueueTail = NULL;

	insertTask(&powerSubTCB, &taskQueueHead, &taskQueueTail);
	insertTask(&satelliteCommsTCB, &taskQueueHead, &taskQueueTail);
	insertTask(&thrusterSubTCB, &taskQueueHead, &taskQueueTail);
	insertTask(&vehicleCommsTCB, &taskQueueHead, &taskQueueTail);
	insertTask(&oledDisplayTCB, &taskQueueHead, &taskQueueTail);
	insertTask(&warningAlarmTCB, &taskQueueHead, &taskQueueTail);

    // Run... forever!!!
    while(1)
    {
    	// Get pointer to first task
    	TCBPtr = taskQueueHead;
    	// Loop through task queue & perform each task
    	while (TCBPtr->next != NULL && TCBPtr->next != taskQueueHead)
    	{
	        TCBPtr->taskPtr( (TCBPtr->taskDataPtr) );
	        TCBPtr = TCBPtr->next;
    	}

    	// Adds/deletes solarPanel and keypad task as necessary
    	if (panelAndKeypadTask)
    	{
    		insertTask(&solarPanelTCB, &taskQueueHead, &taskQueueTail);
    		insertTask(&keyboardDataTCB, &taskQueueHead, &taskQueueTail);
    	}
    	else 
    	{
    		deleteTask(&solarPanelTCB, &taskQueueHead, &taskQueueTail);
    		deleteTask(&keyboardDataTCB, &taskQueueHead, &taskQueueTail);
    	}

    	schedule(scheduleData);
    }
    return EXIT_SUCCESS;
}

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
        
    
    // Set GPIO Pins A0 and A1 for input from keypad
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
	GPIOPinTypeGPIOInput(GPIO_PORTA_BASE, GPIO_PIN_0|GPIO_PIN_1);
	GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_0|GPIO_PIN_1, GPIO_STRENGTH_2MA,
	                     GPIO_PIN_TYPE_STD_WPU);
        
        
	// clear green LED		
	GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_5, 0x00);		
	// clear yellow LED		
	GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_6, 0x00);		
	// clear red LED		
	GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_7, 0x00);
        
    
    //Enable PWM
    SysCtlPWMClockSet(SYSCTL_PWMDIV_1);
    
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
	// Set sampling rate to lowest rate, 125K/s
	SysCtlADCSpeedSet(SYSCTL_ADCSPEED_125KSPS);
	// Enable sample sequence 3 to be triggered on external
	// TODO configure proper trigger for battery
	ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_PROCESSOR, 0);
	// Configure step 0 on sequence 3
	ADCSequenceStepConfigure(ADC0_BASE, 3, 0, ADC_CTL_CH0 | ADC_CTL_IE |
                             ADC_CTL_END);
	// Enable sequence 0
	ADCSequenceEnable(ADC0_BASE, 3);

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
	//IntEnable(INT_UART0);
	UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT);

	// Remind the user to send data
	//UARTSend((unsigned char *)"Enter text: ", 12); // TODO DEBUG enters fault ISR here
}

void initializeGlobalVariables()
{
	// Initialization 
    unsigned int batteryLevelArray[16] = {100, 0, 0, 0,
                                			0, 0, 0, 0, 
                               				0, 0, 0, 0, 
                               				0, 0, 0, 0};
	battLevelPtr = batteryLevelArray; // equivalent to = &battLevel[0]

	fuelLevel = 0;
	powerConsumption = 0;
	powerGeneration = 0;
	panelState = FALSE;
	panelDeploy = FALSE;
	panelRetract = FALSE;
	panelMotorSpeedUp = FALSE;
	panelMotorSpeedDown = FALSE;

	thrust = 0;
	fuelLow = FALSE;
	battLow = FALSE;

	// vehicleCommand ＝ NULL; Can't initialize as NULL, just left it as its default
	// vehicleResponse = NULL;

	isMajorCycle = TRUE;
	globalCount = 0;
	blinkTimer = 0;
}

void deleteTask(TCB* node, TCB** head, TCB** tail) 
{
	// Node is not in list
	if (NULL == node->next && NULL == node->prev)
	{
		return;
	}
	// List is empty
	else if (NULL == head)
	{
		return;
	} 
	// List has 1 node
	else if (*head == *tail)
	{
		// If node is this node, delete it
		if (*head == node)
		{
			*head = NULL;
			*tail = NULL;
		}
	}
	// Head node is node to delete
	else if (*head == node) 
	{
		*head = node->next;
		node->next = node->prev;
		node->prev = NULL;
		node->next = NULL;
	}
	// If tail node is node to delete
	else if (*tail == node)
	{
		*tail = node->prev;
		node->prev->next = NULL;
		node->prev = NULL;
	}
	// Node is in the middle, just need to update prevs and nexts of it
	//  and neighbor node(s)
	else
	{
		node->next->prev = node->prev;
		node->prev->next = node->next;
		node->next = NULL;
		node->prev = NULL;
	}
	return;
}

void insertTask(TCB* node, TCB** head, TCB** tail)
	{
	if(NULL == (*head)) // If the head pointer is pointing to nothing
	{
		*head = node; // set the head and tail pointers to point to this node
		*tail = node;
	}
	else // otherwise, head is not NULL, add the node to the end of the list
	{
		(*tail)->next = node;
		node->prev = *tail; // note that the tail pointer is still pointing
		// to the prior last node at this point
		*tail = node; // update the tail pointer
	}
	// Always set node next pointer to null for end of list
	node->next = NULL;
	return;
}

void ADCIntHandler(void) 
{
	unsigned int* battLevel = (unsigned int*) battLevelPtr; // Points to address of battLevelPtr[0]

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
    //while(!ADCIntStatus(ADC0_BASE, 3, false))
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
    battLevel[0] = adcReadingConverted;
	return;
}
