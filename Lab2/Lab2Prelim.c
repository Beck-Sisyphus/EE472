//Include Statements...
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "lab2.h"
#include "inc/hw_gpio.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"
#include "drivers/rit128x96x4.h"
#include "inc/lm3s8962.h"
#include "utils/ustdlib.h"

//Define some Constants...
const unsigned short HALF_WARN_LEVEL = 50;
const unsigned short FUEL_WARN_LEVEL = 10;
const unsigned short BATT_WARN_LEVEL = 10;
const unsigned short MAX_FUEL_LEVEL = 100;
const unsigned short MAX_BATT_LEVEL = 100;
const unsigned short TASK_QUEUE_LENGTH = 6;

//Define and Initialize Global Variables
unsigned short battLevel;
uint32_t fuelLevel = 12441600; // 6 month has 6*30*24*60*60 seconds, and 
unsigned short powerConsumption = 0;
unsigned short powerGeneration = 0;
Bool panelState = FALSE;
uint16_t thrust = 0; //16bit encoded thrust command [15:8]Duration,[7:4]Magnitude,[3:0]Direction
Bool fuelLow = FALSE;
Bool battLow = FALSE;
Bool isMajorCycle = TRUE;
unsigned short globalCount = 0;

int main(){
	//TODO Local Variable Declarations
  
	unsigned short motorDrive = 0;
        
        battLevel = MAX_BATT_LEVEL;
        fuelLevel = MAX_FUEL_LEVEL;

	//Define Data Structs
	powerSubDataStruct powerSubData 	        = {&panelState, &battLevel, &powerConsumption, &powerGeneration, &globalCount, &isMajorCycle};
	thrusterSubDataStruct thrusterSubData 		= {&thrust, &fuelLevel, &globalCount, &isMajorCycle};
	satelliteCommsDataStruct satelliteCommsData     = {&fuelLow, &battLow, &panelState, &battLevel, &fuelLevel, &powerConsumption, &powerGeneration, &thrust, &globalCount, &isMajorCycle};
	oledDisplayDataStruct oledDisplayData 		= {&fuelLow, &battLow, &panelState, &battLevel, &fuelLevel, &powerConsumption, &powerGeneration, &globalCount, &isMajorCycle};
	warningAlarmDataStruct warningAlarmData 	= {&fuelLow, &battLow, &battLevel, &fuelLevel, &globalCount, &isMajorCycle};
        scheduleDataStruct scheduleData 	        = {&globalCount, &isMajorCycle};

	//Define TCBs
	TCB powerSubTCB;
	TCB thrusterSubTCB;
	TCB satelliteCommsTCB;
	TCB oledDisplayTCB;
	TCB warningAlarmTCB;
	TCB* TCBptr; //ptr to active TCB

	//TODO Populate TCBs
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


	//TODO Task Queue (Array of Structs)
	TCB* taskQueue[6];

	taskQueue[0] = &powerSubTCB;
	taskQueue[1] = &thrusterSubTCB;
	taskQueue[2] = &satelliteCommsTCB;
	taskQueue[3] = &oledDisplayTCB;
	taskQueue[4] = &warningAlarmTCB;

    SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN |
                   SYSCTL_XTAL_8MHZ);

    // Enable GPIO C
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
        
    // Set pins C4, C5, C6, C7 as an output
    GPIOPinTypeGPIOOutput(GPIO_PORTC_BASE, GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|
                          GPIO_PIN_7);

    //Run... forever!!!
    while(1){
            //dispatch each task in turn
            for (int i = 0; i < TASK_QUEUE_LENGTH - 1; ++i)
            {
        //printf("Global count: %d \n", globalCount);
                    TCBptr = taskQueue[i];
                    TCBptr->taskPtr( (TCBptr->taskDataPtr) );
            }
            schedule(scheduleData);
    }
    return EXIT_SUCCESS;
}
