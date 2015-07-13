//  Declare the prototypes for the tasks

#ifndef HEADERSTUFF
#define HEADERSTUFF

	void get (void* aNumber);				//  input task
	void increment (void* aNumber);			//  computation task
	void display (void* aNumber);			//  output task

	//  Declare a TCB structure

	typedef struct 
	{
		void* taskDataPtr;
		void (*taskPtr)(void*);
	}
	TCB;

#endif