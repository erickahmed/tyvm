/*
    TVM is a virtual machine for LC-3 based operating systems and it is used for educational purposes.
    Open-source software under MIT License
    Written by Erick Ahmed, 2022
*/

#define IS_UNIX

/* universal libraries */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>

/* unix only libraries */
#ifdef IS_UNIX
    #include <unistd.h>
    #include <fcntl.h>
#endif

/* sys libraries */
#include <sys/time.h>
#include <sys/types.h>
#include <sys/termios.h>
#include <sys/mman.h>

#define TRUE 1
#define FALSE 0

#define BR
#define ADD
#define LD
#define ST
#define JSR
#define AND
#define LDR
#define STR
#define RTI
#define NOT
#define LDI
#define STI
#define JMP
#define RES
#define LEA
#define TRAP
#define BAD_OPCODE

/* Initializing 10 registers of which:
8 general purpose, 1 program counter, 1 conditional */
enum registers {
    RG_GP0 = 0,
    RG_GP1,
    RG_GP2,
    RG_GP3,
    RG_GP4,
    RG_GP5,
    RG_GP6,
    RG_GP7,
    RG_PC,           // program counter
    RG_COND,         // condition flag
    RG_COUNT
};

/* Initializing memory and register storages */
uint16_t memory[UINT16_MAX];
uint16_t reg[RG_COUNT];

/* Creating instruction set opcodes */
enum opcodes {
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
enum flags {
    FL_POS = 1,         // P
    FL_ZRO = 1 << 1,    // Z
    FL_NEG = 1 << 2,    // N
};


int main(int argc, const char* argv[]) {

    reg[RG_COND]  = FL_ZRO;
    reg[RG_PC]    = 0x3000;     //0x3000 is default load address

    int running = TRUE;
    while(running) {
        uint16_t instr = mem_read(reg[RG_PC]++);
        uint16_t op = instr >> 12;

        switch (op) {
            case OP_BR:
                {BR};
                break;
            case OP_ADD:
                {ADD};
                break;
            case OP_LD:
                {LD};
                break;
            case OP_ST:
                {ST}
                break;
            case OP_JSR:
                {JSR};
                break;
            case OP_AND:
                {AND};
                break;
            case OP_LDR:
                {LDR};
                break;
            case OP_STR:
                {STR};
                break;
            case OP_NOT:
                {NOT};
                break;
            case OP_LDI:
                {LDI};
                break;
            case OP_STI:
                {STI};
                break;
            case OP_JMP:
                {JMP};
                break;
            case OP_LEA:
                {LEA};
                break;
            case OP_TRAP:
                {TRAP};
                break;
            case OP_RES:
            case OP_RTI:
            default:
                {BAD_OPCODE}
                break;



        };
    }
}
