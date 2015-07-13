//  Building a simple OS kernel - step 4

#include <stdio.h>
#include "kernel3.h"


void main(void)
{
	int i=0;							//  queue index
	int data;							//  declare a shared data
	int* aPtr = &data;					//  point to it

	TCB* queue[3];						//  declare queue as an array of pointers to TCBs

	//  	Declare some TCBs
	TCB inTask;
	TCB compTask;
	TCB outTask;
	TCB* aTCBPtr;

	//  	Initialize the TCBs
	inTask.taskDataPtr = (void*)&data;
	inTask.taskPtr = get;

	compTask.taskDataPtr = (void*)&data;
	compTask.taskPtr = increment;

	outTask.taskDataPtr = (void*)&data;
	outTask.taskPtr = display;

   // 	Initialize the task queue
	queue[0] = &inTask;
	queue[1] = &compTask;
	queue[2] = &outTask;

	// schedule and dispatch the tasks
	while(1)
	{
		aTCBPtr = queue[i];
		aTCBPtr->taskPtr( (aTCBPtr->taskDataPtr) );
		i = (i+1)%3;
	}

	return;

}


