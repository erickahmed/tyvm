# TyVM - [T]in[y] [V]irtual [M]achine
TyVM is a 16bit VM based on the LC-3 architechture that runs Assembly code, and could more generally run even an operating system

## Introduction
LC-3 is a type of computer educational assembly language.
It features a relatively simple instruction set, but can be used to write moderately complex assembly programs, and is a viable target for a C compiler. The language is less complex than x86 assembly but has many features similar to those in more complex languages. These features make it useful for beginning instruction, so it is most often used to teach fundamentals of programming and computer architecture to computer science and computer engineering students. 

The LC-3 specifies a word size of 16 bits for its registers and uses a 16-bit addressable memory with a 216-location address space. The register file contains eight registers, referred to by number as R0 through R7. All of the registers are general-purpose in that they may be freely used by any of the instructions that can write to the register file, but in some contexts (such as translating from C code to LC-3 assembly) some of the registers are used for special purposes.

Instructions are 16 bits wide and have 4-bit opcodes. The instruction set defines instructions for fifteen of the sixteen possible opcodes, though some instructions have more than one mode of operation. Individual instructions' execution is regulated by a state machine implemented with a control ROM and microsequencing unit. 

### Hardware
TYVM is a very barebone VM that has:
  - 128kb memory
  - 16 opcodes
  - 10 registers
  - 3 condition flags

#### Memory
It has 2^16(=65,536) individual memory locations
```
uint16_t memory[UINT16_MAX];
```
#### Registers
TYVM has n.10 16-bit registers.
    - 8 general purpose registers
    - 1 program counter (PC)
    - 1 conditional register (COND)

```c
enum {
    RG_000 = 0,
    RG_001,
    RG_010,
    RG_011,
    RG_100,
    RG_101,
    RG_110,
    RG_111,
    RG_PC,
    RG_COND,
    RG_COUNT
};
uint16_t registers[R_COUNT];
```
#### Instruction set
TYVM has n.16 16-bit opcodes, that instructs the machine to do some simple calculation:
```c
enum {           // [name, 8-bit value]
    OP_BR = 0,      // branch, 0000
    OP_ADD,         // add, 0001
    OP_LD,          // load, 0010
    OP_ST,          // store, 0011
    OP_JSR,         // jump register, 0100
    OP_AND,         // bitwise and, 0101
    OP_LDR,         // load register, 0110
    OP_STR,         // store register, 0111
    OP_RTI,         // unused opcode
    OP_NOT,         // bitwise not, 1001
    OP_LDI,         // indirect load, 1010
    OP_STI,         // indirect store, 1011
    OP_JMP,         // jump, 1100
    OP_RES,         // reserved opcode,
    OP_LEA,         // load effective address, 1110
    OP_TRAP,        // execute trap, 1111
};
```

### Logic
10  Load instruction from memory at program counter [RG_PC] address;
20  Increment RG_PC;
30  Read opcode for next instruction;
40  Perform instruction read in 40;
50  Goto 10;

### Build
On file "preprocessor.c" define the OS where you want to run TYVM:

  #define __UNIX

Uncomment this line if you want to build for Unix based OS, otherwise comment it out to build for Windows
Build using the command:
``` 
gcc tyvm.c -o tyvm
```
Or simply using makefile (optional: in makefile change binary file name wether building on Unix or Windows):

``` 
make
```

### Usage
```bash
./tyvm <*.asm>
```

Below is a `hello-word` program for `TYVM`, the assembled program can be found in `asm` directory
```shell
.ORIG x3000
LEA R0, HENLO_WORLD
PUTS
HALT
HELLO_STR .STRINGZ "Henlo World!"
.END
```
