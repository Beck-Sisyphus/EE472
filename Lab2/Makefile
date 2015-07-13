# Beck Pang
# Work for precompile the code easier

CC = gcc
CFLAGS=-Wall -g -std=c11 -c
lab2: Lab2Prelim.o Lab2Funct.o
	$(CC) Lab2Prelim.o Lab2Funct.o -o lab2
Lab2Prelim.o: lab2.h Lab2Prelim.c
	$(CC) $(CFLAGS) Lab2Prelim.c
Lab2Funct.o: lab2.h Lab2Funct.c
	$(CC) $(CFLAGS) Lab2Funct.c
clean:
	rm -f *.o