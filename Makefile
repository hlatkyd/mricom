CC=gcc
CFLAGS=-I.
LLIBS=-lreadline -lcomedi
mricom: mricom.o func.o
	$(CC) -o mricom mricom.o func.o $(LLIBS)
