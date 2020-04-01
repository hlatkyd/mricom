LCOMEDI:=-lcomedi
CC=gcc
CFLAGS=-I.
LLIBS=-lreadline
mricom: mricom.o func.o parser.o
	$(CC) -o mricom mricom.o func.o parser.o $(LLIBS) $(LCOMEDI)
