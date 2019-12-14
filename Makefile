CC=gcc
CFLAGS=-I.
mricom: mricom.o func.o
	$(CC) -o mricom mricom.o func.o
