#include "preprocessor.c"

/* initialization of registers and memory

/* memory mapped register tables */
enum mmr {
    MR_KSR = 0xFE00,   // keyboard status
    MR_KDR = 0xFE02    // keyboard data
};

/* Initializing 10 registers of which:
8 general purpose, 1 program counter, 1 conditional */
enum registers {
    RG_R0 = 0,
    RG_R1,
    RG_R2,
    RG_R3,
    RG_R4,
    RG_R5,
    RG_R6,
    RG_R7,
    RG_PC,           // program counter
    RG_COND,         // condition flag
    RG_COUNT
};

/* Creating instruction set opcodes */
enum opcodes {      // [name, 8-bit value]
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

/* Trap codes used for OP_TRAP */
enum trapcodes {
    TC_GETC  = 0x20,  // get charcter from keyboard
    TC_OUT   = 0x21,  // output a character
    TC_PUTS  = 0x22,  // output a word string
    TC_IN    = 0x23,  // get charcter from keyboard and echo to terminal
    TC_PUTSP = 0x24,  // output a byte string
    TC_HALT  = 0x25   // halt program
};

/* Creating condition flags */
enum flags {
    FL_P = 1,         // Positive
    FL_Z = 1 << 1,    // Zero
    FL_N = 1 << 2,    // Negative
};

/* Initializing memory and register storages */
uint16_t memory[UINT16_MAX];
uint16_t reg[RG_COUNT];