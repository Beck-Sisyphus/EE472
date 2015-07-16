# Beck Pang
# Work for precompile the code easier

CC = gcc
CFLAGS=-Wall -g -std=c11 -c
lab2: Lab2Prelim.o Lab2Funct.o random.o
	$(CC) Lab2Prelim.o Lab2Funct.o random.o -o lab2
Lab2Prelim.o: lab2.h Lab2Prelim.c
	$(CC) $(CFLAGS) Lab2Prelim.c
Lab2Funct.o: lab2.h Lab2Funct.c
	$(CC) $(CFLAGS) Lab2Funct.c
random.o: lab2.h randomInteger.c
	$(CC) $(CFLAGS) randomInteger.c
clean:
	rm -f *~ *.o lab2