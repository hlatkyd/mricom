CC=gcc
CFLAGS=-I.
LLIBS=-lreadline
mricom: mricom.o func.o
	$(CC) -o mricom mricom.o func.o $(LLIBS)
