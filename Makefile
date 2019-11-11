CC=gcc
CFLAGS=-I.
mricom: mricom.o test.o
	$(CC) -o mricom mricom.o test.o
