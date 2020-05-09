
DEPS = common.h
CC=gcc
CFLAGS=-I.
LLIBS=-lreadline -lm -lcomedi
all: mricom

objects := mricom.o common.o func.o blockstim.o analogdaq.o mrikst.o 
objects += blockstim.o testproc.o

mricom: ./obj/mricom.o ./obj/func.o ./obj/common.o
	$(CC) -o mricom ./obj/mricom.o ./obj/func.o ./obj/common.o $(LLIBS)

mribg: mribg.o common.o
	$(CC) -o mribg mribg.o common.o $(LLIBS)

blockstim: blockstim.o common.o 
	$(CC) -o blockstim blockstim.o common.o $(LLIBS)

analogdaq: analogdaq.o common.o
	$(CC) -o analogdaq analogdaq.o common.o $(LLIBS)

mrikst: mrikst.o common.o
	$(CC) -o mrikst mrikst.o common.o

testproc: testproc.o common.o
	$(CC) -o testproc testproc.o common.o


