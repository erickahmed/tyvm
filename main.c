/*
    TVM is a LC-3 based virtual machine used for educational purposes
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