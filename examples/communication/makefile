
LFLAGS    = -L../../ -lpFactory -lpthread

helloworld: Communication.o
	g++ -o communication Communication.o $(LFLAGS)

Helloworld.o: Helloworld.cc
	g++ -std=c++14 -O3 -c Communication.cc 

clean:
	rm -rf communication *.o
