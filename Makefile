LCOMEDI:=-lcomedi
CC=gcc
CFLAGS=-I.
LLIBS=-lreadline -lm
mricom: mricom.o func.o parser.o comedifunc.o
	$(CC) -o mricom mricom.o func.o parser.o comedifunc.o $(LLIBS) $(LCOMEDI)
