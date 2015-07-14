#include <stdio.h>
#include <stdint.h>
#include <limits.h>
#include "lab2.h"

//TODO Define all Functions Here

//define some constants
extern const unsigned short HALF_WARN_LEVEL;
extern const unsigned short FUEL_WARN_LEVEL;
extern const unsigned short BATT_WARN_LEVEL;
extern const unsigned short MAX_FUEL_LEVEL;
extern const unsigned short MAX_BATT_LEVEL;
extern const unsigned short TASK_QUEUE_LENGTH;

//define and initiallize global variables
extern unsigned short battLevel;
extern unsigned short fuelLevel;
extern unsigned short powerConsumption;
extern unsigned short powerGeneration;
extern Bool panelState;
extern uint16_t thrust; //16bit encoded thrust command [15:8]Duration,[7:4]Magnitude,[3:0]Direction
extern Bool fuelLow;
extern Bool battLow;
extern unsigned short globalCount;
extern Bool majorMinorCycle;
int seed = 12;


//TODO
void schedule(){
	if (globalCount == 0){								//On first cycle of five...
		majorMinorCycle = FALSE;							//Execute a Major Cycle
	}
	else {												//On all other cycles...
		majorMinorCycle = TRUE;							//Execute a Minor Cycle
	}
	globalCount = (globalCount + 1) % (TASK_QUEUE_LENGTH - 1); //count to 5, then start over again
	delay_ms(100000);
}

//TODO
void powerSub(void* taskDataPtr){
	printf("We get to power sub system with count at: %i \n", globalCount);
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
	if (*powerConsumption<10) {    //TODO, does this work???
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


// Require : 
// Modifies: fuelLevel
void thrusterSub(void* taskDataPtr){
	//TODO concatenate signals into control command
	printf("thruster sub system with count at: %i \n", globalCount);
	//TODO figure out fuel consumption
	thrusterSubDataStruct* thrustCommandPtr = (thrusterSubDataStruct*) taskDataPtr;

	uint16_t temp = *(uint16_t*)(thrustCommandPtr->thrustPtr);
	printf("the thrust command should change to: %d\n", temp);
}

//TODO
void satelliteComms(void* taskDataPtr){
	//TODO send info
	printf("Comms at: %i \n", globalCount);

	satelliteCommsDataStruct* commPtr = (satelliteCommsDataStruct*) taskDataPtr;

	//TODO receive (rando) thrust commands
	uint16_t thrustCommand = randomInteger(-100, 100);
	// char* byteRead = (char*) &thrustCommand;
	*(uint16_t*)(commPtr->thrustPtr) = thrustCommand;
	
		//TODO implement rand num gen
}

//TODO
void oledDisplay(void* taskDataPtr){
	//TODO two modes??
	printf("OLED Display wooorrrrrrrrrkkkkkkkkkkkkkkkkk at: %i \n", globalCount);

	        //
    // Initialize the OLED display.
    //
    RIT128x96x4Init(1000000);
    
    oledDisplayDataStruct* dataPtr = (oledDisplayDataStruct*) taskDataPtr;
    
    unsigned short* battLevel = (unsigned short*) dataPtr->battLevelPtr;
    unsigned short* fuelLevel = (unsigned short*) dataPtr->fuelLevelPtr;
    unsigned short* powerConsumption = (unsigned short*) dataPtr->powerConsumptionPtr;
    Bool* panelState = (Bool*) dataPtr->panelStatePtr;
    Bool* fuelLow = (Bool*) dataPtr->fuelLowPtr;
    Bool* battLow = (Bool*) dataPtr->battLowPtr;

    
    // TODO display bool values in words not ints
    // Status mode
    char arrStatus[4][20];
    sprintf(arrStatus[0], "Solar Panel Deployed: %d", *panelState);
    sprintf(arrStatus[1], "Battery Level: %d", *battLevel);
    sprintf(arrStatus[2], "Fuel Level: %d", *fuelLevel);
    sprintf(arrStatus[3], "Power Consumption: %d", *powerConsumption);
    
    // Annunciation mode
    char arrAnnun[2][20];
    sprintf(arrAnnun[0], "Fuel Low: %d", *fuelLow);
    sprintf(arrAnnun[1], "Battery Low: %d", *battLow);
    
    for (int i = 0; i < 5; i++)
    {      
      char *pcStr = arr[i];
      
      RIT128x96x4StringDraw(pcStr, 5, 24 + 10*i, 15);
    }
}

void warningAlarm(void* taskDataPtr){

	if ((battLevel<BATT_WARN_LEVEL)&(fuelLevel>HALF_WARN_LEVEL)){
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

// Import from the function from Prof. Peckol
int randomInteger(int low, int high)
{
	double randNum = 0.0;
 	int multiplier = 2743;
	int addOn = 5923;
	double max = INT_MAX + 1.0;

	int retVal = 0;

	if (low > high)
		retVal = randomInteger(high, low);
	else
	{
   		seed = seed*multiplier + addOn;
   		randNum = seed;

		if (randNum <0)
		{
			randNum = randNum + max;
		}

		randNum = randNum/max;

		retVal =  ((int)((high-low+1)*randNum))+low;
	}
	
	return retVal;
}