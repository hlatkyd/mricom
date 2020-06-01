# Run make all
#
#

CC=gcc
CFLAGS=-I ./src
LIBS=-lreadline -lm -lcomedi
OBJ=./obj
SRC=./src
BIN=./bin
DAT=./data

all: dir mricom mribg mrikst ttlctrl blockstim vnmrclient analogdaq test_proc test_console test_create
tests: test_proc test_console test_create

dir: 
	mkdir -p $(OBJ)
	mkdir -p $(BIN)
	mkdir -p $(DAT)

vnmr: dir vnmrclient

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

ttlctrl: $(OBJ)/ttlctrl.o $(OBJ)/common.o $(OBJ)/socketcomm.o
	$(CC) -o $(BIN)/ttlctrl $(OBJ)/ttlctrl.o $(OBJ)/common.o $(OBJ)/socketcomm.o $(LIBS)

test_proc: $(OBJ)/test_proc.o $(OBJ)/common.o
	$(CC) -o $(BIN)/test_proc $(OBJ)/test_proc.o $(OBJ)/common.o $(LIBS)

test_console: $(OBJ)/test_console.o $(OBJ)/common.o $(OBJ)/socketcomm.o
	$(CC) -o $(BIN)/test_console $(OBJ)/test_console.o $(OBJ)/common.o $(OBJ)/socketcomm.o $(LIBS)

test_create: $(OBJ)/test_create.o $(OBJ)/common.o
	$(CC) -o $(BIN)/test_create $(OBJ)/test_create.o $(OBJ)/common.o $(LIBS)
clean:
	rm -f $(OBJ)/*
	rm -f $(BIN)/*

count:
	find . -name '*.c' | xargs wc -l
	find . -name '*.h' | xargs wc -l
