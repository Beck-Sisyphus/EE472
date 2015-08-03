// Include Statements
#include <stdio.h>
#include <stdlib.h>
#include "lab3.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"

// Define some Constants
const unsigned short MAX_BATT_LEVEL = 100;
const unsigned short HALF_BATT_LEVEL = 50;
const unsigned short BATT_WARN_LEVEL = 10;
// 6 month has x = 6*30*24*60*60 seconds, and 100% has y = x/20 seconds, 
// since the magnitude command has 4 bit, aka the consumption ranks from 0 to 15, 
// setting a total fuel level as 15 * y helps to calculate the fuel easier
// Assuming the minor cycle is running at 1 second per cycle rate.
const uint32_t MAX_FUEL_LEVEL = 11664000;
const uint32_t HALF_FUEL_LEVEL = 5832000; // at 50% level
const uint32_t FUEL_WARN_LEVEL = 1166400; // below 10% is warnning level

// Define Global Variables storing status data
unsigned int* battLevelPtr;
unsigned int batteryLevelArray[16] = {100};
uint32_t fuelLevel;
unsigned short powerConsumption;
unsigned short powerGeneration;
Bool panelState;
Bool panelDeploy;
Bool panelRetract;
Bool panelMotorSpeedUp;
Bool panelMotorSpeedDown;
Bool panelDone;
// 16bit encoded thrust command [15:8]Duration,[7:4]Magnitude,[3:0]Direction
uint16_t thrust; 
Bool fuelLow;
Bool battLow;
Bool isMajorCycle;

// Added for lab 3 communication
unsigned char vehicleCommand;
unsigned char vehicleResponse[3];
Bool hasNewKeyboardInput;
// If true, task should be inserted into task queue if not already present
// If false, task should be removed from task queue if present
// Keypad task is also tied
Bool panelAndKeypadTask;

// Global variable created for passing data through different function safer
unsigned short globalCount;
unsigned short blinkTimer;
uint32_t fuelLevellll;

int main()
{
	enableOLED();
	enableGPIO();
	enableADC();
	enableUART();
	enableTimer();
	initializeGlobalVariables();


	// Define Data Structs
	powerSubDataStruct powerSubData           	= {&panelState, &panelDeploy, &panelRetract, &battLevelPtr, &powerConsumption, &powerGeneration};
	solarPanelStruct solarPanelData           	= {&panelState, &panelDeploy, &panelRetract, &panelMotorSpeedUp, &panelMotorSpeedDown, &globalCount, &isMajorCycle};
	satelliteCommsDataStruct satelliteCommsData	= {&fuelLow, &battLow, &panelState, &battLevelPtr, &fuelLevel, &powerConsumption, &powerGeneration, &thrust, &globalCount, &isMajorCycle};
	thrusterSubDataStruct thrusterSubData     	= {&thrust, &fuelLevel, &globalCount, &isMajorCycle};
	vehicleCommsStruct vehicleCommsData       	= {&vehicleCommand, &vehicleResponse, &globalCount, &isMajorCycle};
	oledDisplayDataStruct oledDisplayData     	= {&fuelLow, &battLow, &panelState, &battLevelPtr, &fuelLevel, &powerConsumption, &powerGeneration, &globalCount, &isMajorCycle};
	keyboardDataStruct keyboardData           	= {&panelMotorSpeedUp, &panelMotorSpeedDown};
	warningAlarmDataStruct warningAlarmData   	= {&fuelLow, &battLow, &battLevelPtr, &fuelLevel, &globalCount, &isMajorCycle};
	scheduleDataStruct scheduleData           	= {&globalCount, &isMajorCycle};

	// Define TCBs
	TCB powerSubTCB;
	TCB solarPanelTCB;
	TCB satelliteCommsTCB;
	TCB thrusterSubTCB;
	TCB vehicleCommsTCB;
	TCB oledDisplayTCB;
	TCB keyboardDataTCB;
	TCB warningAlarmTCB;
	TCB* TCBPtr; //ptr to active TCB

	// Populate TCBs
	powerSubTCB.taskDataPtr = (void*)&powerSubData;
	powerSubTCB.taskPtr = powerSub;
	powerSubTCB.next = NULL;
	powerSubTCB.prev = NULL;

	solarPanelTCB.taskDataPtr = (void*)&solarPanelData;
	solarPanelTCB.taskPtr = solarPanelControl;
	solarPanelTCB.next = NULL;
	solarPanelTCB.prev = NULL;

	satelliteCommsTCB.taskDataPtr = (void*)&satelliteCommsData;
	satelliteCommsTCB.taskPtr = satelliteComms;
	satelliteCommsTCB.next = NULL;
	satelliteCommsTCB.prev = NULL;

	thrusterSubTCB.taskDataPtr = (void*)&thrusterSubData;
	thrusterSubTCB.taskPtr = thrusterSub;
	thrusterSubTCB.next = NULL;
	thrusterSubTCB.prev = NULL;

	vehicleCommsTCB.taskDataPtr = (void*)&vehicleCommsData;
	vehicleCommsTCB.taskPtr = vehicleComms;
	vehicleCommsTCB.next = NULL;
	vehicleCommsTCB.prev = NULL;

	oledDisplayTCB.taskDataPtr = (void*)&oledDisplayData;
	oledDisplayTCB.taskPtr = oledDisplay;
	oledDisplayTCB.next = NULL;
	oledDisplayTCB.prev = NULL;

	keyboardDataTCB.taskDataPtr = (void*)&keyboardData;
	keyboardDataTCB.taskPtr = consoleKeyboard;
	keyboardDataTCB.next = NULL;
	keyboardDataTCB.prev = NULL;

	warningAlarmTCB.taskDataPtr = (void*)&warningAlarmData;
	warningAlarmTCB.taskPtr = warningAlarm;
	warningAlarmTCB.next = NULL;
	warningAlarmTCB.prev = NULL;


	// Task Queue head and tail pointers
	TCB* taskQueueHead = NULL;
	TCB* taskQueueTail = NULL;

	insertTask(&powerSubTCB, &taskQueueHead, &taskQueueTail);
	insertTask(&satelliteCommsTCB, &taskQueueHead, &taskQueueTail);
	insertTask(&thrusterSubTCB, &taskQueueHead, &taskQueueTail);
	insertTask(&vehicleCommsTCB, &taskQueueHead, &taskQueueTail);
	insertTask(&oledDisplayTCB, &taskQueueHead, &taskQueueTail);
	insertTask(&warningAlarmTCB, &taskQueueHead, &taskQueueTail);

    // Run... forever!!!
    while(1)
    {
    	// Get pointer to first task
    	TCBPtr = taskQueueHead;

    	// Adds/deletes solarPanel and keypad task as necessary
    	if (panelAndKeypadTask)
    	{
    		insertTask(&keyboardDataTCB, &taskQueueHead, &taskQueueTail);
    		insertTask(&solarPanelTCB, &taskQueueHead, &taskQueueTail);
    	}
    	else 
    	{
    		deleteTask(&keyboardDataTCB, &taskQueueHead, &taskQueueTail);
    		deleteTask(&solarPanelTCB, &taskQueueHead, &taskQueueTail);
    	}
        
        // Loop through task queue & perform each task
    	while (TCBPtr != NULL)
    	{
	        TCBPtr->taskPtr( (TCBPtr->taskDataPtr) );
	        TCBPtr = TCBPtr->next;
    	}

    	schedule(scheduleData);
    }
    return EXIT_SUCCESS;
}

void initializeGlobalVariables()
{
	// Initialization 
	battLevelPtr = &batteryLevelArray[0]; // equivalent to = &battLevel[0]

	fuelLevel = MAX_FUEL_LEVEL;
	powerConsumption = 0;
	powerGeneration = 0;
	panelState = FALSE;
	panelDeploy = FALSE;
	panelRetract = FALSE;
	panelMotorSpeedUp = FALSE;
	panelMotorSpeedDown = FALSE;

	thrust = 0;
	fuelLow = FALSE;
	battLow = FALSE;

	// vehicleCommand = NULL; Can't initialize as NULL, just left it as its default
	// vehicleResponse = NULL;
	hasNewKeyboardInput = FALSE;

	isMajorCycle = TRUE;
	globalCount = 0;
	blinkTimer = 0;
}

void deleteTask(TCB* node, TCB** head, TCB** tail) 
{
	// Node is not in list
	if (NULL == node->next && NULL == node->prev)
	{
		return;
	}
	// List is empty
	else if (NULL == head)
	{
		return;
	} 
	// List has 1 node
	else if (*head == *tail)
	{
		// If node is this node, delete it
		if (*head == node)
		{
			*head = NULL;
			*tail = NULL;
		}
	}
	// Head node is node to delete
	else if (*head == node) 
	{
		*head = node->next;
		// node->next = node->prev;
                (*head)->prev = NULL;
		node->prev = NULL;
		node->next = NULL;
	}
	// If tail node is node to delete
	else if (*tail == node)
	{
		*tail = node->prev;
		(*tail)->next = NULL;
		node->prev = NULL;
                node->next = NULL;
	}
	// Node is in the middle, just need to update prevs and nexts of it
	//  and neighbor node(s)
	else
	{
		node->next->prev = node->prev;
		node->prev->next = node->next;
		node->next = NULL;
		node->prev = NULL;
	}
	return;
}

void insertTask(TCB* node, TCB** head, TCB** tail)
	{
	if(NULL == (*head)) // If the head pointer is pointing to nothing
	{
		*head = node; // set the head and tail pointers to point to this node
		*tail = node;
                (*head)->next = NULL;
                (*head)->prev = NULL;
	}
	else // otherwise, head is not NULL, add the node to the end of the list
	{
                // Check if the node is in the list already	
                TCB* temp = *tail;
                while (temp->prev != NULL) {
                  if (temp == node) {
                     return;
                  }
                  temp = temp->prev;
                }
                
                (*tail)->next = node;
		node->prev = *tail; // note that the tail pointer is still pointing
		// to the prior last node at this point
		*tail = node; // update the tail pointer
	}
	// Always set node next pointer to null for end of list
	node->next = NULL;
	return;
}
