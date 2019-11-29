CC=gcc
CFLAGS=-I.
mricom: mricom.o test.o util.o
	$(CC) -o mricom mricom.o test.o util.o
