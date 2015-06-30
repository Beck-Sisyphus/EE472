#include "drivers/rit128x96x4.h"
void delay(unsigned long aValue);

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