CC = g++
CCFLAGS= -Wextra -Wall -Wvla -std=c++11 -pthread -g -DNDEBUG
TARGETS = libuthreads

all: $(TARGETS)

# Library Compilation
libuthreads: uthreads.h uthreads.o Thread.o Thread.h
	ar rcs libuthreads.a uthreads.o Thread.o

	
# Object Files	
Thread.o: Thread.cpp Thread.h	
	$(CC) $(CCFLAGS) -c Thread.cpp

uthreads.o: uthreads.cpp uthreads.h Thread.h Thread.cpp
	$(CC) $(CCFLAGS) -c uthreads.cpp
	
#tar
tar:
	tar -cf ex2.tar uthreads.cpp Thread.cpp Thread.h Makefile README
	
.PHONY: clean

clean:
	-rm -f *.o libosm
