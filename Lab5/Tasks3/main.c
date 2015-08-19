/*
 * Creates all the application tasks, then starts the scheduler.  The WEB
 * documentation provides more details of the standard demo application tasks.
 * In addition to the standard demo tasks, the following tasks and tests are
 * defined and/or created within this file:
 *
 * "OLED" task - the OLED task is a 'gatekeeper' task.  It is the only task that
 * is permitted to access the display directly.  Other tasks wishing to write a
 * message to the OLED send the message on a queue to the OLED task instead of
 * accessing the OLED themselves.  The OLED task just blocks on the queue waiting
 * for messages - waking and displaying the messages as they arrive.
 *
 * "Check" hook -  This only executes every five seconds from the tick hook.
 * Its main function is to check that all the standard demo tasks are still
 * operational.  Should any unexpected behaviour within a demo task be discovered
 * the tick hook will write an error to the OLED (via the OLED task).  If all the
 * demo tasks are executing with their expected behaviour then the check task
 * writes PASS to the OLED (again via the OLED task), as described above.
 *
 * "uIP" task -  This is the task that handles the uIP stack.  All TCP/IP
 * processing is performed in this task.
 */




/*************************************************************************
 * Please ensure to read http://www.freertos.org/portlm3sx965.html
 * which provides information on configuring and running this demo for the
 * various Luminary Micro EKs.
 *************************************************************************/

/* Set the following option to 1 to include the WEB server in the build.  By
default the WEB server is excluded to keep the compiled code size under the 32K
limit imposed by the KickStart version of the IAR compiler.  The graphics
libraries take up a lot of ROM space, hence including the graphics libraries
and the TCP/IP stack together cannot be accommodated with the 32K size limit. */

//  set this value to non 0 to include the web server

#define mainINCLUDE_WEB_SERVER      1


/* Standard includes. */
#include <stdio.h>
#include <stdlib.h>

/* Lab 4 parts includes. */
#include "lab4.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

/* Hardware library includes. */
#include "hw_memmap.h"
#include "hw_types.h"
#include "hw_sysctl.h"
#include "sysctl.h"
#include "gpio.h"
#include "grlib.h"
#include "rit128x96x4.h"
#include "osram128x64x4.h"
#include "formike128x128x16.h"

/* Demo app includes. */

#include "lcd_message.h"
#include "bitmap.h"


/*-----------------------------------------------------------*/

// Lab 4 parts that we wrote 

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
unsigned int batteryLevelArray[16] = {100};
unsigned int battTempArray0[16] = {0};
unsigned int battTempArray1[16] = {0};
//unsigned int rawImageData[256] = {0};		
signed int processedImageData[256] = {0};		
double imageFrequency;
unsigned long transportTimeArray[2] = {0};
unsigned long transportTimeTicks = 0;
double frequency = 0;
unsigned long transportDistance = 0;
uint32_t fuelLevel;
unsigned short powerConsumption;
unsigned short powerGeneration;
unsigned short pirateProximity;
Bool battOverTemp;
Bool panelState;
Bool panelDeploy;
Bool panelRetract;
Bool panelMotorSpeedUp;
Bool panelMotorSpeedDown;
Bool panelDone;
xTaskHandle solarPanelHandle;
xTaskHandle consoleKeyboardHandle;
xTaskHandle pirateHandle;
xTaskHandle imageCaptureHandle;

// 16bit encoded thrust command [15:8]Duration,[7:4]Magnitude,[3:0]Direction
unsigned long thrust; 
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
unsigned long pulsecount;
unsigned short blinkTimer8;
unsigned short blinkTimer10;
unsigned short tempAlarm;
uint32_t fuelLevellll;

/*--------------------------------------------------------*/

/* 
  The time between cycles of the 'check' functionality (defined within the
  tick hook. 
*/
#define mainCHECK_DELAY ( ( portTickType ) 5000 / portTICK_RATE_MS )

// Size of the stack allocated to the uIP task.
#define mainBASIC_WEB_STACK_SIZE            ( configMINIMAL_STACK_SIZE * 3 )

// The OLED task uses the sprintf function so requires a little more stack too.
#define mainOLED_TASK_STACK_SIZE      ( configMINIMAL_STACK_SIZE + 50 )

//  Task priorities.
#define mainQUEUE_POLL_PRIORITY       ( tskIDLE_PRIORITY + 2 )
#define mainCHECK_TASK_PRIORITY       ( tskIDLE_PRIORITY + 3 )
#define mainSEM_TEST_PRIORITY       ( tskIDLE_PRIORITY + 1 )
#define mainBLOCK_Q_PRIORITY        ( tskIDLE_PRIORITY + 2 )
#define mainCREATOR_TASK_PRIORITY           ( tskIDLE_PRIORITY + 3 )
#define mainINTEGER_TASK_PRIORITY           ( tskIDLE_PRIORITY )
#define mainGEN_QUEUE_TASK_PRIORITY     ( tskIDLE_PRIORITY )


//  The maximum number of messages that can be waiting for display at any one time.
#define mainOLED_QUEUE_SIZE         ( 10 )

// Dimensions the buffer into which the jitter time is written. 
#define mainMAX_MSG_LEN           25

/* 
  The period of the system clock in nano seconds.  This is used to calculate
  the jitter time in nano seconds. 
*/

#define mainNS_PER_CLOCK ( ( unsigned portLONG ) ( ( 1.0 / ( double ) configCPU_CLOCK_HZ ) * 1000000000.0 ) )


// Constants used when writing strings to the display.

#define mainCHARACTER_HEIGHT        ( 9 )
#define mainMAX_ROWS_128        ( mainCHARACTER_HEIGHT * 14 )
#define mainMAX_ROWS_96         ( mainCHARACTER_HEIGHT * 10 )
#define mainMAX_ROWS_64         ( mainCHARACTER_HEIGHT * 7 )
#define mainFULL_SCALE          ( 15 )
#define ulSSI_FREQUENCY         ( 3500000UL )

/*-----------------------------------------------------------*/
/* The task that handles the uIP stack.  All TCP/IP processing is performed in
* this task.
*/
extern void vuIP_Task( void *pvParameters );
    
/*
 * The display is written two by more than one task so is controlled by a
 * 'gatekeeper' task.  This is the only task that is actually permitted to
 * access the display directly.  Other tasks wanting to display a message send
 * the message to the gatekeeper.
 */

static void vOLEDTask( void *pvParameters );

/*
 * Configure the hardware .
 */
static void prvSetupHardware( void );

/*
 * Configures the high frequency timers - those used to measure the timing
 * jitter while the real time kernel is executing.
 */
extern void vSetupHighFrequencyTimer( void );

/*
 * Hook functions that can get called by the kernel.
 */
void vApplicationStackOverflowHook( xTaskHandle *pxTask, signed portCHAR *pcTaskName );
void vApplicationTickHook( void );
void initializeGlobalVariables( void );

/*-----------------------------------------------------------*/

/* 
  The queue used to send messages to the OLED task. 
*/
xQueueHandle xOLEDQueue;

/*-----------------------------------------------------------*/


/*************************************************************************
 * Please ensure to read http://www.freertos.org/portlm3sx965.html
 * which provides information on configuring and running this demo for the
 * various Luminary Micro EKs.
 *************************************************************************/

int main( void )
{
    prvSetupHardware();

    /*  
        Create the queue used by the OLED task.  Messages for display on the OLED
        are received via this queue. 
    */
    
    xOLEDQueue = xQueueCreate( mainOLED_QUEUE_SIZE, sizeof( xOLEDMessage ) );

    initializeGlobalVariables();
    
    //Depricated, but included out of love <3
    //DebugWorkGodDamnit DebugWork                = {&panelState, &panelDeploy, &panelRetract, &batteryLevelArray, &battTempArray0, &battTempArray1, &battOverTemp, &powerConsumption, &powerGeneration};
    
    satelliteCommsDataStruct satelliteCommsData = {&fuelLow, &battLow, &panelState, &batteryLevelArray, &battTempArray0, &battTempArray1, &fuelLevel, &powerConsumption, &powerGeneration, &thrust, &globalCount, &isMajorCycle};
    thrusterSubDataStruct thrusterSubData       = {&thrust, &fuelLevel, &globalCount, &isMajorCycle};
    vehicleCommsStruct vehicleCommsData         = {&vehicleCommand, &vehicleResponse, &globalCount, &isMajorCycle};
    oledDisplayDataStruct oledDisplayData       = {&fuelLow, &battLow, &panelState, &panelDeploy, &panelRetract, &batteryLevelArray, &battTempArray0, &battTempArray1, &fuelLevel, &powerConsumption, &powerGeneration, &transportDistance, &globalCount, &isMajorCycle};
    keyboardDataStruct keyboardData             = {&panelMotorSpeedUp, &panelMotorSpeedDown};
    warningAlarmDataStruct warningAlarmData     = {&fuelLow, &battLow, &batteryLevelArray, &battOverTemp, &fuelLevel, &globalCount, &isMajorCycle};
    scheduleDataStruct scheduleData             = {&globalCount, &isMajorCycle, &battOverTemp};
    transportDataStruct transportData           = {&globalCount};
    powerSubDataStruct powerSubData             = {&panelState, &panelDeploy, &panelRetract, &batteryLevelArray, &battTempArray0, &battTempArray1, &battOverTemp, &powerConsumption, &powerGeneration};
    solarPanelStruct solarPanelData             = {&panelState, &panelDeploy, &panelRetract, &panelMotorSpeedUp, &panelMotorSpeedDown, &globalCount, &isMajorCycle};
    pirateDataStruct pirateData                 = {&pirateProximity};
    imageCaptureDataStruct imageCaptureData     = {&processedImageData, &imageFrequency};
    
    // Create Handles for temp tasks
    solarPanelHandle = NULL;
    consoleKeyboardHandle = NULL;
    pirateHandle = NULL;

    /* Start the tasks */
    
    xTaskCreate( vOLEDTask, ( signed portCHAR * ) "OLED", mainOLED_TASK_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL );
    xTaskCreate(schedule,          "schedule",          40, (void*)&scheduleData,       mainCHECK_TASK_PRIORITY - 1, NULL);
    xTaskCreate(powerSub,          "powerSub",          100,(void*)&powerSubData,       mainCHECK_TASK_PRIORITY, NULL);
    xTaskCreate(solarPanelControl, "solarPanelControl", 60, (void*)&solarPanelData,     mainCHECK_TASK_PRIORITY, &solarPanelHandle);         
    vTaskSuspend(solarPanelHandle);
    xTaskCreate(satelliteComms,    "satelliteComms",    60, (void*)&satelliteCommsData, mainCHECK_TASK_PRIORITY + 1, NULL);
    xTaskCreate(vehicleComms,      "vehicleComms",      50, (void*)&vehicleCommsData,   mainCHECK_TASK_PRIORITY, NULL);
    xTaskCreate(thrusterSub,       "thrusterSub",       60, (void*)&thrusterSubData,    mainCHECK_TASK_PRIORITY, NULL);
    xTaskCreate(oledDisplay,       "oledDisplay",       120,(void*)&oledDisplayData,    mainCHECK_TASK_PRIORITY, NULL);
    xTaskCreate(consoleKeyboard,   "consoleKeyboard",   60, (void*)&keyboardData,       mainCHECK_TASK_PRIORITY, &consoleKeyboardHandle);    
    vTaskSuspend(consoleKeyboardHandle);
    xTaskCreate(warningAlarm,      "warningAlarm",      100,(void*)&warningAlarmData,   mainCHECK_TASK_PRIORITY, NULL);
    xTaskCreate(transport,         "transport",         100,(void*)&transportData,      mainCHECK_TASK_PRIORITY, NULL);
    xTaskCreate(pirates,           "pirates",           100,(void*)&pirateData,         mainCHECK_TASK_PRIORITY, &pirateHandle);
    //vTaskSuspend(pirateHandle);
    xTaskCreate(imageCapture,      "imageCapture",      4000,(void*)&imageCaptureData,   2, &imageCaptureHandle);
    vTaskSuspend(imageCaptureHandle);
    
    #if mainINCLUDE_WEB_SERVER != 0
    {
      /* 
          Create the uIP task if running on a processor that includes a MAC and PHY. 
      */
      
      if( SysCtlPeripheralPresent( SYSCTL_PERIPH_ETH ) )
      {
          xTaskCreate( vuIP_Task, ( signed portCHAR * ) "uIP", mainBASIC_WEB_STACK_SIZE, NULL, mainCHECK_TASK_PRIORITY - 1, NULL );
      }
    }
    #endif
    /* 
      Configure the high frequency interrupt used to measure the interrupt
      jitter time. 
    */
    
    vSetupHighFrequencyTimer();

    /* 
      Start the scheduler. 
    */
    
    vTaskStartScheduler();

    /* Will only get here if there was insufficient memory to create the idle task. */
    
    return 0;
}

void initializeGlobalVariables()
{
  // Initialization
  battOverTemp = FALSE;

  transportDistance = 0;

  fuelLevel = MAX_FUEL_LEVEL;
  imageFrequency = 0.0;
  powerConsumption = 0;
  powerGeneration = 0;
  pirateProximity = 0;
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
  blinkTimer8 = 0;
  blinkTimer10 = 0;
  tempAlarm = 0;
  pulsecount = 0;
}

/*
  the OLED Task
*/

void vOLEDTask( void *pvParameters )
{
    xOLEDMessage xMessage;
    unsigned portLONG ulY, ulMaxY;
    static portCHAR cMessage[ mainMAX_MSG_LEN ];
    extern volatile unsigned portLONG ulMaxJitter;
    unsigned portBASE_TYPE uxUnusedStackOnEntry;
    const unsigned portCHAR *pucImage;

// Functions to access the OLED. 

    void ( *vOLEDInit )( unsigned portLONG ) = NULL;
    void ( *vOLEDStringDraw )( const portCHAR *, unsigned portLONG, unsigned portLONG, unsigned portCHAR ) = NULL;
    void ( *vOLEDImageDraw )( const unsigned portCHAR *, unsigned portLONG, unsigned portLONG, unsigned portLONG, unsigned portLONG ) = NULL;
    void ( *vOLEDClear )( void ) = NULL;
  
  
    vOLEDInit = RIT128x96x4Init;
    vOLEDStringDraw = RIT128x96x4StringDraw;
    vOLEDImageDraw = RIT128x96x4ImageDraw;
    vOLEDClear = RIT128x96x4Clear;
    ulMaxY = mainMAX_ROWS_96;
    pucImage = pucBasicBitmap;
              
// Just for demo purposes.
    uxUnusedStackOnEntry = uxTaskGetStackHighWaterMark( NULL );
  
    ulY = ulMaxY;
    
    /* Initialise the OLED  */
    vOLEDInit( ulSSI_FREQUENCY ); 
    
    while( 1 )
    {
      // Wait for a message to arrive that requires displaying.
      
      xQueueReceive( xOLEDQueue, &xMessage, portMAX_DELAY );
  
      // Write the message on the next available row. 
      
      ulY += mainCHARACTER_HEIGHT;
      if( ulY >= ulMaxY )
      {
          ulY = mainCHARACTER_HEIGHT;
         // vOLEDClear();
      }
  
      // Display the message  
                      
      sprintf( cMessage, "%s", xMessage.pcMessage);
      
      vOLEDStringDraw( cMessage, 0, ulY, mainFULL_SCALE );
      
  }
}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook( xTaskHandle *pxTask, signed portCHAR *pcTaskName )
{
    ( void ) pxTask;
    ( void ) pcTaskName;
  
    while( 1 );
}

/*-----------------------------------------------------------*/

void prvSetupHardware( void )
{
    /* 
      If running on Rev A2 silicon, turn the LDO voltage up to 2.75V.  This is
      a workaround to allow the PLL to operate reliably. 
    */
  
    // if( DEVICE_IS_REVA2 )
    // {
    //     SysCtlLDOSet( SYSCTL_LDO_2_75V );
    // }
  
    // // Set the clocking to run from the PLL at 50 MHz
    
    // SysCtlClockSet( SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_8MHZ );
    
    /*   
      Enable Port F for Ethernet LEDs
            LED0        Bit 3   Output
            LED1        Bit 2   Output 
    */
    
    SysCtlPeripheralEnable( SYSCTL_PERIPH_GPIOF );
    GPIODirModeSet( GPIO_PORTF_BASE, (GPIO_PIN_2 | GPIO_PIN_3), GPIO_DIR_MODE_HW );
    GPIOPadConfigSet( GPIO_PORTF_BASE, (GPIO_PIN_2 | GPIO_PIN_3 ), GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD ); 
  
    enableSysClock();
    enableGPIO();
    enableADC();
    enableUART();
}


/*-----------------------------------------------------------*/

void vApplicationTickHook( void )
{
    static xOLEDMessage xMessage = { "PASS" };
    static unsigned portLONG ulTicksSinceLastDisplay = 0;
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

    /* 
      Called from every tick interrupt.  Have enough ticks passed to make it
      time to perform our health status check again? 
    */
    
    ulTicksSinceLastDisplay++;
    if( ulTicksSinceLastDisplay >= mainCHECK_DELAY )
    {
       ulTicksSinceLastDisplay = 0;
            
    }
}

// Function to return max int of a or b
// Used to compute battery overtemp condition
int max(int a, int b)
{
  int result = a;
  if (a < b)
    result = b;
  // If a == b, return a
  return result;
}