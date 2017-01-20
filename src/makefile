# Makefile for the C++ build 

CC=g++

IDIR =../include
CFLAGS=-I$(IDIR)

ODIR=obj
LDIR =../lib
LIB=-lncurses -lpthread


#      $(patsubst pattern,replacement,text)
_DEPS = error.h irobot.h packets.h psensor.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = main.o irobot.o psensor.o 
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


$(ODIR)/%.o: %.cpp $(DEPS)
	$(CC) -c -std=c++11 -o $@ $< $(CFLAGS)

iRobot: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIB)

.PHONY: clean
clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ 
