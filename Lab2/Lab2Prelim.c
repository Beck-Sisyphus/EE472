//TODO Include Statements...
#include <stdio.h>
// #include "inc/hw_types.h"
// #include "driverlib/debug.h"
// #include "driverlib/sysctl.h"
// #include "drivers/rit128x96x4.h"
#include <stdint.h>
#include "lab2.h"


//TODO Global Variables...


//define some constants
const int HALF_WARN_LEVEL = 50;
const int FUEL_WARN_LEVEL = 10;
const int BATT_WARN_LEVEL = 10;
const int MAX_FUEL_LEVEL = 100;
const int MAX_BATT_LEVEL = 100;
const int TASK_QUEUE_LENGTH = 6;

//define and initiallize global variables
unsigned short battLevel = MAX_BATT_LEVEL;
unsigned short fuelLevel = MAX_FUEL_LEVEL;
unsigned short powerConsumption = 0;
unsigned short powerGeneration = 0;
Bool panelState = FALSE;
unsigned int thrust = 0; //16bit encoded thrust command [15:8]Duration,[7:4]Magnitude,[3:0]Direction
Bool fuelLow = FALSE;
Bool battLow = FALSE;

//TODO Global Variable Definitions


int main(){
	//TODO Local Variable Declarations
	unsigned short left = 0;
	unsigned short right = 0;
	unsigned short up = 0;
	unsigned short down = 0;

	unsigned short motorDrive = 0;

	//TODO Define Data Structs
	powerSubDataStruct powerSubData;
	thrusterSubDataStruct thrusterSubData;
	satelliteCommsDataStruct satelliteCommsData;
	oledDisplayDataStruct oledDisplayData;
	warningAlarmDataStruct warningAlarmData;

	//TODO Populate Data Structs
	powerSubData = (&panelState, &battLevel, &powerConsumption, &powerGeneration);
	thrusterSubData = (&thrust, &fuelLevel);
	satelliteCommsData = (&fuelLow, &battLow, &panelState, &battLevel, &fuelLevel, &powerConsumption, &powerGeneration, &thrust);
	oledDisplayData = (&fuelLow, &battLow, &panelState, &battLevelP, &fuelLevel, &powerConsumption, &powerGeneration);
	warningAlarmData = (&fuelLow, &battLow, &battLevel, &fuelLevel);

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
	TCB* taskQueue[TASK_QUEUE_LENGTH];

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
			TCBptr = taskQueue[i];

		}

		//delay(localCounter);
	}
}