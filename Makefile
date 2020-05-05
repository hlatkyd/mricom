
DEPS = common.h
CC=gcc
CFLAGS=-I.
LLIBS=-lreadline -lm -lcomedi
all: mricom blockstim testproc mrikst analogdaq

mricom: mricom.o func.o common.o
	$(CC) -o mricom mricom.o func.o common.o $(LLIBS)

blockstim: blockstim.o common.o 
	$(CC) -o blockstim blockstim.o common.o $(LLIBS)

analogdaq: analogdaq.o common.o
	$(CC) -o analogdaq analogdaq.o common.o $(LLIBS)

mrikst: mrikst.o common.o
	$(CC) -o mrikst mrikst.o common.o

testproc: testproc.o common.o
	$(CC) -o testproc testproc.o common.o


all clean:
		rm -f *.o
