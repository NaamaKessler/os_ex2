CC = g++
CCFLAGS= -Wextra -Wall -Wvla -std=c++11 -pthread -g -DNDEBUG
TARGETS = libuthreads

all: $(TARGETS)

# Library Compilation
libuthreads: uthreads.h uthreads.cpp Thread.o 
	ar rcs libuthreads.a uthreads

	
# Object Files	
osm.o: osm.cpp osm.h	
	$(CC) $(CCFLAGS) -c osm.cpp

	
#tar
tar:
	tar -cf ex1.tar osm.cpp Makefile README
	
.PHONY: clean

clean:
	-rm -f *.o libosm
