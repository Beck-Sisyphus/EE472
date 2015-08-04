#include <stdio.h>
#include <stdint.h>
#include "lab4.h"

// Implement a new pseudo random number generator
uint16_t randomInteger(const unsigned short* globalCount){
	uint16_t retVal = 0;
	// It's always at 2
	switch (*globalCount) {
		case 0: retVal = 0x0FF1; break;
		case 1:	retVal = 0x0FE2; break;
		case 2: retVal = 0x0FD3; break;
		case 3: retVal = 0x0FC4; break;
		case 4: retVal = 0x0FB5; break;
		default: retVal = 0x0FF1;break;
	}
	return retVal;
}