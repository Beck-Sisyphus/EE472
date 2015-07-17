#include <stdio.h>
#include <stdint.h>
#include <limits.h>
#include "lab2.h"
// int seed = 12;

// Import from the function from Prof. Peckol
uint16_t randomInteger(const unsigned short* globalCount){
	uint16_t retVal = 0;
	// It's always at 2
	switch (*globalCount) {
		case 0: retVal = 0x11F1; break;
		case 1:	retVal = 0x12E2; break;
		case 2: retVal = 0x13D3; break;
		case 3: retVal = 0x14C4; break;
		case 4: retVal = 0x15B5; break;
		default: break;
	}
	return retVal;
}