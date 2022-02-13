CC := gcc
CSTND := --std=c11
CFLAGS := -o
SRC := tyvm.c
DEPS := lc3_lib.h lc3_lib.c preprocessor.c registers.c

OUT := tyvm-unix
#OUT := tyvm-win

.PHONY: all clean
all: tyvm

tyvm: $(SRC) $(DEPS)
	$(CC) $(CSTND) $(SRC) $(CFLAGS) $(OUT)