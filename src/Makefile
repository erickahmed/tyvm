.PHONY: all
all: tyvm-unix tyvm-win

tyvm-unix: tyvm.c includes.h registers.h
	gcc --std=c11 tyvm.c -o tyvm-unix

lc3sim-win: main.c lc3os.c vm.h vm.c
	gcc --std=c11 tyvm.c -o tyvm-win