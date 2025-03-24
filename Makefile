CC = gcc
CFLAGS = -Wall -Wextra -std=c99

all: myRISCVSim

myRISCVSim: main.o myRISCVSim.o
	$(CC) $(CFLAGS) -o myRISCVSim main.o myRISCVSim.o

main.o: main.c myRISCVSim.h
	$(CC) $(CFLAGS) -c main.c

myRISCVSim.o: myRISCVSim.c myRISCVSim.h
	$(CC) $(CFLAGS) -c myRISCVSim.c

clean:
	rm -f *.o myRISCVSim
