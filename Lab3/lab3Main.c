// Include Statements
#include <stdlib.h>
#include <stdint.h>
#include "lab3.h"
#include "inc/hw_gpio.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/adc.h"
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

// Global variable created for passing data through different function safer
unsigned short globalCount;
unsigned short blinkTimer;
uint32_t fuelLevellll;

int main(){
	enableOLED();
	enableGPIO();
	enableADC();
	initializeGlobalVariables();

	// Define Data Structs
	powerSubDataStruct powerSubData           = {&panelState, &panelDeploy, &panelRetract, &battLevelPtr, &powerConsumption, &powerGeneration};
	solarPanelStruct solarPanelData           = {&panelState, &panelDeploy, &panelRetract, &panelMotorSpeedUp, &panelMotorSpeedDown, &globalCount, &isMajorCycle};
	satelliteCommsDataStruct satelliteCommsData     = {&fuelLow, &battLow, &panelState, &battLevelPtr, &fuelLevel, &powerConsumption, &powerGeneration, &thrust, &globalCount, &isMajorCycle};
	thrusterSubDataStruct thrusterSubData     = {&thrust, &fuelLevel, &globalCount, &isMajorCycle};
	vehicleCommsStruct vehicleCommsData         = {&vehicleCommand, &vehicleResponse, &globalCount, &isMajorCycle};
	oledDisplayDataStruct oledDisplayData     = {&fuelLow, &battLow, &panelState, &battLevelPtr, &fuelLevel, &powerConsumption, &powerGeneration, &globalCount, &isMajorCycle};
	keyboardDataStruct keyboardData           = {&panelMotorSpeedUp, &panelMotorSpeedDown};
	warningAlarmDataStruct warningAlarmData   = {&fuelLow, &battLow, &battLevelPtr, &fuelLevel, &globalCount, &isMajorCycle};
	scheduleDataStruct scheduleData           = {&globalCount, &isMajorCycle};

	// Define TCBs
	TCB powerSubTCB;
	TCB solarPanelTCB;
	TCB satelliteCommsTCB;
	TCB thrusterSubTCB;
	TCB vehicleCommsTCB;
	TCB oledDisplayTCB;
	TCB keyboardDataTCB;
	TCB warningAlarmTCB;
	TCB* TCBptr; //ptr to active TCB

	// Populate TCBs
	powerSubTCB.taskDataPtr = (void*)&powerSubData;
	powerSubTCB.taskPtr = powerSub;

	solarPanelTCB.taskDataPtr = (void*)&solarPanelData;
	solarPanelTCB.taskPtr = solarPanelControl;

	satelliteCommsTCB.taskDataPtr = (void*)&satelliteCommsData;
	satelliteCommsTCB.taskPtr = satelliteComms;

	thrusterSubTCB.taskDataPtr = (void*)&thrusterSubData;
	thrusterSubTCB.taskPtr = thrusterSub;

	vehicleCommsTCB.taskDataPtr = (void*)&vehicleCommsData;
	vehicleCommsTCB.taskPtr = vehicleComms;

	oledDisplayTCB.taskDataPtr = (void*)&oledDisplayData;
	oledDisplayTCB.taskPtr = oledDisplay;

	keyboardDataTCB.taskDataPtr = (void*)&keyboardData;
	keyboardDataTCB.taskPtr = consoleKeyboard;

	warningAlarmTCB.taskDataPtr = (void*)&warningAlarmData;
	warningAlarmTCB.taskPtr = warningAlarm;


	// Task Queue 
	TCB* taskQueue[8];

	taskQueue[0] = &powerSubTCB;
	taskQueue[1] = &solarPanelTCB;
	taskQueue[2] = &satelliteCommsTCB;
	taskQueue[3] = &thrusterSubTCB;
	taskQueue[4] = &vehicleCommsTCB;
	taskQueue[5] = &oledDisplayTCB;
	taskQueue[6] = &keyboardDataTCB;
	taskQueue[7] = &warningAlarmTCB;

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
        TCBptr = taskQueue[5];
        TCBptr->taskPtr( (TCBptr->taskDataPtr) );
        TCBptr = taskQueue[6];
        TCBptr->taskPtr( (TCBptr->taskDataPtr) );
        TCBptr = taskQueue[7];
        TCBptr->taskPtr( (TCBptr->taskDataPtr) );
    	schedule(scheduleData);
    }
    return EXIT_SUCCESS;
}

void enableOLED() {
	// Initialize SysClk.
	SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN |
		SYSCTL_XTAL_8MHZ);
        
	// Initialize the OLED display.
	RIT128x96x4Init(1000000);
}

void enableGPIO() {
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

	// Set GPIO for ADC
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
	GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_7);
			
	// clear green LED		
	GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_5, 0x00);		
	// clear yellow LED		
	GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_6, 0x00);		
	// clear red LED		
	GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_7, 0x00);
}

void enableADC() 
{
	// Enable ADC0
	SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
	// Set sampling rate to lowest rate, 125K/s
	SysCtlADCSpeedSet(SYSCTL_ADCSPEED_125KSPS);
	// Enable sample sequence 0 to be triggered on external
	// TODO configure proper trigger for battery
	ADCSequenceConfigure(ADC0_BASE, 0, ADC_TRIGGER_EXTERNAL, 0);
	// Configure step 0 on sequence 0
	ADCSequenceStepConfigure(ADC0_BASE, 0, 0, ADC_CTL_CH0 | ADC_CTL_IE |
                             ADC_CTL_END);
	// Enable sequence 0
	ADCSequenceEnable(ADC0_BASE, 0);

}

void initializeGlobalVariables() {
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