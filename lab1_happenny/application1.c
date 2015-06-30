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

void clearScreen(int aDelay);
void delay(int aDelay);
void display(char character, int aDelay, int xCord, int yCord);

//*****************************************************************************
//
// Print A B C D for ~1 second
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
        
    while(TRUE)
    {
      
      // Show 'A' on screen
      display('A', 0, 34, 54);
      // Show ' ' on screen
      display(' ', 0, 40, 54); 
      // Show 'B' on screen  
      display('B', 0, 46, 54);
      // Show ' ' on screen
      display(' ', 0, 52, 54); 
      // Show 'C' on screen     
      display('C', 0, 58, 54);
      // Show ' ' on screen
      display(' ', 0, 64, 54); 
      // Show 'D' on screen 
      display('D', 0, 70, 54);
      
      // Show 'A B C D' for ~1 second
      delay(8000);
      
      // Clear the screen and keep blank for ~1 seconds
      clearScreen(8000);
      
    }
}

/**
  * aDelay: the duration to keep the screen cleared for
**/
void clearScreen(int aDelay)
{
    // Clear the OLED
    RIT128x96x4Clear();
    // Keep screen cleared for aDelay amount of time
    delay(aDelay);
    return;
}

/**
  * aDelay: the duration to keep the screen cleared for
**/
void delay(int aDelay)
{
    volatile int i = 0;

    volatile int j = 0;
    
    for (i = aDelay; i > 0; i--)
    {
        for (j = 0; j < 100; j++);
    }

    return;
}
/**
  * character: the character to display on the screen
  * aDelay: the duration to show the character on screen
  * xCord: the x-coordinate of the screen to display the character at
  * yCord: the x-coordinate of the screen to display the character at
**/
void display(char character, int aDelay, int xCord, int yCord)
{
    char tempChar[2];
    tempChar[0] = character;
    tempChar[1] = '\0';
    
    // Draw the character at the given x and y coordinates
    RIT128x96x4StringDraw(tempChar, xCord, yCord, 15);
    
    delay(aDelay);
    
    return;
}
