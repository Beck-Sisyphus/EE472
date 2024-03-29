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

#define TRUE 1  // Defines the variable TRUE to be a 1, high bit
#define FALSE 0 // Defines the variable FALSE to be a 0, low bit


void delay(unsigned long aValue);       // Adds a software delay of aValue in length
void f1Data(unsigned long *delayValue);
void f2Clear(unsigned long *delayValue);

volatile int i = 0;
int k = 15;
//  working C style string
char myData[3];   // Create a 3 character array

//*****************************************************************************
//
// Print "0123456789" to the OLED on the Stellaris evaluation board.
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
    
    //  define some local variables
    // user settable delay value between characters output on OLED
    unsigned long delayValue = 1000;
    unsigned long *delayPtr = &delayValue;
    
    //
    // The value if i is:
    //
    RIT128x96x4StringDraw("The value of i is: \n", 10, 24, 15); // Output the value of i to OLED
    
    //
    //  print the digits 9 8 7 6 5 4 3 2 1 0
    while(TRUE)
    {      
      
      f1Data(delayPtr);
      
      k = 15;
      
      myData[0] = ' ';              //  print a space
      myData[1] = '\0';
      
      //  clear the line
      f2Clear(delayPtr);
      
      k = 15;
    }

}

//  software delay
void delay(unsigned long aValue)
{
    volatile unsigned long i = 0;

    volatile unsigned int j = 0;
    
    for (i = aValue; i > 0; i--)
    {
        for (j = 0; j < 100; j++);      // Adds more delay to every i from aValue to 0
    }

    return;
}

void combined(char c, unsigned long delayValue)
{
  
}

// output the data to the OLED screen
// with the given delayValue between characters
void f1Data(unsigned long *delayValue)
{
  
    for (i = 9; i >=0; i--)
    {
    myData[0] = i + '0';        //  convert the int i to ascii
    myData[1] = '\0';           //  terminate the string

    k = k + 10;                //  start at oled position 15
                             //  move 10 units to right for next
                              //  character
     
    RIT128x96x4StringDraw(myData, k, 44, 15);       // Output myData on OLED
                                                  // at k position

    delay(*delayValue);                //  delay so we can read the display
    }
}

// clear the OLED one character at a time
// with the given delayValue between characters
void f2Clear(unsigned long *delayValue)
{

    for (i = 0; i < 10; i++)
    {    
      k = k + 10;     // Start at screen position 15, add 10 units to right for next char
         
      RIT128x96x4StringDraw(myData, k, 44, 15);
      
      delay(*delayValue);
    }
}
