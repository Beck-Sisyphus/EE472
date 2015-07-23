# Beck Pang
# Work for precompile the code easier

CC = gcc
CFLAGS=-Wall -g -std=c99 -c
lab3: Lab3Main.o Lab3Funct.o
	$(CC) Lab3Main.o Lab3Funct.o -o lab3
Lab3Main.o: lab3.h Lab3Main.c
	$(CC) $(CFLAGS) Lab3Main.c
Lab3Funct.o: lab3.h Lab3Funct.c
	$(CC) $(CFLAGS) Lab3Funct.c
clean:
	rm -f *~ *.o lab3