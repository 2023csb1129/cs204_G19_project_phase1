CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++11

all: myRISCVSim

myRISCVSim: main.o myRISCVSim.o
	$(CXX) $(CXXFLAGS) -o myRISCVSim main.o myRISCVSim.o

main.o: main.cpp myRISCVSim.h
	$(CXX) $(CXXFLAGS) -c main.cpp

myRISCVSim.o: myRISCVSim.cpp myRISCVSim.h
	$(CXX) $(CXXFLAGS) -c myRISCVSim.cpp

clean:
	rm -f *.o myRISCVSim
