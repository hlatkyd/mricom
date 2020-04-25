
DEPS = common.h
CC=gcc
CFLAGS=-I.
LLIBS=-lreadline -lm -lcomedi
all: mricom blockstim

mricom: mricom.o func.o common.o comedifunc.o 
	$(CC) -o mricom mricom.o func.o common.o comedifunc.o $(LLIBS)

blockstim: blockstim.o common.o 
	$(CC) -o blockstim blockstim.o common.o $(LLIBS)

all clean:
		rm -f *.o
