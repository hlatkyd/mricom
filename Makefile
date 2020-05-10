all: mricom mribg mrikst blockstim analogdaq testproc 

CC=gcc
CFLAGS=-I ./src
LIBS=-lreadline -lm -lcomedi
OBJ=./obj
SRC=./src
BIN=./bin


$(OBJ)/%.o: $(SRC)/%.c 
	$(CC) -c $(CFLAGS) $< -o $@

mricom: $(OBJ)/mricom.o $(OBJ)/common.o $(OBJ)/func.o
	$(CC) -o mricom $(OBJ)/mricom.o $(OBJ)/common.o $(OBJ)/func.o $(LIBS)

mribg: $(OBJ)/mribg.o $(OBJ)/common.o
	$(CC) -o $(BIN)/mribg $(OBJ)/mribg.o $(OBJ)/common.o $(LIBS)

mrikst: $(OBJ)/mrikst.o $(OBJ)/common.o
	$(CC) -o $(BIN)/mrikst $(OBJ)/mrikst.o $(OBJ)/common.o $(LIBS)

blockstim: $(OBJ)/blockstim.o $(OBJ)/common.o
	$(CC) -o $(BIN)/blockstim $(OBJ)/blockstim.o $(OBJ)/common.o $(LIBS)

analogdaq: $(OBJ)/analogdaq.o $(OBJ)/common.o
	$(CC) -o $(BIN)/analogdaq $(OBJ)/analogdaq.o $(OBJ)/common.o $(LIBS)

vnmrjclient: $(OBJ)/vnmrjclient.o $(OBJ)/common.o
	$(CC) -o $(BIN)/vnmrjclient $(OBJ)/vnmrjclient.o $(OBJ)/common.o $(LIBS)

testproc: $(OBJ)/testproc.o $(OBJ)/common.o
	$(CC) -o $(BIN)/testproc $(OBJ)/testproc.o $(OBJ)/common.o $(LIBS)

clean:
	rm -f $(OBJ)/*
