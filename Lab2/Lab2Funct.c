#include <stdio.h>
#include <stdint.h>
#include <limits.h>
#include "lab2.h"
// #include "inc/hw_gpio.h"
// #include "inc/hw_ints.h"
// #include "inc/hw_memmap.h"
// #include "inc/hw_types.h"
// #include "driverlib/debug.h"
// #include "driverlib/gpio.h"
// #include "driverlib/interrupt.h"
// #include "driverlib/sysctl.h"
// #include "drivers/rit128x96x4.h"
// #include "inc/lm3s8962.h"
// #include "utils/ustdlib.h"

//define some constants
extern const unsigned short HALF_WARN_LEVEL;
extern const unsigned short FUEL_WARN_LEVEL;
extern const unsigned short BATT_WARN_LEVEL;
extern const unsigned short MAX_FUEL_LEVEL;
extern const unsigned short MAX_BATT_LEVEL;
extern const unsigned short TASK_QUEUE_LENGTH;

//define and initiallize global variables
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
	unsigned short left = 0;
	unsigned short right = 0;
	unsigned short up = 0;
	unsigned short down = 0;
	unsigned short magnitude = 0;
	unsigned short duration = 0;
	//TODO concatenate signals into control command
	thrusterSubDataStruct* thrustCommandPtr = (thrusterSubDataStruct*) taskDataPtr;
	printf("thruster sub system with count at: %i \n", globalCount);

	//TODO figure out fuel consumption
	uint16_t command = *(uint16_t*)(thrustCommandPtr->thrustPtr);

	if (command & 0x0001) { left  = 1; } else { left  = 0;}
	if (command & 0x0002) { right = 1; } else { right = 0;}
	if (command & 0x0004) { up    = 1; } else { up    = 0;}
	if (command & 0x0008) { down  = 1; } else { down  = 0;}
	magnitude = (command >> 4) & 0x000F;
	// get duration, unsigned char pointer moves every 8 bits
	// so it can separate first 8 bits and last 8 bits
	duration = *((unsigned char*)&command + 1);
}

//TODO
void satelliteComms(void* taskDataPtr){
	//TODO send info
	printf("Comms at: %i \n", globalCount);
	satelliteCommsDataStruct* commPtr = (satelliteCommsDataStruct*) taskDataPtr;

	Bool* fuelLowSignal = (Bool*)commPtr->fuelLowPtr;
	Bool* battLowSignal = (Bool*)commPtr->battLowPtr;
	unsigned short* battLevelSignal = (unsigned short*)commPtr->battLevelPtr;
	unsigned short* fuelLevelSignal = (unsigned short*)commPtr->fuelLevelPtr;
	unsigned short* powerConsumptionSignal = (unsigned short*)commPtr->powerConsumptionPtr;
	unsigned short* powerGenerationSignal = (unsigned short*)commPtr->powerGenerationPtr;
	Bool* panelStateSignal = (Bool*)commPtr->panelStatePtr;

	printf("fuel low is : %u\n", *fuelLowSignal);
	printf("battery low is : %u\n", *battLowSignal);
	printf("battery level : %hu\n", *battLevelSignal);
	printf("fuel level : %hu\n", *fuelLevelSignal);
	printf("power consumption is: %hu\n", *powerConsumptionSignal);
	printf("power generation is : %hu\n", *powerGenerationSignal);
	printf("panel state is : %u\n", *panelStateSignal);

	//TODO receive (rando) thrust commands, generate from 0 to 2^16 -1
	uint16_t thrustCommand = randomInteger(0, 65535); 
	*(uint16_t*)(commPtr->thrustPtr) = thrustCommand;
	//TODO implement rand num gen
}

//TODO
void oledDisplay(void* taskDataPtr){
	/*

	//TODO two modes??
	//printf("OLED Display wooorrrrrrrrrkkkkkkkkkkkkkkkkk at: %i \n", globalCount);
        
    // Initialize the OLED display.
    RIT128x96x4Init(1000000);
    //RIT128x96x4Clear();
    RIT128x96x4StringDraw("After clear", 5, 24, 15);
    
    oledDisplayDataStruct* dataPtr = (oledDisplayDataStruct*) taskDataPtr;
    
    unsigned short* battLevel = (unsigned short*) dataPtr->battLevelPtr;
    unsigned short* fuelLevel = (unsigned short*) dataPtr->fuelLevelPtr;
    unsigned short* powerConsumption = (unsigned short*) dataPtr->powerConsumptionPtr;
    Bool* panelState = (Bool*) dataPtr->panelStatePtr;
    Bool* fuelLow = (Bool*) dataPtr->fuelLowPtr;
    Bool* battLow = (Bool*) dataPtr->battLowPtr;

    char arr[4][24];
    int arrSize;
    // TODO display bool values in words not ints
    // Status mode
    Bool statusMode = 1;
    if (statusMode)
    {
      arrSize = 4;
      char tempArr0[24];
      usnprintf(tempArr0, 24, "Panel Deployed: %d", *panelState); // Goes into fault interrupt if arr[0] is used
      //strcpy(tempArr0, arr[0], 24);
      RIT128x96x4StringDraw(tempArr0, 5, 40, 15);
      // usnprintf(arr[1], 24, "Battery Level: %d", *battLevel);
      // usnprintf(arr[2], 24, "Fuel Level: %d", *fuelLevel);
      // usnprintf(arr[3], 24, "Power Consumption: %d", *powerConsumption);
    
    } else // Annunciation mode
    {
      arrSize = 2;
      // usnprintf(arr[0], 24, "Fuel Low: %d", *fuelLow);
      // usnprintf(arr[1], 24, "Battery Low: %d", *battLow);
    }
    
    for (int i = 0; i < arrSize; i++)
    {      
      char *pcStr = arr[i];
      
      //RIT128x96x4StringDraw(pcStr, 5, 24 + 10*i, 15);
    }

    */
}

void warningAlarm(void* taskDataPtr){
	warningAlarmDataStruct* dataPtr = (warningAlarmDataStruct*) taskDataPtr;
	Bool* fuelLow = (Bool*)dataPtr->fuelLowPtr;
	Bool* battLow = (Bool*)dataPtr->battLowPtr;
	unsigned short* battLevel = (unsigned short*)dataPtr->battLevelPtr;
	unsigned short* fuelLevel = (unsigned short*)dataPtr->fuelLevelPtr;

    // GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_5, 0xF0);
	if ((*battLevel<BATT_WARN_LEVEL)&(*fuelLevel>HALF_WARN_LEVEL)){
                //display solid green LED
		// GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_5, 0xF0);
	}
	else{
		if (*battLevel<BATT_WARN_LEVEL){
			//TODO flash red 1 sec
		}
		else if (*fuelLevel<FUEL_WARN_LEVEL){
			//TODO flash red 2 sec
		}
		if (*battLevel<BATT_WARN_LEVEL){
			//TODO flash red 1 sec
		}
		else if (*fuelLevel<FUEL_WARN_LEVEL){
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