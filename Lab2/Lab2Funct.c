#include <stdio.h>
#include <stdint.h>
// #include <limits.h>
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

//define some constants
extern const unsigned short MAX_BATT_LEVEL;
extern const unsigned short HALF_BATT_LEVEL;
extern const unsigned short BATT_WARN_LEVEL;
extern const uint32_t MAX_FUEL_LEVEL;
extern const uint32_t HALF_FUEL_LEVEL;
extern const uint32_t FUEL_WARN_LEVEL;
extern const unsigned short TASK_QUEUE_LENGTH;

//define and initiallize global variables
const int fuelBuringRatio = 20; // Set as a large number in demo


//TODO
void schedule(scheduleDataStruct scheduleData){
	unsigned short* globalCount = (unsigned short*) scheduleData.globalCountPtr;
	Bool* isMajorCycle = (Bool*) scheduleData.isMajorCyclePtr;

	*isMajorCycle = ((*globalCount) == 0);			//Execute a Major Cycle when the count is zero.

	(*globalCount) = ((*globalCount) + 1) % (TASK_QUEUE_LENGTH - 1); //count to 5, then start over again
	delay_ms(100);
}

// Requires: power sub data struct
// Modifies: powerConsumption, powerGeneration, battLevel, panelState
void powerSub(void* taskDataPtr){
	powerSubDataStruct* dataPtr = (powerSubDataStruct*) taskDataPtr;

	// unsigned short* globalCount = (unsigned short*) dataPtr->globalCountPtr;
	Bool* isMajorCycle = (Bool*) dataPtr->isMajorCyclePtr;
	unsigned short* battLevel = (unsigned short*) dataPtr->battLevelPtr;
	unsigned short* powerConsumption = (unsigned short*) dataPtr->powerConsumptionPtr;
	unsigned short* powerGeneration = (unsigned short*) dataPtr->powerGenerationPtr;
	Bool* panelState = (Bool*) dataPtr->panelStatePtr;

	//powerConsumption
	static unsigned short runCount = 1;				//tracks even/odd calls of this function
	static Bool consumpUpDown = TRUE;

	if (*isMajorCycle)
	{
		runCount = (runCount + 1) % 2;					//alternates between 1 and 0 for odd/een calls respectively
		if (consumpUpDown) {    //TODO, does this work???
			if (runCount==0){							//on even calls...
				(*powerConsumption) += 2;		//increment by 2
				if ((*powerConsumption)>=10) {
					consumpUpDown = FALSE;
				}
			} 
			else{										//on odd calls...
				(*powerConsumption) -= 1;		//decrement by 1
			}
		}
		else {
			if (runCount==0){							//on even calls...
				(*powerConsumption) -= 2;	//decrement by 2
				if ((*powerConsumption)<=5) {
					consumpUpDown = TRUE;
				}
			}
			else{										//on odd calls...
				(*powerConsumption) += 1;		//increment by 1
			}
		}        
		    
		//powerGeneration
		if ((*panelState)==FALSE) {											//else solar panel not deployed...
			if ((*battLevel)<=30){		//if battery less than/equal to 10%
				(*panelState) = TRUE;		//deploy solar panel
			}
		}
		if ((*panelState)==TRUE) {			//if solar panel is deployed...
			if ((*battLevel)>95){		//if battery greater than 95%
				(*panelState)=FALSE;		//retract solar panel
				(*powerGeneration) = 0;         //SPEC CHANGE
			}
			else{										//else battery less than/equal to 95%
				if (runCount==0){							//on even calls...
					(*powerGeneration) += 2;		//increment by 2
				}
				else if((*battLevel)<=50){	//on odd calls... while battery less than/equal to 50%
					(*powerGeneration) += 1;		//increment by 1
				}
			}
		}
	}

	//batteryLevel
	if ((*panelState)==FALSE){
		(*battLevel) = (*battLevel) - 3*(*powerConsumption);
	}
	else{
		(*battLevel) = (*battLevel) - (*powerConsumption) + (*powerGeneration);
	}
	if((*battLevel)>100){ //"OVERLOAD PROTECTION"
		(*battLevel)=100;
	}
}

// Require : the minor clock running at 1 second per cycle, 
//			and thruster sub data struct; 
// Modifies: global constant fuelLevel, taking in count of duration.
void thrusterSub(void* taskDataPtr){

	thrusterSubDataStruct* thrustCommandPtr = (thrusterSubDataStruct*) taskDataPtr;
        
	// unsigned short* globalCount = (unsigned short*) thrustCommandPtr->globalCountPtr;
	Bool* isMajorCycle = (Bool*) thrustCommandPtr->isMajorCyclePtr;
	uint32_t* fuelPtr = (uint32_t*) thrustCommandPtr->fuelLevelPtr;

	unsigned short left = 0;
	unsigned short right = 0;
	unsigned short up = 0;
	unsigned short down = 0;
	static unsigned short magnitude = 0;
	static unsigned short duration = 0;

	// Only updates the magnitude when the duration is positive, otherwise no change on magnitude
	if (*isMajorCycle)
	{
		uint16_t command = *(uint16_t*)(thrustCommandPtr->thrustPtr);

		if (command & 0x0001) { left  = 1; } else { left  = 0;}
		if (command & 0x0002) { right = 1; } else { right = 0;}
		if (command & 0x0004) { up    = 1; } else { up    = 0;}
		if (command & 0x0008) { down  = 1; } else { down  = 0;}
		// get duration, unsigned char pointer moves every 8 bits
		// so it can separate first 8 bits and last 8 bits
		duration = *((unsigned char*)&command + 1);
		if (duration) { magnitude = (command >> 4) & 0x000F; }
	}

	//If the new command ask use a 
	if (duration)
	{
		*fuelPtr -= magnitude * fuelBuringRatio;
		duration -= 1;
	}

	// When the fuel level goes below 0, the unsigned int when to really big
	if (*fuelPtr > MAX_FUEL_LEVEL)
	{
		*fuelPtr = 0;
	}
}

// Communication only store the command without decoding it
// Require : satellite communication data struct,
//			 randomInteger function;
// Modifies: thrust command.
void satelliteComms(void* taskDataPtr){
        

	satelliteCommsDataStruct* commPtr = (satelliteCommsDataStruct*) taskDataPtr;


	unsigned short* globalCount = (unsigned short*) commPtr->globalCountPtr;
	Bool* isMajorCycle = (Bool*) commPtr->isMajorCyclePtr;
	if (isMajorCycle)
	{
		// send info, use print as a method to send
		Bool* fuelLowSignal = (Bool*)commPtr->fuelLowPtr;
		Bool* battLowSignal = (Bool*)commPtr->battLowPtr;
		unsigned short* battLevelSignal = (unsigned short*)commPtr->battLevelPtr;
		uint32_t* fuelLevelSignal = (uint32_t*)commPtr->fuelLevelPtr;
		unsigned short* powerConsumptionSignal = (unsigned short*)commPtr->powerConsumptionPtr;
		unsigned short* powerGenerationSignal = (unsigned short*)commPtr->powerGenerationPtr;
		Bool* panelStateSignal = (Bool*)commPtr->panelStatePtr;	
		printf("fuel low is : %u\n", *fuelLowSignal);
		printf("battery low is : %u\n", *battLowSignal);
		printf("battery level : %hu\n", *battLevelSignal);
		printf("fuel level : %u\n", *fuelLevelSignal);
		printf("power consumption is: %hu\n", *powerConsumptionSignal);
		printf("power generation is : %hu\n", *powerGenerationSignal);
		printf("panel state is : %u\n", *panelStateSignal);

		// receive (rando) thrust commands, generate from 0 to 2^16 -1
		uint16_t thrustCommand = randomInteger(globalCount);
		*(uint16_t*)(commPtr->thrustPtr) = thrustCommand;
	}
}

//TODO
void oledDisplay(void* taskDataPtr){
/*
    oledDisplayDataStruct* dataPtr = (oledDisplayDataStruct*) taskDataPtr;
    
    unsigned short* battLevel = (unsigned short*) dataPtr->battLevelPtr;
    unsigned short* fuelLevel = (unsigned short*) dataPtr->fuelLevelPtr;
    unsigned short* powerConsumption = (unsigned short*) dataPtr->powerConsumptionPtr;
    Bool* panelState = (Bool*) dataPtr->panelStatePtr;
    Bool* fuelLow = (Bool*) dataPtr->fuelLowPtr;
    Bool* battLow = (Bool*) dataPtr->battLowPtr;

    unsigned short* globalCount = (unsigned short*) dataPtr->globalCountPtr;
    Bool* isMajorCycle = (Bool*) dataPtr->isMajorCyclePtr;

    // TODO get statusMode from a button or based on globalCount
    // Status mode
    // volatile unsigned long statusMode;
    if (*isMajorCycle)
    {

		int statusMode = 0;//(globalCount % 2 == 0) ? 0 : 1;

	    char tempArr0[24];    
	    if (0 == statusMode)//0 == statusMode)
	    {
	      //arrSize = 4;
	      char panelDepl = 'Y';//(1 == *panelState) ? 'Y' : 'N';
	      //usnprintf(tempArr0, 10, "Yellow");
	      //usnprintf(tempArr0, 24, "Panel Deployed: %c", panelDepl);
	      //RIT128x96x4StringDraw(tempArr0, 5, 10, 15);

	      //usnprintf(tempArr0, 24, "Battery Level: %d", *battLevel);
	      //RIT128x96x4StringDraw(tempArr0, 5, 20, 15);

	      //usnprintf(tempArr0, 24, "Fuel Level: %d", *fuelLevel);
	      //RIT128x96x4StringDraw(tempArr0, 5, 30, 15);

	      //usnprintf(tempArr0, 24, "Power Consumption: %d", *powerConsumption);
	      //RIT128x96x4StringDraw(tempArr0, 5, 40, 15);
	    
	    } else // Annunciation mode
	    {
	      char fuelLowStr = (1 == *fuelLow) ? 'Y' : 'N';
	      //usnprintf(tempArr0, 24, "Fuel Low: %c", fuelLowStr);
	      //RIT128x96x4StringDraw(tempArr0, 5, 10, 15);

	      char battLowStr = (1 == *battLow) ? 'Y' : 'N';
	      //usnprintf(tempArr0, 24, "Battery Low: %c", battLowStr);
	      //RIT128x96x4StringDraw(tempArr0, 5, 20, 15);
	    }
	    */

	  /*
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
	    unsigned short* globalCount = (unsigned short*) dataPtr->globalCountPtr;
	    Bool* isMajorCycle = (Bool*) dataPtr->isMajorCyclePtr;

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

    }
    
*/
}


// Require : warning alarm data struct;
// Modifies: fuelLow pointer, battLow pointer.
void warningAlarm(void* taskDataPtr){
	
	warningAlarmDataStruct* dataPtr = (warningAlarmDataStruct*) taskDataPtr;
	Bool* fuelLow = (Bool*)dataPtr->fuelLowPtr;
	Bool* battLow = (Bool*)dataPtr->battLowPtr;
	unsigned short* battLevel = (unsigned short*)dataPtr->battLevelPtr;
	uint32_t* fuelLevel = (uint32_t*)dataPtr->fuelLevelPtr;
	// unsigned short* globalCount = (unsigned short*) dataPtr->globalCountPtr;
	Bool* isMajorCycle = (Bool*) dataPtr->isMajorCyclePtr;

    if (*fuelLevel < FUEL_WARN_LEVEL) { *fuelLow = TRUE; } else { *fuelLow = FALSE; }
    if (*battLevel < BATT_WARN_LEVEL) { *battLow = TRUE; } else { *battLow = FALSE; }

    if (*isMajorCycle)
    {
		
		if ((*battLevel<BATT_WARN_LEVEL)&(*fuelLevel>HALF_FUEL_LEVEL)){
			//display solid green LED
			//
			GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_5, 0xFF);
		}
		else{
			//SPEC CHANGE!!! YELLOW is BATT, RED is FUEL
			if (*battLevel<BATT_WARN_LEVEL){
				//TODO flash yellow 1 sec
				if ((*globalCount)%2)
					GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_6, 0xFF);
				else
					GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_6, 0x00);
			}
			else if (*battLevel<HALF_BATT_LEVEL){
				//TODO flash yellow 2 sec
				GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_6, 0xFF);
			}
			if (*fuelLevel<FUEL_WARN_LEVEL){
				//TODO flash red 1 sec
				GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_7, 0xFF);
			}
			else if (*fuelLevel<HALF_FUEL_LEVEL){
				//TODO flash red 2 sec
				GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_7, 0xFF);
			}
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

