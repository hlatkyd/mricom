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
TST=./test

all: dir mricom mribg mrikst ttlctrl blockstim vnmrclient analogdaq \
	managedio \
	test_proc test_console test_create test_usergo test_run

test: test_proc test_console test_create test_usergo test_run

dir: 
	mkdir -p $(OBJ)
	mkdir -p $(BIN)
	mkdir -p $(DAT)

vnmr: dir vnmrclient

$(OBJ)/%.o: $(SRC)/%.c 
	$(CC) -c $(CFLAGS) $< -o $@

mricom: $(OBJ)/mricom.o $(OBJ)/common.o $(OBJ)/func.o $(OBJ)/socketcomm.o $(OBJ)/help.o
	$(CC) -o mricom $(OBJ)/mricom.o $(OBJ)/common.o $(OBJ)/func.o $(OBJ)/socketcomm.o $(OBJ)/help.o $(LIBS)

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

managedio: $(OBJ)/managedio.o
	$(CC) -o $(BIN)/managedio $(OBJ)/managedio.o $(LIBS)

# test subprograms
#-----------------------------------------------

test_proc: $(OBJ)/test_proc.o $(OBJ)/common.o
	$(CC) -o $(TST)/test_proc $(OBJ)/test_proc.o $(OBJ)/common.o $(LIBS)

test_console: $(OBJ)/test_console.o $(OBJ)/common.o $(OBJ)/socketcomm.o
	$(CC) -o $(TST)/test_console $(OBJ)/test_console.o $(OBJ)/common.o $(OBJ)/socketcomm.o $(LIBS)

test_create: $(OBJ)/test_create.o $(OBJ)/common.o
	$(CC) -o $(TST)/test_create $(OBJ)/test_create.o $(OBJ)/common.o $(LIBS)

test_usergo: $(OBJ)/test_usergo.o $(OBJ)/common.o
	$(CC) -o $(TST)/test_usergo $(OBJ)/test_usergo.o $(OBJ)/common.o

test_run: $(OBJ)/test_run.o $(OBJ)/common.o
	$(CC) -o $(TST)/test_run $(OBJ)/test_run.o $(OBJ)/common.o $(LIBS)

# util
# ------------------------------------------------

clean:
	rm -f $(OBJ)/*
	rm -f $(BIN)/*

count:
	find . -name '*.c' | xargs wc -l
	find . -name '*.h' | xargs wc -l
