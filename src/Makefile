CC := gcc
CSTND := --std=c11
CFLAGS := -o
SRC := tyvm.c

#OUT = tyvm-unix
OUT := tyvm-win

.PHONY: all clean
all: tyvm

tyvm: tyvm.c includes.h registers.h
	$(CC) $(CSTND) $(SRC) $(CFLAGS) $(OUT)