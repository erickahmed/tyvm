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
    R_GP0,
    R_GP1,
    R_GP2,
    R_GP3,
    R_GP4,
    R_GP5,
    R_GP6,
    R_GP7,
    R_PC = 0,
    R_COND,
    R_COUNT
};

uint16_t reg[R_COUNT];

/* Creating LC-3 instruction set opcodes */
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
