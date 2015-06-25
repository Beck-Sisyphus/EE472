//*****************************************************************************
//
// Built from hello.c - Simple hello world example.
//
// Copyright (c) 2006-2011 Texas Instruments Incorporated.  All rights reserved.
// Software License Agreement
// 
// Texas Instruments (TI) is supplying this software for use solely and
// exclusively on TI's microcontroller products. The software is owned by
// TI and/or its suppliers, and is protected under applicable copyright
// laws. You may not combine this software with "viral" open-source
// software in order to form a larger program.
// 
// THIS SOFTWARE IS PROVIDED "AS IS" AND WITH ALL FAULTS.
// NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT
// NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. TI SHALL NOT, UNDER ANY
// CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
// DAMAGES, FOR ANY REASON WHATSOEVER.
// 
// This is part of revision 7243 of the EK-LM3S8962 Firmware Package.
//
//*****************************************************************************

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
    int myValue = 0;
    int* myPtr = &myValue;      //  let myPtr point to myValue
    
    char myData[2];		//  declare a character array
    

    while(TRUE)
    {
      getData(myPtr);
      
      myData[0] = *myPtr+'0';
      
      myData[1] = '\0';           //  terminate the string
        
      RIT128x96x4StringDraw("Data returned: \n", 15, 44, 15);              
      RIT128x96x4StringDraw(myData, 15, 54, 15);
        
      delay(1000);                //  delay so we can read the display
    }
}

void getData(int* valuePtr)
{
    //  declare a temp place to store the data
    int tempValue;
    static int i = 0;
    
    char myData[2];

    //  let valuePtr point to it
    valuePtr = &tempValue;
 
    //  get the data
    *valuePtr = i;
     
     myData[0] = i + '0';        //  convert the int i to ascii
     myData[1] = '\0';           //  terminate the string

    //  display its value as a character 
     
    RIT128x96x4StringDraw("getData data is: \n", 15, 24, 15);     
    
    RIT128x96x4StringDraw(myData, 15, 34, 15);
        
    delay(1000);                //  delay so we can read the display
    
    i = (i+1) % 8;

    return;

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