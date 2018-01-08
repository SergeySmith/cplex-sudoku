#!/bin/bash

FILE="Makefile"
BIN="sudoku"
DAT="test.dat"

make -f $FILE clean && make -f $FILE && ./$BIN $DAT

echo "[*] Done                                         [ ok ]"
