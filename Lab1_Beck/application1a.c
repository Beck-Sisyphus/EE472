// Beck Pang, Summer 2015, EE 472.

#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/sysctl.h"
#include "drivers/rit128x96x4.h"

//*****************************************************************************
//
// The error routine that is called if the driver library encounters an error.
//
//*****************************************************************************
#ifdef DEBUG
void
__error__(char *pcFilename, unsigned long ulLine)
{
}
#endif

#define TRUE 1
#define FALSE 0


void delay(unsigned long aValue);
void getData(int* valuePtr);

//*****************************************************************************
//
// Exchange data between two functions
//
//*****************************************************************************
int
main(void)
{
    //
    // Set the clocking to run directly from the crystal.
    //
    SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN |
                   SYSCTL_XTAL_8MHZ);

    //
    // Initialize the OLED display.
    //
    RIT128x96x4Init(1000000);
    
    //  declare a shared variable and a pointer to it
    int i = 0;
    
    char myData[9];                 //  declare a character array
    
    RIT128x96x4StringDraw("The array contains: \n", 10, 24, 15);

    while(TRUE)
    {
      
      for (i = 0; i <= 3; i++)
      {
        myData[2*i] = i + 'A';        //  convert the int i to ascii
        myData[2*i + 1] = ' ';
      }
      
      myData[2*i] = '\0';             //  terminate the string
        
           
      RIT128x96x4StringDraw(myData, 15, 44, 15);
        
      delay(1000);                //  delay so we can read the display
    }
}

void delay(unsigned long aValue)
{
    volatile unsigned long i = 0;

    volatile unsigned int j = 0;
    
    for (i = aValue; i > 0; i--)
    {
        for (j = 0; j < 100; j++);
    }

    return;
}