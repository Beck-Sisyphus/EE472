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
void display(unsigned long *delay, char *startChar);

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
    unsigned long delayTime = 1000;
    char startChar  = 'i';

    //
    // The value if i is:
    //
    RIT128x96x4StringDraw("The value of i is: \n", 10, 24, 15);
    
    //
    //  print the digits 9 8 7 6 5 4 3 2 1 0
    while(TRUE)
    {
      display(&delayTime, &startChar);
    }

}
//  display a decrementing character stream
void display(unsigned long *delayTime, char *startChar) {
    //  working C style string
    volatile int i = 0;
    int k = 15;
    char myData[3];
    for (i = 9; i >=0; i--)
    {
        myData[0] = i + *startChar; //  convert the int i to ascii
        myData[1] = '\0';           //  terminate the string
        
        k = k + 10;                //  start at oled position 15
                                   //  move 10 units to right for next
                                   //  character
           
        RIT128x96x4StringDraw(myData, k, 44, 15);
        
        delay(*delayTime);                //  delay so we can read the display
    }
      
    //  clear the line
    k = 15;
    
    myData[0] = ' ';              //  print a space
    myData[1] = '\0';
    for (i = 0; i < 10; i++)
    {    
        k = k + 10;
           
        RIT128x96x4StringDraw(myData, k, 44, 15);
        
        delay(*delayTime);
    }

}

//  software delay
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
