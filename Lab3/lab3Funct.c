#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "lab3.h"
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

// Constants defined in main
extern const unsigned short MAX_BATT_LEVEL;
extern const unsigned short HALF_BATT_LEVEL;
extern const unsigned short BATT_WARN_LEVEL;
extern const uint32_t MAX_FUEL_LEVEL;
extern const uint32_t HALF_FUEL_LEVEL;
extern const uint32_t FUEL_WARN_LEVEL;
extern const unsigned short TASK_QUEUE_LENGTH;
extern unsigned short globalCount;
extern unsigned short blinkTimer;
extern uint32_t fuelLevellll;

// local variable used in functions
const int fuelBuringRatio = 20000; // Set as a large number in demo

// Control the major or minor cycle in main function
void schedule(scheduleDataStruct scheduleData){
    Bool* isMajorCycle = (Bool*) scheduleData.isMajorCyclePtr;

    if (0  == globalCount) {*isMajorCycle = TRUE;} else {*isMajorCycle = FALSE;}			//Execute a Major Cycle when the count is zero.

    delay_ms(7500);
    globalCount = (globalCount + 1) % (TASK_QUEUE_LENGTH - 1); //count to 5, then start over again
    blinkTimer = (blinkTimer + 1) % 8;

}

// Requires: power sub data struct
// Modifies: powerConsumption, powerGeneration, battLevel, panelState
void powerSub(void* taskDataPtr){
    // // Start oscillascope measurement
    // GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4, 0xFF);

	powerSubDataStruct* dataPtr = (powerSubDataStruct*) taskDataPtr;

	// unsigned short* globalCount = (unsigned short*) dataPtr->globalCountPtr;
	// Bool* isMajorCycle = (Bool*) dataPtr->isMajorCyclePtr;
	unsigned int* battLevel = (unsigned int*) dataPtr->battLevelPtr; // Points to address of battLevelPtr[0]
	unsigned short* powerConsumption = (unsigned short*) dataPtr->powerConsumptionPtr;
	unsigned short* powerGeneration = (unsigned short*) dataPtr->powerGenerationPtr;
	Bool* panelState = (Bool*) dataPtr->panelStatePtr;

	//powerConsumption
	static unsigned short runCount = 1;         //tracks even/odd calls of this function
	static Bool consumpUpDown = TRUE;
        
    //powerGeneration
    if (!(*panelState)) {                       //else solar panel not deployed...
        if ((*battLevel)<=30){                  //if battery less than/equal to 10%
            (*panelState) = TRUE;               //deploy solar panel
        }
    } 
    if (*panelState) {                          //if solar panel is deployed...
        if ((*battLevel)>95){		            //if battery greater than 75%
            (*powerGeneration) = 0;             //SPEC CHANGE
            (*panelState)=FALSE;		        //retract solar panel
        }
        else{					                //else battery less than/equal to 95%
            if (0==runCount){			        //on even calls...
                (*powerGeneration) += 2;		//increment by 2
            }
            else if((*battLevel)<=50){	        //on odd calls... while battery less than/equal to 50%
                (*powerGeneration) += 1;		//increment by 1
            }
        }
    } 

    runCount = !runCount;			            //alternates between zero and non zero for odd/een calls respectively
    if (consumpUpDown) {
        if (0==runCount){				        //on even calls...
            (*powerConsumption) += 2;		    //increment by 2
            if ((*powerConsumption)>=10) {
                consumpUpDown = FALSE;
            }
        } 
        else{						            //on odd calls...
            (*powerConsumption) -= 1;		    //decrement by 1
        }
    }
    else {
        if (0==runCount){		                //on even calls...
            (*powerConsumption) -= 2;	        //decrement by 2
            if ((*powerConsumption)<=5) {
                consumpUpDown = TRUE;
            }
        }
        else{						            //on odd calls...
            (*powerConsumption) += 1;		    //increment by 1
        }
    }
    
    // TODO remove this before final version; included for testing
    /*/ // DEPRECATED    
	//batteryLevel
	if (!(*panelState)){
		(*battLevel) = (*battLevel) - 3*(*powerConsumption);
	}
	else{
		(*battLevel) = (*battLevel) - (*powerConsumption) + (*powerGeneration);
	}
	
        if(((*battLevel)>100)&&((*battLevel)<300)){ //"OVERLOAD PROTECTION"
		(*battLevel)=100;
	}
	if((*battLevel)>65000){                        //"NEGATIVE PROTECTION"
		(*battLevel)=0;
	}/**/

    // Battery measurement
    // following interrupt:
    // delay 600us
    delay_ms(100); // TODO determine correct value for 600us
    // Below code for ADC measurement adapted from single_ended.c
    //  in IAR example file
    // Clear interrupt status flag
    ADCIntClear(ADC0_BASE, 3);
    ADCProcessorTrigger(ADC0_BASE, 3);

    // Wait for conversion to be completed.
    while(!ADCIntStatus(ADC0_BASE, 3, false))
    {
    }

    // Clear the ADC interrupt flag.
    ADCIntClear(ADC0_BASE, 3);
    // Create array to hold ADC value
    unsigned int adcReading[1]; // TODO ADC returns long or int?
    // Read ADC Value.
    ADCSequenceDataGet(ADC0_BASE, 3, ulADC0_Value);

    // convert adcReading from 4.25V to 36V scale
    // If ADC returns 10bit int (0-1023), each digit ~= 0.00415V
    // Then, multiply by 8.4706 to get to 36V range
    unsigned int adcReadingConverted = adcReading[0] * 0.00415 * 8.4706;
    // move previous readings to next array slot
    for (int i = sizeof(battLevel) - 2; i > 0; --i)
    {
        battLevel[i+1] = battLevel[i];
    }
    // Add new reading to front of circular buffer
    battLevel[0] = adcReadingConverted;

    // // End oscillascope measurement
    // GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4, 0x00);
}

void solarPanelControl(void* taskDataPtr) {
//        // Compute the PWM period based on the system clock.
//        // Base clock 8MHz, want 2Hz for panel motor ( / 4000000)
        unsigned long ulPeriod = SysCtlClockGet() / 80; //run at 100kHz
        static unsigned long dutyCycle = 50;
        
        // Read keypad input and adjust duty cycle based on keypress
        if (GPIOPinRead(GPIO_PORTA_BASE, GPIO_PIN_0)){
          dutyCycle += 5;
        }
        if (GPIOPinRead(GPIO_PORTA_BASE, GPIO_PIN_1)){
          dutyCycle -= 5;
        }
        dutyCycle = dutyCycle % 100;
        
        //Set the PWM period to 2 Hz.
        PWMPulseWidthSet(PWM0_BASE, PWM_OUT_0, ulPeriod);
        
        PWMGenConfigure(PWM0_BASE, PWM_GEN_0,
                    PWM_GEN_MODE_UP_DOWN | PWM_GEN_MODE_NO_SYNC);
        PWMGenPeriodSet(PWM0_BASE, PWM_GEN_0, ulPeriod);
        
        PWMPulseWidthSet(PWM0_BASE, PWM_OUT_0, ulPeriod * dutyCycle / 100);
        
        //Enable the PWM0 and PWM1 output signals.

        PWMOutputState(PWM0_BASE, PWM_OUT_0_BIT | PWM_OUT_1_BIT, true);

       
        //Enable the PWM generator.
        
        PWMGenEnable(PWM0_BASE, PWM_GEN_0);
        return;
}


// Communication only store the command without decoding it
// Require : satellite communication data struct,
//           randomInteger function;
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
        //printf("fuel low is : %u\n", *fuelLowSignal);
        //printf("battery low is : %u\n", *battLowSignal);
        //printf("battery level : %hu\n", *battLevelSignal);
        //printf("fuel level : %u\n", *fuelLevelSignal);
        //printf("power consumption is: %hu\n", *powerConsumptionSignal);
        //printf("power generation is : %hu\n", *powerGenerationSignal);
        //printf("panel state is : %u\n", *panelStateSignal);

        // receive (rando) thrust commands, generate from 0 to 2^16 -1
        // uint16_t thrustCommand = randomInteger(globalCount);
        uint16_t thrustCommand = 0x0FF1;
        *(uint16_t*)(commPtr->thrustPtr) = thrustCommand;
    }
    return;
}

void vehicleComms(void* taskDataPtr){

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

	// If the new command ask use a 
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
        
    // Convert inner counter to 100 scale for OLED display
    fuelLevellll = ((*fuelPtr) * 100) / MAX_FUEL_LEVEL;
}

//TODO
void oledDisplay(void* taskDataPtr){
    
  
    oledDisplayDataStruct* dataPtr = (oledDisplayDataStruct*) taskDataPtr;
    
    unsigned int* battLevel = (unsigned int*) dataPtr->battLevelPtr;
    uint32_t* fuelLevelPtr2 = (uint32_t*) dataPtr->fuelLevelPtr;
    unsigned short* powerConsumption = (unsigned short*) dataPtr->powerConsumptionPtr;
    Bool* panelState = (Bool*) dataPtr->panelStatePtr;
    Bool* fuelLow = (Bool*) dataPtr->fuelLowPtr;
    Bool* battLow = (Bool*) dataPtr->battLowPtr;

    unsigned short* globalCount = (unsigned short*) dataPtr->globalCountPtr;
    Bool* isMajorCycle = (Bool*) dataPtr->isMajorCyclePtr;

    // Push select button to change to annunciation mode on OLED
    long buttonRead = 2;
    char* bufferPtr;

    // Pushed = 0, released = 2 from our measurement
    buttonRead = GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_1);
 
    if (2 == buttonRead)    // Status display mode
    {
        RIT128x96x4Clear();

        // Display panel state.
        char panelDepl = (1 == *panelState) ? 'Y' : 'N';
        RIT128x96x4StringDraw( "Panel Deployed: ", 5, 10, 15);
        RIT128x96x4StringDraw( &panelDepl, 5, 20, 15);
        
        // Display battery level. Cast integer to char array using snprintf
        snprintf(bufferPtr, 20, "%d", *battLevel);
        RIT128x96x4StringDraw( "Battery Level: ", 5, 30, 15); // battLevel points to 0x200001EA, globalCount
        RIT128x96x4StringDraw( bufferPtr, 5, 40, 15);

        // Display fuel level.
        snprintf(bufferPtr, 20, "%d", fuelLevellll);
        RIT128x96x4StringDraw( "Fuel Level: ", 5, 50, 15);
        RIT128x96x4StringDraw( bufferPtr, 5, 60, 15);

        // Display power consumption.
        snprintf(bufferPtr, 20, "%d", *powerConsumption);
        RIT128x96x4StringDraw( "Power Consumption: ", 5, 70, 15);
        RIT128x96x4StringDraw( bufferPtr, 5, 80, 15);
      
    } else if (0 == buttonRead) // Annunciation mode
    {
        RIT128x96x4Clear();
        
        // Display fuel low flag.
        char fuelLowStr = (1 == *fuelLow) ? 'Y' : 'N';
        RIT128x96x4StringDraw( "Fuel Low: ", 5, 10, 15);
        RIT128x96x4StringDraw( &fuelLowStr, 5, 20, 15);

        // Display battery low flag.
        char battLowStr = (1 == *battLow) ? 'Y' : 'N';
        RIT128x96x4StringDraw( "Battery Low: ", 5, 30, 15);
        RIT128x96x4StringDraw( &battLowStr, 5, 40, 15);
    }

    return;
}

void consoleKeyboard(void* taskDataPtr){

}

// Require : warning alarm data struct;
// Modifies: fuelLow pointer, battLow pointer.
void warningAlarm(void* taskDataPtr){
	
    warningAlarmDataStruct* dataPtr = (warningAlarmDataStruct*) taskDataPtr;
    Bool* fuelLow = (Bool*)dataPtr->fuelLowPtr;
    Bool* battLow = (Bool*)dataPtr->battLowPtr;
    unsigned int* battLevel = (unsigned int*)dataPtr->battLevelPtr;
    uint32_t* fuelLevel = (uint32_t*)dataPtr->fuelLevelPtr;
    unsigned short* globalCount = (unsigned short*) dataPtr->globalCountPtr;
    Bool* isMajorCycle = (Bool*) dataPtr->isMajorCyclePtr;

    if (*fuelLevel < FUEL_WARN_LEVEL) { *fuelLow = TRUE; } else { *fuelLow = FALSE; }
    if (*battLevel < BATT_WARN_LEVEL) { *battLow = TRUE; } else { *battLow = FALSE; }
    
    if (((*battLevel)>HALF_BATT_LEVEL)&&((*fuelLevel)>HALF_FUEL_LEVEL)){
        //display solid green LED
        GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_5, 0xFF);
        //clear yellow LED
        GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_6, 0x00);
        //clear red LED
        GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_7, 0x00);
    }
    else{
        //clear solid green LED
        GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_5, 0x00);
        //SPEC CHANGE!!! YELLOW is BATT, RED is FUEL
            if ((*battLevel<BATT_WARN_LEVEL)&&(*battLevel<HALF_BATT_LEVEL)){
                //TODO flash yellow 1 sec
                if ((0==blinkTimer)||(2==blinkTimer)||(4==blinkTimer)||
                    (6==blinkTimer))
                    GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_6, 0xFF);
                if ((1==blinkTimer)||(3==blinkTimer)||(5==blinkTimer)||
                    (7==blinkTimer))
                    GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_6, 0x00);
            }
            else if (*battLevel<HALF_BATT_LEVEL){
                //TODO flash yellow 2 sec
                if ((0==blinkTimer)||(1==blinkTimer)||(4==blinkTimer)||(5==blinkTimer))  
                    GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_6, 0xFF);
                if ((2==blinkTimer)||(3==blinkTimer)||(6==blinkTimer)||(7==blinkTimer))  
                    GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_6, 0x00);
            }
            if ((*fuelLevel<FUEL_WARN_LEVEL)&(*fuelLevel<HALF_FUEL_LEVEL)){
                //TODO flash red 1 sec
                if ((0==blinkTimer)||(2==blinkTimer)||(4==blinkTimer)||(6==blinkTimer))  
                    GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_7, 0xFF);
                if ((1==blinkTimer)||(3==blinkTimer)||(5==blinkTimer)||(7==blinkTimer))  
                    GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_7, 0x00);
            }
            else if (*fuelLevel<HALF_FUEL_LEVEL){
                //TODO flash red 2 sec
                if ((0==blinkTimer)||(1==blinkTimer)||(4==blinkTimer)||(5==blinkTimer))  
                    GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_7, 0xFF);
                if ((2==blinkTimer)||(3==blinkTimer)||(6==blinkTimer)||(7==blinkTimer))  
                    GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_7, 0x00);
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
