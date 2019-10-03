
libpFactory.a: pFactory.o Communicators.o
	ar rcs libpFactory.a pFactory.o Communicators.o

pFactory.o: pFactory.cc pFactory.h
	g++ -std=c++14 -O3 -c pFactory.cc

Communicators.o: Communicators.cc Communicators.h
	g++ -std=c++14 -O3 -c Communicators.cc

clean:
	rm -rf *.o libpFactory.a
