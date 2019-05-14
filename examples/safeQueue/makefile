
LFLAGS    = -L../../ -lpFactory -lpthread

safeQueue: SafeQueue.o
	g++ -o safeQueue SafeQueue.o $(LFLAGS)

SafeQueue.o: SafeQueue.cc
	g++ -std=c++14 -O3 -c SafeQueue.cc 

clean:
	rm -rf safeQueue *.o
