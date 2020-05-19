all: mricom mribg mrikst blockstim vnmrclient analogdaq testproc 

vnmr: vnmrclient

CC=gcc
CFLAGS=-I ./src
LIBS=-lreadline -lm -lcomedi
OBJ=./obj
SRC=./src
BIN=./bin


$(OBJ)/%.o: $(SRC)/%.c 
	$(CC) -c $(CFLAGS) $< -o $@

mricom: $(OBJ)/mricom.o $(OBJ)/common.o $(OBJ)/func.o $(OBJ)/socketcomm.o
	$(CC) -o mricom $(OBJ)/mricom.o $(OBJ)/common.o $(OBJ)/func.o $(OBJ)/socketcomm.o $(LIBS)

mribg: $(OBJ)/mribg.o $(OBJ)/common.o $(OBJ)/socketcomm.o
	$(CC) -o mribg $(OBJ)/mribg.o $(OBJ)/common.o $(OBJ)/socketcomm.o $(LIBS)

mrikst: $(OBJ)/mrikst.o $(OBJ)/common.o
	$(CC) -o $(BIN)/mrikst $(OBJ)/mrikst.o $(OBJ)/common.o $(LIBS)

blockstim: $(OBJ)/blockstim.o $(OBJ)/common.o
	$(CC) -o $(BIN)/blockstim $(OBJ)/blockstim.o $(OBJ)/common.o $(LIBS)

analogdaq: $(OBJ)/analogdaq.o $(OBJ)/common.o
	$(CC) -o $(BIN)/analogdaq $(OBJ)/analogdaq.o $(OBJ)/common.o $(LIBS)

vnmrclient: $(OBJ)/vnmrclient.o $(OBJ)/common.o $(OBJ)/socketcomm.o
	$(CC) -o $(BIN)/vnmrclient $(OBJ)/vnmrclient.o $(OBJ)/common.o $(OBJ)/socketcomm.o $(LIBS)

testproc: $(OBJ)/testproc.o $(OBJ)/common.o
	$(CC) -o $(BIN)/testproc $(OBJ)/testproc.o $(OBJ)/common.o $(LIBS)

clean:
	rm -f $(OBJ)/*
	rm -f $(BIN)/*

count:
	find . -name '*.c' | xargs wc -l
