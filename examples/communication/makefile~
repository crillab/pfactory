
LFLAGS    = -L../../ -lpFactory -lpthread

helloworld: Helloworld.o
	g++ -o helloworld Helloworld.o $(LFLAGS)

Helloworld.o: Helloworld.cc
	g++ -std=c++14 -O3 -c Helloworld.cc 

clean:
	rm -rf helloworld *.o
