#####################################################
#   Makefile for compiling sorces 
#   with CPLEX optimizer
#####################################################

# include 	./src/Makefile.var

CC = g++
GCC = gcc

SRC = src/sudoku.cpp
## CPLEX headers, e.g.
HDRS = /opt/ibm/ILOG/CPLEX_Studio1262/cplex/include
## For C++:
HDRSXX = /opt/ibm/ILOG/CPLEX_Studio1262/concert/include

## CPLEX Headers:
INCLUDE = -I$(HDRS)
INCLUDE += -I$(HDRSXX)

## General flags:
CFLAGS = -fopenmp -g #-pg -v
CFLAGS += -Wall -Wextra -Woverflow -W #-Weffc++ #-Winline
## Preprocessing:
CFLAGS += -DIL_STD 
## Optimization:
CFLAGS += -O2 #--help=optimizers
## Loop alignment (default n=1, n=2 better)
CFLAGS += -falign-loops=2 -funroll-loops

## CPLEX libs, e.g.
LIB_PATH = -L/opt/ibm/ILOG/CPLEX_Studio1262/cplex/lib/x86-64_linux/static_pic 
LIB_PATH += -L/opt/ibm/ILOG/CPLEX_Studio1262/concert/lib/x86-64_linux/static_pic 
LIBS = -lilocplex -lconcert -lcplex
LIBS += -lm -lpthread

TARGET = sudoku

main: $(SRC)

	$(CC) $(CFLAGS) $(INCLUDE) $(SRC) $(LIBS) $(LIB_PATH) -o $(TARGET)

clean:

	rm -f $(TARGET) 
