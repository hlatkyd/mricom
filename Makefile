USE_COMEDI=1
ifeq (USE_COMEDI,1)
LCOMEDI:=-lcomedi
else
LCOMEDI:=
endif
CC=gcc
CFLAGS=-I.
LLIBS=-lreadline
mricom: mricom.o func.o
	$(CC) -o mricom mricom.o func.o $(LLIBS) $(LCOMEDI)
