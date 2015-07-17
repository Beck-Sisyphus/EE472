#ifndef HEADER_FILE //prevents header file from being included twice
#define HEADER_FILE
	
	//Define Boolean Values
	enum myBool { FALSE = 0, TRUE = 1 };
	typedef enum myBool Bool;

	//Function Prototypes
	void schedule();
	void powerSub(void* taskDataPtr);
	void thrusterSub(void* taskDataPtr);
	void satelliteComms(void* taskDataPtr);
	void oledDisplay(void* taskDataPtr);
	void warningAlarm(void* taskDataPtr);
	void delay_ms(int time_in_ms);
	uint16_t randomInteger(const unsigned short* globalCount);
        
	//Declare TCB Struct

	typedef struct TCB{
		void* taskDataPtr; 		  //ptr to generic data
		void (*taskPtr)(void*);    //ptr to generic function
	} TCB;

	//Declare DataStruct Structs
	typedef struct scheduleDataStruct{
		void* globalCountPtr;
		void* isMajorCyclePtr;
	} scheduleDataStruct;
        
	typedef struct powerSubDataStruct{
		void* panelStatePtr;
		void* battLevelPtr;
		void* powerConsumptionPtr;
		void* powerGenerationPtr;
		void* globalCountPtr;
		void* isMajorCyclePtr;
	} powerSubDataStruct;
	
	typedef struct thrusterSubDataStruct{
		void* thrustPtr;
		void* fuelLevelPtr;
		void* globalCountPtr;
		void* isMajorCyclePtr;
	} thrusterSubDataStruct;

	typedef struct satelliteCommsDataStruct{
		void* fuelLowPtr;
		void* battLowPtr;
		void* panelStatePtr;
		void* battLevelPtr;
		void* fuelLevelPtr;
		void* powerConsumptionPtr;
		void* powerGenerationPtr;
		void* thrustPtr;
		void* globalCountPtr;
		void* isMajorCyclePtr;
	} satelliteCommsDataStruct;

	typedef struct oledDisplayDataStruct{
		void* fuelLowPtr;
		void* battLowPtr;
		void* panelStatePtr;
		void* battLevelPtr;
		void* fuelLevelPtr;
		void* powerConsumptionPtr;
		void* powerGenerationPtr;
		void* globalCountPtr;
		void* isMajorCyclePtr;
	} oledDisplayDataStruct;

	typedef struct warningAlarmDataStruct{
		void* fuelLowPtr;
		void* battLowPtr;
		void* battLevelPtr;
		void* fuelLevelPtr;
		void* globalCountPtr;
		void* isMajorCyclePtr;
	} warningAlarmDataStruct;

#endif