#include <stdio.h>
#include "lab2.h"

//TODO Define all Functions Here

//define some constants
extern const int HALF_WARN_LEVEL;
extern const int FUEL_WARN_LEVEL;
extern const int BATT_WARN_LEVEL;
extern const int MAX_FUEL_LEVEL;
extern const int MAX_BATT_LEVEL;
extern const int TASK_QUEUE_LENGTH;

//define and initiallize global variables
extern unsigned short battLevel;
extern unsigned short fuelLevel;
extern unsigned short powerConsumption;
extern unsigned short powerGeneration;
extern Bool panelState;
extern unsigned int thrust; //16bit encoded thrust command [15:8]Duration,[7:4]Magnitude,[3:0]Direction
extern Bool fuelLow;
extern Bool battLow;

//TODO
void schedule(){

}

//TODO
void powerSub(void* taskDataPtr){

	//powerConsumption
	static unsigned short runCount = 1;				//tracks even/odd calls of this function

	runCount = (runCount + 1) % 2;					//alternates between 1 and 0 for odd/een calls respectively
	if (*(taskDataPtr->powerConsumptionPtr)<10)     //TODO, does this work???
		if (runCount==0){							//on even calls...
			*(taskDataPtr->powerConsumptionPtr) += 2;		//increment by 2
		}
		else{										//on odd calls...
			*(taskDataPtr->powerConsumptionPtr) -= 1;		//decrement by 1
		}
	}
	else{
		if (runCount==0){							//on even calls...
			*(taskDataPtr->powerConsumptionPtr) -= 2;	//decrement by 2
		}
		else{										//on odd calls...
			*(taskDataPtr->powerConsumptionPtr) += 1;		//increment by 1
	}


	//powerGeneration
	if (*(taskDataPtr->panelStatePtr)==1){			//if solar panel is deployed...
		if (*(taskDataPtr->battLevelPtr)<95){		//if battery greater than 95%
			*(taskDataPtr->panelStatePtr)=0;		//retract solar panel
		}
		else{										//else battery less than/equal to 95%
			if (runCount==0){							//on even calls...
			*(taskDataPtr->powerGenerationPtr) += 2;		//increment by 2
			}
			else if(taskDataPtr->battLevelPtr)<=50){	//on odd calls... while battery less than/equal to 50%
			*(taskDataPtr->powerGenerationPtr) += 1;		//increment by 1
			}
		}
	}
	else											//else solar panel not deployed...
		if (*(taskDataPtr->battLevelPtr)<=10){		//if battery less than/equal to 10%
			*(taskDataPtr->panelStatePtr)=1;		//deploy solar panel
		}
	}


	//batteryLevel
	if (*(taskDataPtr->panelStatePtr)==0){
		(*(taskDataPtr->battLevelPtr)) = (*(taskDataPtr->battLevelPtr)) - 3*(*(taskDataPtr->powerConsumptionPtr));
	}
	else{
		(*(taskDataPtr->battLevelPtr)) = (*(taskDataPtr->battLevelPtr)) - (*(taskDataPtr->powerConsumptionPtr)) + (*(taskDataPtr->powerGenerationPtr));
	}
}

//TODO
void thrusterSub(void* taskDataPtr){
	//TODO concatenate signals into control command

	//TODO figure out fuel consumption
}

//TODO
void satelliteComms(void* taskDataPtr){
	//TODO send info

	//TODO receive (rando) thrust commands
		//TODO implement rand num gen
}

//TODO
void oledDisplay(void* taskDataPtr){
	//TODO two modes??
}

void warningAlarm(void* taskDataPtr){

	if ((battLevel<BATT_WARN_LEVEL))&(fuelLevel>HALF_WARN_LEVEL){
		//TODO solid green
	}
	else{
		if (battLevel<BATT_WARN_LEVEL){
			//TODO flash red 1 sec
		}
		else if (fuelLevel<FUEL_WARN_LEVEL){
			//TODO flash red 2 sec
		}
		if (battLevel<BATT_WARN_LEVEL){
			//TODO flash red 1 sec
		}
		else if (fuelLevel<FUEL_WARN_LEVEL){
			//TODO flash red 2 sec
		}
	}
	return;
}

void delay_ms(int time_in_ms){
	//TODO put stuff in here
}