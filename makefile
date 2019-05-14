
libpFactory.a: Parallel.o Communicators.o
	ar rcs libpFactory.a Parallel.o Communicators.o

Parallel.o: Parallel.cc Parallel.h
	g++ -std=c++14 -O3 -c Parallel.cc

Communicators.o: Communicators.cc Communicators.h
	g++ -std=c++14 -O3 -c Communicators.cc

clean:
	rm -rf *.o libpFactory.a
