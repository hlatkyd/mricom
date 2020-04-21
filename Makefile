
DEPS = common.h
CC=gcc
CFLAGS=-I.
LLIBS=-lreadline -lm -lcomedi
all: mricom blockstim

mricom: common.o mricom.o func.o parser.o comedifunc.o
	$(CC) -o common.o mricom mricom.o func.o parser.o comedifunc.o $(LLIBS)

#blockstim: blockstim.o mricom.o parser.o 
#	$(CC) -o blockstim blockstim.o mricom.o parser.o $(LLIBS)

all clean:
		rm -f *.o
