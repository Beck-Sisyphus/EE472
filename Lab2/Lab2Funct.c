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
	if (globalCount == 0){								//On first cycle of five...
		majorMinorCycle = 0;							//Execute a Major Cycle
	}
	else {												//On all other cycles...
		majorMinorCycle = 1;							//Execute a Minor Cycle
	}
	globalCount = globalCount + 1 % (TASK_QUEUE_LENGTH -1); //count to 5, then start over again
	delay_ms(1000);
}

//TODO
void powerSub(void* taskDataPtr){
	powerSubDataStruct* dataPtr = (powerSubDataStruct*) taskDataPtr;

	unsigned short* battLevel;
	unsigned short* powerConsumption;
	unsigned short* powerGeneration;
	Bool* panelState;

	battLevel = (unsigned short*) dataPtr->battLevelPtr;
	powerConsumption = (unsigned short*) dataPtr->powerConsumptionPtr;
	powerGeneration = (unsigned short*) dataPtr->powerGenerationPtr;
	panelState = (Bool*) dataPtr->panelStatePtr;

	//powerConsumption
	static unsigned short runCount = 1;				//tracks even/odd calls of this function

	runCount = (runCount + 1) % 2;					//alternates between 1 and 0 for odd/een calls respectively
	if (*powerConsumption<10)     //TODO, does this work???
		if (runCount==0){							//on even calls...
			*powerConsumption += 2;		//increment by 2
		}
		else{										//on odd calls...
			*powerConsumption -= 1;		//decrement by 1
		}
	}
	else{
		if (runCount==0){							//on even calls...
			*powerConsumption -= 2;	//decrement by 2
		}
		else{										//on odd calls...
			*powerConsumption += 1;		//increment by 1
	}


	//powerGeneration
	if (*panelState==1){			//if solar panel is deployed...
		if (*battLevel<95){		//if battery greater than 95%
			*panelState=0;		//retract solar panel
		}
		else{										//else battery less than/equal to 95%
			if (runCount==0){							//on even calls...
			*powerGeneration += 2;		//increment by 2
			}
			else if(*battLevel<=50){	//on odd calls... while battery less than/equal to 50%
			*powerGeneration += 1;		//increment by 1
			}
		}
	}
	else											//else solar panel not deployed...
		if (*battLevel<=10){		//if battery less than/equal to 10%
			*panelState = 1;		//deploy solar panel
		}
	}


	//batteryLevel
	if (*panelState==0){
		(*battLevel) = (*battLevel) - 3*(*powerConsumption);
	}
	else{
		(*battLevel) = (*battLevel) - (*powerConsumption) + (*powerGeneration);
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
	volatile unsigned long i = 0;
    volatile unsigned int j = 0;
    
    for (i = time_in_ms; i > 0; i--)
    {
        for (j = 0; j < 100; j++);
    }
    return;
}