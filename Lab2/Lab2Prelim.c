//TODO Include Statements...
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/sysctl.h"
// #include "drivers/rit128x96x4.h"
#include "lab2.h"
#include "funct.c"


//TODO Global Variables...


//define some constants
/*#define HALF_WARN_LEVEL 50
#define FUEL_WARN_LEVEL 10
#define BATT_WARN_LEVEL 10
#define MAX_FUEL_LEVEL 100
#define MAX_BATT_LEVEL 100
#define TASK_QUEUE_LENGTH 6*/

//define some constants
const unsigned short HALF_WARN_LEVEL = 50;
const unsigned short FUEL_WARN_LEVEL = 10;
const unsigned short BATT_WARN_LEVEL = 10;
const unsigned short MAX_FUEL_LEVEL = 100;
const unsigned short MAX_BATT_LEVEL = 100;
const unsigned short TASK_QUEUE_LENGTH = 6;

//define and initialize global variables
unsigned short battLevel;// = MAX_BATT_LEVEL; // complains that these expressions must have a constant value
unsigned short fuelLevel;// = MAX_FUEL_LEVEL;
unsigned short powerConsumption = 0;
unsigned short powerGeneration = 0;
Bool panelState = FALSE;
uint16_t thrust = 0; //16bit encoded thrust command [15:8]Duration,[7:4]Magnitude,[3:0]Direction
Bool fuelLow = FALSE;
Bool battLow = FALSE;
unsigned short globalCount = 0;
Bool majorMinorCycle = 0;

//TODO Global Variable Definitions


int main(){
	//TODO Local Variable Declarations
	unsigned short left = 0;
	unsigned short right = 0;
	unsigned short up = 0;
	unsigned short down = 0;

	unsigned short motorDrive = 0;
        
        battLevel = MAX_BATT_LEVEL;
        fuelLevel = MAX_FUEL_LEVEL;

	//TODO Define Data Structs
	powerSubDataStruct powerSubData 	        = {&panelState, &battLevel, &powerConsumption, &powerGeneration};
	thrusterSubDataStruct thrusterSubData 		= {&thrust, &fuelLevel};
	satelliteCommsDataStruct satelliteCommsData     = {&fuelLow, &battLow, &panelState, &battLevel, &fuelLevel, &powerConsumption, &powerGeneration, &thrust};
	oledDisplayDataStruct oledDisplayData 		= {&fuelLow, &battLow, &panelState, &battLevel, &fuelLevel, &powerConsumption, &powerGeneration};
	warningAlarmDataStruct warningAlarmData 	= {&fuelLow, &battLow, &battLevel, &fuelLevel};

	//TODO Define TCBs
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
	//TCB* taskQueue[TASK_QUEUE_LENGTH];
        TCB* taskQueue[6];


	taskQueue[0] = &powerSubTCB;
	taskQueue[1] = &thrusterSubTCB;
	taskQueue[2] = &satelliteCommsTCB;
	taskQueue[3] = &oledDisplayTCB;
	taskQueue[4] = &warningAlarmTCB;

	//Run... forever!!!
	while(1){
		//dispatch each task in turn
		for (int i = 0; i < TASK_QUEUE_LENGTH - 1; ++i)
		{
                  printf("Global count: %d", globalCount);
			TCBptr = taskQueue[i];
			TCBptr->taskPtr( (TCBptr->taskDataPtr) );
		}
		schedule();
	}

	return EXIT_SUCCESS;
}
