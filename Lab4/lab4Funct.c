#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "lab4.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/lm3s8962.h"
#include "driverlib/adc.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/pwm.h"
#include "driverlib/sysctl.h"
#include "lcd_message.h" // OLED
#include "rit128x96x4.h" // OLED

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"


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
extern Bool panelAndKeypadTask;
extern Bool panelDone;
extern Bool hasNewKeyboardInput;
extern unsigned int* battLevelPtr;

/* 
  The queue used to send messages to the OLED task.
  Defined and initialized in main.c 
*/
extern xQueueHandle xOLEDQueue;

// local variable used in functions
const int fuelBuringRatio = 2000; // Set as a large number in demo
const int sysDelayLength = 1000000;

// Control the major or minor cycle in main function
void schedule(scheduleDataStruct scheduleData)
{
    Bool* isMajorCycle = (Bool*) scheduleData.isMajorCyclePtr;

    //Execute a Major Cycle when the count is zero.
    if (0  == globalCount) 
    {
        *isMajorCycle = TRUE;
    } 
    else 
    {
        *isMajorCycle = FALSE; 
    }
    SysCtlDelay(sysDelayLength);

    blinkTimer = (blinkTimer + 1) % 8;
}

// Requires: power sub data struct
// Modifies: powerConsumption, powerGeneration, battLevel, panelState
void powerSub(void* taskDataPtr)
{
    powerSubDataStruct* dataPtr = (powerSubDataStruct*) taskDataPtr;
    unsigned int* battLevel = (unsigned int*) dataPtr->battLevelPtr; // Points to address of battLevelPtr[0]
    unsigned short* powerConsumption = (unsigned short*) dataPtr->powerConsumptionPtr;
    unsigned short* powerGeneration = (unsigned short*) dataPtr->powerGenerationPtr;
    Bool* panelState = (Bool*) dataPtr->panelStatePtr;
    Bool* panelDeploy = (Bool*) dataPtr->panelDeployPtr;
    Bool* panelRetract = (Bool*) dataPtr->panelRetractPtr;
    
    while(1)
    {
        //powerConsumption
        static unsigned short runCount = 1;         //tracks even/odd calls of this function
        static Bool consumpUpDown = TRUE;

        if (!(*panelState) && panelDone)
        {
            *panelState = TRUE;
            panelDone = FALSE;
            panelAndKeypadTask = FALSE;
        }
        else if ((*panelState) && panelDone)
        {
            *panelState = FALSE;
            panelDone = FALSE;
            panelAndKeypadTask = FALSE;
        }
            
        //powerGeneration
        if (!(*panelState)) {                       //else solar panel not deployed...
            if ((*battLevel)<=20){                  //if battery less than/equal to 20%
                (*panelDeploy) = TRUE;              //deploy solar panel
                (*panelRetract) = FALSE;
                panelAndKeypadTask = TRUE;        // Set flag to add solarPanel and keyboard tasks to task queue
            }
        } 
        if (*panelState) {                          //if solar panel is deployed...
            if ((*battLevel)>95){                   //if battery greater than 95%
                (*powerGeneration) = 0;             //SPEC CHANGE
                (*panelDeploy) = FALSE;
                (*panelRetract)=TRUE;               //retract solar panel
                panelAndKeypadTask = TRUE;        // Set flag to add solarPanel and keyboard tasks to task queue
            }
            else{                                   //else battery less than/equal to 95%
                if (0==runCount){                   //on even calls...
                    (*powerGeneration) += 2;        //increment by 2
                }
                else if((*battLevel)<=50){          //on odd calls... while battery less than/equal to 50%
                    (*powerGeneration) += 1;        //increment by 1
                }
            }
        } 

        runCount = !runCount;                       //alternates between zero and non zero for odd/een calls respectively
        if (consumpUpDown) {
            if (0==runCount){                       //on even calls...
                (*powerConsumption) += 2;           //increment by 2
                if ((*powerConsumption)>=10) {
                    consumpUpDown = FALSE;
                }
            } 
            else{                                   //on odd calls...
                (*powerConsumption) -= 1;           //decrement by 1
            }
        }
        else {
            if (0==runCount){                       //on even calls...
                (*powerConsumption) -= 2;           //decrement by 2
                if ((*powerConsumption)<=5) {
                    consumpUpDown = TRUE;
                }
            }
            else{                                   //on odd calls...
                (*powerConsumption) += 1;           //increment by 1
            }
        }

        unsigned long adcReading[1] = {0};

        // Below code adapted from temperature_sensor.c in IAR examples/peripherals
        // Clear the interrupt status flag.  This is done to make sure the
        // interrupt flag is cleared before we sample.
        ADCIntClear(ADC0_BASE, 3);

        // Trigger the ADC conversion.
        ADCProcessorTrigger(ADC0_BASE, 3);

        // Wait for conversion to be completed.
        while(!ADCIntStatus(ADC0_BASE, 3, false))
        {
        }

        // Clear the ADC interrupt flag.
        ADCIntClear(ADC0_BASE, 3);

        // Read ADC Value.
        ADCSequenceDataGet(ADC0_BASE, 3, adcReading);

        // convert adcReading from 3.0V to 36V scale
        // If ADC returns 10bit int (0-1023), each digit ~= 0.00293V
        // Then, multiply by 0.098 to convert to percentage of 36V
        unsigned int adcReadingConverted = (int) (adcReading[0] * 0.098);
        // move previous readings to next array slot
        for (int i = 0; i < 15; ++i)
        {
            battLevel[i+1] = battLevel[i];
        }
        // Add new reading to front of circular buffer
        battLevel[0] = adcReadingConverted;

        vTaskDelay(100);
    }
}

void solarPanelControl(void* taskDataPtr)
{
    solarPanelStruct* solarPanelPtr = (solarPanelStruct*) taskDataPtr;
    Bool* isMajorCycle = (Bool*) solarPanelPtr->isMajorCyclePtr;
    Bool* panelState = (Bool*) solarPanelPtr->panelStatePtr;
    Bool* panelDeploy = (Bool*) solarPanelPtr->panelDeployPtr;
    Bool* panelRetract = (Bool*) solarPanelPtr->panelRetractPtr;
    Bool* panelMotorSpeedUp = (Bool*) solarPanelPtr->panelMotorSpeedUpPtr;
    Bool* panelMotorSpeedDown = (Bool*) solarPanelPtr->panelMotorSpeedDownPtr;
    unsigned short* globalCount = (unsigned short*) solarPanelPtr->globalCountPtr;
    
    while(1)
    {
        
        // Compute the PWM period based on the system clock.
        // Base clock 8MHz, want 2Hz for panel motor ( / 4000000)
        unsigned long ulPeriod = SysCtlClockGet() / 80; //run at 2Hz
        static unsigned long dutyCycle = 5;                                                                             //changed because it's annoying
        
        // Read keypad input and adjust duty cycle based on keypress
        if (GPIOPinRead(GPIO_PORTB_BASE, GPIO_PIN_2)){
            dutyCycle += 5;
        }
        if (GPIOPinRead(GPIO_PORTB_BASE, GPIO_PIN_3)){
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
        if(panelDeploy||panelRetract){
            PWMGenEnable(PWM0_BASE, PWM_GEN_0);
        }
        
        else if(panelDone){
            PWMPulseWidthSet(PWM0_BASE, PWM_OUT_0, 0);
            PWMGenEnable(PWM0_BASE, PWM_GEN_0);
        }
        PWMGenEnable(PWM0_BASE, PWM_GEN_0);
        vTaskDelay(100);
    }
}

void consoleKeyboard(void* taskDataPtr)
{
    keyboardDataStruct* keyboardData = (keyboardDataStruct*) taskDataPtr;
    Bool* panelMotorSpeedUp = (Bool*) keyboardData->panelMotorSpeedUpPtr;
    Bool* panelMotorSpeedDown = (Bool*) keyboardData->panelMotorSpeedDownPtr;
  
    while(1)
    {
        // Read keypad input and adjust duty cycle based on keypress
        if (GPIOPinRead(GPIO_PORTB_BASE, GPIO_PIN_2)){
            *panelMotorSpeedUp = TRUE;
        }
        else {
            *panelMotorSpeedUp = FALSE;
        }
        
        if (GPIOPinRead(GPIO_PORTB_BASE, GPIO_PIN_3)){
            *panelMotorSpeedDown = TRUE;
        }
        else {
            *panelMotorSpeedDown = FALSE;
        }
        vTaskDelay(100);
    }
}

// Communication only store the command without decoding it
// Require : satellite communication data struct,
//           randomInteger function;
// Modifies: thrust command.
void satelliteComms(void* taskDataPtr)
{
    satelliteCommsDataStruct* commPtr = (satelliteCommsDataStruct*) taskDataPtr;
    unsigned short* globalCount = (unsigned short*) commPtr->globalCountPtr;
    Bool* isMajorCycle = (Bool*) commPtr->isMajorCyclePtr;
    Bool* fuelLowSignal = (Bool*)commPtr->fuelLowPtr;
    Bool* battLowSignal = (Bool*)commPtr->battLowPtr;
    unsigned short* battLevelSignal = (unsigned short*)commPtr->battLevelPtr;
    uint32_t* fuelLevelSignal = (uint32_t*)commPtr->fuelLevelPtr;
    unsigned short* powerConsumptionSignal = (unsigned short*)commPtr->powerConsumptionPtr;
    unsigned short* powerGenerationSignal = (unsigned short*)commPtr->powerGenerationPtr;
    Bool* panelStateSignal = (Bool*)commPtr->panelStatePtr; 
    
    while(1)
    {    
        if (isMajorCycle)
        {
            // receive (rando) thrust commands, generate from 0 to 2^16 -1
            uint16_t thrustCommand = randomInteger(globalCount);
            // uint16_t thrustCommand = 0x0FF1;
            *(uint16_t*)(commPtr->thrustPtr) = thrustCommand;
        }    
        vTaskDelay(100);    
    }
}


void vehicleComms(void* taskDataPtr)
{
    vehicleCommsStruct* dataPtr = (vehicleCommsStruct*) taskDataPtr;
    unsigned char* vehicleCommandLocal = (unsigned char*) dataPtr->vehicleCommandPtr;
    unsigned char* vehicleResponseLocal = (unsigned char*) dataPtr->vehicleResponsePtr;
    
    while(1)
    {
        // Clean up the screen
        UARTSend((unsigned char *)"\033[2J", 6);
        
        vehicleResponseLocal[0] = 'A';
        vehicleResponseLocal[1] = ' ';

        // Receive command
        if(hasNewKeyboardInput)
        {
            if ('F' == *vehicleCommandLocal || 'B' == *vehicleCommandLocal ||
                'L' == *vehicleCommandLocal || 'R' == *vehicleCommandLocal ||
                'D' == *vehicleCommandLocal || 'H' == *vehicleCommandLocal ||
                'T' == *vehicleCommandLocal || 'D' == *vehicleCommandLocal ||
                'K' == *vehicleCommandLocal || 'C' == *vehicleCommandLocal) {
                vehicleResponseLocal[0] = 'A';
                UARTSend((unsigned char *)"Valid Command:", 14);
            } else {
                vehicleResponseLocal[0] = '-';
                UARTSend((unsigned char *)"Invalid Command:", 16);
            }
            hasNewKeyboardInput = FALSE;
            UARTSend((unsigned char*)vehicleResponseLocal, 3);
        }   else {
            UARTSend((unsigned char *)"Enter Command:", 14);
        }      

        vTaskDelay(100);
    }
}

// Require : the minor clock running at 1 second per cycle, 
//          and thruster sub data struct; 
// Modifies: global constant fuelLevel, taking in count of duration.
void thrusterSub(void* taskDataPtr)
{
    thrusterSubDataStruct* thrustCommandPtr = (thrusterSubDataStruct*) taskDataPtr; 
    // unsigned short* globalCount = (unsigned short*) thrustCommandPtr->globalCountPtr;
    Bool* isMajorCycle = (Bool*) thrustCommandPtr->isMajorCyclePtr;
    uint32_t* fuelPtr = (uint32_t*) thrustCommandPtr->fuelLevelPtr;
    uint16_t command = *(uint16_t*)(thrustCommandPtr->thrustPtr);
    static unsigned short left = 0;
    static unsigned short right = 0;
    static unsigned short up = 0;
    static unsigned short down = 0;
    static unsigned short magnitude = 0;
    static unsigned short duration = 0;
    
    while(1) 
    {
        // Only updates the magnitude when the duration is positive, otherwise no change on magnitude
        if (*isMajorCycle)
        {
            
            // get duration, unsigned char pointer moves every 8 bits
            // so it can separate first 8 bits and last 8 bits
            duration = *((unsigned char*)&command + 1);
            if (duration) { magnitude = (command >> 4) & 0x000F; }
            if (command & 0x0001) 
            { 
              left  = 1; 
            } 
            else 
            { 
              left  = 0;
            }
            if (command & 0x0002) { right = 1; } else { right = 0;}
            if (command & 0x0004) { up    = 1; } else { up    = 0;}
            if (command & 0x0008) { down  = 1; } else { down  = 0;}
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

        vTaskDelay(100);      
    }
}

void oledDisplay(void* taskDataPtr)
{
    oledDisplayDataStruct* dataPtr = (oledDisplayDataStruct*) taskDataPtr;
        
    unsigned int* battLevel = (unsigned int*) dataPtr->battLevelPtr;
    uint32_t* fuelLevelPtr2 = (uint32_t*) dataPtr->fuelLevelPtr;
    unsigned short* powerConsumption = (unsigned short*) dataPtr->powerConsumptionPtr;
    Bool* panelState = (Bool*) dataPtr->panelStatePtr;
    Bool* fuelLow = (Bool*) dataPtr->fuelLowPtr;
    Bool* battLow = (Bool*) dataPtr->battLowPtr;
    // uint32_t* battTemp = (uint32_t*) dataPtr->battTempPtr;
    // unsigned long* distance = (unsigned long*) dataPtr->transportDistancePtr;
  
    xOLEDMessage xMsgTest;
    xOLEDMessage xMsgPanelState;
    xOLEDMessage xMsgBattLev;
    xOLEDMessage xMsgBattTemp;
    xOLEDMessage xMsgFuelLev;
    xOLEDMessage xMsgDist;
    xOLEDMessage xMsgFuelLow;
    xOLEDMessage xMsgBattLow;
    
    while(1) 
    {
        xMsgTest.pcMessage = "Hello, World!";
        //Send the message to the OLED gatekeeper for display.
        xQueueSend( xOLEDQueue, &xMsgTest, 0 );
        // Push select button to change to annunciation mode on OLED
        long buttonRead = 2;
        char buffer [100];

        // Pushed = 0, released = 2 from our measurement
        buttonRead = GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_1);
     
        if (2 == buttonRead)    // Status display mode
        {
            RIT128x96x4Clear();

        // Display panel state.
        if (1== *panelState)
        {
            xMsgPanelState.pcMessage = "Panel State: Deployed";
        }
        else
        {
            xMsgPanelState.pcMessage = "Panel State: Retracted";
        }
        xQueueSend( xOLEDQueue, &xMsgPanelState, 0 );
        
        // Display battery level
        snprintf(buffer, 20, "Battery Level: %d", *battLevel);
        xMsgBattLev.pcMessage = buffer;
        xQueueSend( xOLEDQueue, &xMsgBattLev, 0 );

        // Display battery temperature
        //snprintf(buffer, 20, "Battery Temp: %d", *battTemp);
        //xMsgBattTemp.pcMessage = buffer;
        xQueueSend( xOLEDQueue, &xMsgBattTemp, 0 );

        // Display fuel level
        snprintf(buffer, 20, "Fuel Level: %d", fuelLevellll);
        xMsgFuelLev.pcMessage = buffer;
        xQueueSend( xOLEDQueue, &xMsgFuelLev, 0 );

        // Display transport distance ptr
        //snprintf(buffer, 20, "Transport Distance: %d", *distance);
        //xMsgDist.pcMessage = buffer;
        xQueueSend( xOLEDQueue, &xMsgDist, 0 );

          
        } else if (0 == buttonRead) // Annunciation mode
        {
            RIT128x96x4Clear();
            
            // Display fuel low flag.
            if (1 == *fuelLow)
            {
                xMsgFuelLow.pcMessage = "FUEL LOW!";
            }
            else
            {
                xMsgFuelLow.pcMessage = "Fuel Good :)";
            }
            xQueueSend( xOLEDQueue, &xMsgFuelLow, 0 );


            // Display battery low flag.
            if (1 == *battLow)
            {
                xMsgBattLow.pcMessage = "BATTERY LOW!";
            }
            else
            {
                xMsgBattLow.pcMessage = "Battery Good :)";
            }
            xQueueSend( xOLEDQueue, &xMsgBattLow, 0 );
        }        
        
        vTaskDelay(100);
    }
}

// Require : warning alarm data struct;
// Modifies: fuelLow pointer, battLow pointer.
void warningAlarm(void* taskDataPtr)
{
    warningAlarmDataStruct* dataPtr = (warningAlarmDataStruct*) taskDataPtr;
    Bool* fuelLow = (Bool*)dataPtr->fuelLowPtr;
    Bool* battLow = (Bool*)dataPtr->battLowPtr;
    unsigned int* battLevel = (unsigned int*)dataPtr->battLevelPtr;
    uint32_t* fuelLevel = (uint32_t*)dataPtr->fuelLevelPtr;
    unsigned short* globalCount = (unsigned short*) dataPtr->globalCountPtr;
    Bool* isMajorCycle = (Bool*) dataPtr->isMajorCyclePtr;
        
    while(1) 
    {  
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
        vTaskDelay(100);
    } 
}