// Lab 2, EE 472, Summer 2015, University of Washington

#ifndef KERNELHEADER
#define KERNELHEADER 
enum myBool { FALSE = 0, TRUE = 1 };
typedef enum _myBool Bool;

typedef struct taskControlBlocks
	{
		void* taskDataPtr;
		void (*taskPtr)(void*);
	} TCB;

// For Thruster Control
unsigned int ThrusterCommand = 0;

// For Power Management and Status Management and Annunciation
unsigned short BatteryLevel  = 100;
unsigned short FuelLevel     = 100;
unsigned short PowerConsumption = 0;
unsigned short PowerGeneration = 0;

// For Solar Panel Control and Status Management and Annunciation
Bool SolarPanelState = FALSE;


#endif