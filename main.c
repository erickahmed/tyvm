/*
    TVM is a virtual machine for LC-3 based operating systems and it used for educational purposes.
    Open-source software under MIT License
    Written by Erick Ahmed, 2022
*/

#include <stdint.h>>

/* Initializing 128kb of memory */
uint16_t memory[UINT16_MAX];

/* Initializing 10 registers of which:
8 general purpose, 1 program counter, 1 conditional */
enum {
    RG_GP0 = 0,
    RG_GP1,
    RG_GP2,
    RG_GP3,
    RG_GP4,
    RG_GP5,
    RG_GP6,
    RG_GP7,
    RG_PC = 0,       // program counter
    RG_COND,         // condition flag
    RG_COUNT
};

uint16_t reg[RG_COUNT];

/* Creating instruction set opcodes */
enum {
    OP_BR = 0,      // branch
    OP_ADD,         // add
    OP_LD,          // load
    OP_ST,          // store
    OP_JSR,         // jump register
    OP_AND,         // bitwise add
    OP_LDR,         // load register
    OP_STR,         // store register
    OP_RTI,         // unused opcode
    OP_NOT,         // bitwise not
    OP_LDI,         // indirect load
    OP_STI,         // indirect store
    OP_JMP,         // jump
    OP_RES,         // reserved opcode
    OP_LEA,         // load effective address
    OP_TRAP,        // execute trap
};

/* Creating condition flags */
enum {
    FL_POS = 1 << 0,    // P
    FL_ZRO = 1 << 1,    // Z
    FL_NEG = 1 << 2,    // N
}