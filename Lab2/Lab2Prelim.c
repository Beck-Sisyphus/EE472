// Include Statements
#include <stdlib.h>
#include <stdint.h>
#include "lab2.h"
#include "inc/hw_gpio.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
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
unsigned short battLevel;
uint32_t fuelLevel;
unsigned short powerConsumption;
unsigned short powerGeneration;
Bool panelState;
//16bit encoded thrust command [15:8]Duration,[7:4]Magnitude,[3:0]Direction
uint16_t thrust; 
Bool fuelLow;
Bool battLow;
Bool isMajorCycle;

// Global variable created for passing data through different function safer
unsigned short globalCount;
unsigned short blinkTimer;
uint32_t fuelLevellll;

int main(){
	// Initialize SysClk.
	SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN |
		SYSCTL_XTAL_8MHZ);
        
	// Initialize the OLED display.
	RIT128x96x4Init(1000000);

	// Initialization 
	battLevel = MAX_BATT_LEVEL;
	fuelLevel = MAX_FUEL_LEVEL;
	powerConsumption = 0;
	powerGeneration = 0;
	panelState = FALSE;
	thrust = 0;
	fuelLow = FALSE;
	battLow = FALSE;
	isMajorCycle = TRUE;
	globalCount = 0;
	blinkTimer = 0;
        
 
	// Define Data Structs
	powerSubDataStruct powerSubData           = {&panelState, &battLevel, &powerConsumption, &powerGeneration, &globalCount, &isMajorCycle};
	thrusterSubDataStruct thrusterSubData     = {&thrust, &fuelLevel, &globalCount, &isMajorCycle};
	satelliteCommsDataStruct satelliteCommsData     = {&fuelLow, &battLow, &panelState, &battLevel, &fuelLevel, &powerConsumption, &powerGeneration, &thrust, &globalCount, &isMajorCycle};
	oledDisplayDataStruct oledDisplayData     = {&fuelLow, &battLow, &panelState, &battLevel, &fuelLevel, &powerConsumption, &powerGeneration, &globalCount, &isMajorCycle};
	warningAlarmDataStruct warningAlarmData   = {&fuelLow, &battLow, &battLevel, &fuelLevel, &globalCount, &isMajorCycle};
	scheduleDataStruct scheduleData           = {&globalCount, &isMajorCycle};

	// Define TCBs
	TCB powerSubTCB;
	TCB thrusterSubTCB;
	TCB satelliteCommsTCB;
	TCB oledDisplayTCB;
	TCB warningAlarmTCB;
	TCB* TCBptr; //ptr to active TCB

	// Populate TCBs
	powerSubTCB.taskDataPtr = (void*)&powerSubData;
	powerSubTCB.taskPtr = powerSub;

	thrusterSubTCB.taskDataPtr = (void*)&thrusterSubData;
	thrusterSubTCB.taskPtr = thrusterSub;

	satelliteCommsTCB.taskDataPtr = (void*)&satelliteCommsData;
	satelliteCommsTCB.taskPtr = satelliteComms;

	oledDisplayTCB.taskDataPtr = (void*)&oledDisplayData;
	oledDisplayTCB.taskPtr = oledDisplay;

	warningAlarmTCB.taskDataPtr = (void*)&warningAlarmData;
	warningAlarmTCB.taskPtr = warningAlarm;


	// Task Queue (Array of Structs with length of 6)
	TCB* taskQueue[6];

	taskQueue[0] = &powerSubTCB;
	taskQueue[2] = &thrusterSubTCB;
	taskQueue[1] = &satelliteCommsTCB;
	taskQueue[3] = &oledDisplayTCB;
	taskQueue[4] = &warningAlarmTCB;

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

			
	// clear green LED		
	GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_5, 0x00);		
	// clear yellow LED		
	GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_6, 0x00);		
	// clear red LED		
	GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_7, 0x00);

    // Run... forever!!!
    while(1){
    	// dispatch each task in turn
    	// can not use a for loop nested in while loop for cross compile
    	TCBptr = taskQueue[0];
    	TCBptr->taskPtr( (TCBptr->taskDataPtr) );
    	TCBptr = taskQueue[1];
    	TCBptr->taskPtr( (TCBptr->taskDataPtr) );
    	TCBptr = taskQueue[2];
    	TCBptr->taskPtr( (TCBptr->taskDataPtr) );
    	TCBptr = taskQueue[3];
    	TCBptr->taskPtr( (TCBptr->taskDataPtr) );
    	TCBptr = taskQueue[4];
    	TCBptr->taskPtr( (TCBptr->taskDataPtr) );
    	schedule(scheduleData);
    }
    return EXIT_SUCCESS;
}