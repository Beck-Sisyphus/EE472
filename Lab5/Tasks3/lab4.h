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
	typedef struct transportDataStruct {
		unsigned short* globalCountPtr;
	}	transportDataStruct;

	typedef struct scheduleDataStruct {
		unsigned short* globalCountPtr;
		Bool* isMajorCyclePtr;
                Bool* battOverTempPtr;
	} scheduleDataStruct;
        
	typedef struct pirateDataStruct {
		unsigned short* pirateProximity;
	}	pirateDataStruct;
        
        typedef struct powerSubDataStruct {
		Bool* panelStatePtr;
		Bool* panelDeployPtr;
		Bool* panelRetractPtr;
		unsigned int* battLevelPtr;
		unsigned int* battTempPtr0;
		unsigned int* battTempPtr1;
		Bool* battOverTempPtr;
		unsigned short* powerConsumptionPtr;
		unsigned short* powerGenerationPtr;
	} powerSubDataStruct;

	typedef struct solarPanelStruct {
		Bool* panelStatePtr;
		Bool* panelDeployPtr;
		Bool* panelRetractPtr;
		Bool* panelMotorSpeedUpPtr;
		Bool* panelMotorSpeedDownPtr;
		unsigned short* globalCountPtr;
		Bool* isMajorCyclePtr;
	} solarPanelStruct;

	typedef struct keyboardDataStruct {
		Bool* panelMotorSpeedUpPtr;
		Bool* panelMotorSpeedDownPtr;
	} keyboardDataStruct;
	
	typedef struct satelliteCommsDataStruct {
		Bool* fuelLowPtr;
		Bool* battLowPtr;
		Bool* panelStatePtr;
		Bool* battLevelPtr;
		unsigned int* battTempPtr0;
		unsigned int* battTempPtr1;
		uint32_t* fuelLevelPtr;
		unsigned short* powerConsumptionPtr;
		unsigned short* powerGenerationPtr;
		unsigned long* thrustPtr;
		unsigned short* globalCountPtr;
		Bool* isMajorCyclePtr;
	} satelliteCommsDataStruct;

	typedef struct thrusterSubDataStruct {
		unsigned long* thrustPtr;
		uint32_t* fuelLevelPtr;
		unsigned short* globalCountPtr;
		Bool* isMajorCyclePtr;
	} thrusterSubDataStruct;

	typedef struct vehicleCommsStruct {
		unsigned char* vehicleCommandPtr;
		unsigned char* vehicleResponsePtr;
		unsigned short* globalCountPtr;
		Bool* isMajorCyclePtr;
	} vehicleCommsStruct;

	typedef struct oledDisplayDataStruct {
		Bool* fuelLowPtr;
		Bool* battLowPtr;
		Bool* panelStatePtr;
		Bool* panelDeployPtr;
		Bool* panelRetractPtr;
		unsigned int* battLevelPtr;
		unsigned int* battTempPtr0;
		unsigned int* battTempPtr1;
		uint32_t* fuelLevelPtr;
		unsigned short* powerConsumptionPtr;
		unsigned short* powerGenerationPtr;
		unsigned short* globalCountPtr;
		Bool* isMajorCyclePtr;
		unsigned int* transportDistancePtr;
                //double* frequencyPtr;
	} oledDisplayDataStruct;

	typedef struct warningAlarmDataStruct {
		Bool* fuelLowPtr;
		Bool* battLowPtr;
		unsigned int* battLevelPtr;
                Bool* battOverTempPtr;
		uint32_t* fuelLevelPtr;
		unsigned short* globalCountPtr;
		Bool* isMajorCyclePtr;
	} warningAlarmDataStruct;
        
        typedef struct imageCaptureDataStruct {		
		//unsigned int* rawDataPtr;		
		signed int* processedDataPtr;		
		double* frequencyPtr;		
	} imageCaptureDataStruct;
        
	//Function Prototypes
    void enableSysClock();
	void enableOLED();
	void enableGPIO();
	void enableADC();
	void enableUART();
	void enableTimer();
	void initializeGlobalVariables();
	void UARTSend(const unsigned char *pucBuffer, unsigned long ulCount);
	void debugDataCorruptionSponge(void* taskDataPtr);
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
        void imageCapture(void* taskDataPtr);
        void pirates(void* taskDataPtr);
        
	void Timer0IntHandler(void);
        
	int max(int, int);
	unsigned long randomInteger(int, int);

#endif