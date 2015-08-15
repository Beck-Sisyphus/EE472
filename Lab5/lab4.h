// EE 472, University of Washington, Seattle
// Summer 2015, Beck Pang, Grant Taylor, Sean Happenny
// Public header for lab 4
#include <stdint.h>
#ifndef HEADER_FILE //prevents header file from being included twice
#define HEADER_FILE
	
	//Define Boolean Values
	enum myBool { FALSE = 0, TRUE = 1 };
	typedef enum myBool Bool;

	//Declare data structure structs

	typedef struct TCB {
		void* taskDataPtr; 		  //ptr to generic data
		void (*taskPtr)(void*);    //ptr to generic function
		struct TCB* next;
		struct TCB* prev;
	} TCB;

	typedef struct transportDataStruct {
		void* globalCountPtr;
	}	transportDataStruct;

	typedef struct scheduleDataStruct {
		void* globalCountPtr;
		void* isMajorCyclePtr;
	} scheduleDataStruct;
        
	typedef struct powerSubDataStruct {
		void* panelStatePtr;
		void* panelDeployPtr;
		void* panelRetractPtr;
		void* battLevelPtr;
		void* battTempPtr0;
		void* battTempPtr1;
		void* battOverTempPtr;
		void* powerConsumptionPtr;
		void* powerGenerationPtr;
	} powerSubDataStruct;

	typedef struct solarPanelStruct {
		void* panelStatePtr;
		void* panelDeployPtr;
		void* panelRetractPtr;
		void* panelMotorSpeedUpPtr;
		void* panelMotorSpeedDownPtr;
		void* globalCountPtr;
		void* isMajorCyclePtr;
	} solarPanelStruct;

	typedef struct keyboardDataStruct {
		void* panelMotorSpeedUpPtr;
		void* panelMotorSpeedDownPtr;
	} keyboardDataStruct;
	
	typedef struct satelliteCommsDataStruct {
		void* fuelLowPtr;
		void* battLowPtr;
		void* panelStatePtr;
		void* battLevelPtr;
		void* battTempPtr0;
		void* battTempPtr1;
		void* fuelLevelPtr;
		void* powerConsumptionPtr;
		void* powerGenerationPtr;
		void* thrustPtr;
		void* globalCountPtr;
		void* isMajorCyclePtr;
	} satelliteCommsDataStruct;

	typedef struct thrusterSubDataStruct {
		void* thrustPtr;
		void* fuelLevelPtr;
		void* globalCountPtr;
		void* isMajorCyclePtr;
	} thrusterSubDataStruct;

	typedef struct vehicleCommsStruct {
		void* vehicleCommandPtr;
		void* vehicleResponsePtr;
		void* globalCountPtr;
		void* isMajorCyclePtr;
	} vehicleCommsStruct;

	typedef struct oledDisplayDataStruct {
		void* fuelLowPtr;
		void* battLowPtr;
		void* panelStatePtr;
		void* panelDeployPtr;
		void* panelRetractPtr;
		void* battLevelPtr;
		void* battTempPtr0;
		void* battTempPtr1;
		void* fuelLevelPtr;
		void* powerConsumptionPtr;
		void* powerGenerationPtr;
		void* globalCountPtr;
		void* isMajorCyclePtr;
		void* transportDistancePtr;
	} oledDisplayDataStruct;

	typedef struct warningAlarmDataStruct {
		void* fuelLowPtr;
		void* battLowPtr;
		void* battLevelPtr;
		void* fuelLevelPtr;
		void* globalCountPtr;
		void* isMajorCyclePtr;
		void* battOverTempPtr;
	} warningAlarmDataStruct;
        
        
	//Function Prototypes
    void enableSysClock();
	void enableOLED();
	void enableGPIO();
	void enableADC();
	void enableUART();
	void enableTimer();
	void initializeGlobalVariables();
	void UARTSend(const unsigned char *pucBuffer, unsigned long ulCount);
	void insertTask(TCB* node, TCB** head, TCB** tail);
	void deleteTask(TCB* node, TCB** head, TCB** tail);
	unsigned short randomInteger(int, int);
	
	void schedule(void* taskDataPtr);
    void transport(void* taskDataPtr);
	void powerSub(void* taskDataPtr);
	void solarPanelControl(void* taskDataPtr);
	void satelliteComms(void* taskDataPtr);
	void vehicleComms(void* taskDataPtr);
	void thrusterSub(void* taskDataPtr);
	void oledDisplay(void* taskDataPtr);
	void consoleKeyboard(void* taskDataPtr);
	void warningAlarm(void* taskDataPtr);
	void Timer0IntHandler(void);

#endif