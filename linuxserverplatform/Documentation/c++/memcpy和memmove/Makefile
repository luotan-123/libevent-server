CPP = g++
CFLAGS = -g -Wall -O -std=c++11
 
main:main.o
	$(CPP) main.o -o main
main.o:main.cpp
	$(CPP) $(CFLAGS) -c main.cpp 
clean:
	rm -f main.o main
