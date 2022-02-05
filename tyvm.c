/*
    TVM is a virtual machine for LC-3 based operating systems and it is used for educational purposes.
    Copyright (c) under MIT license
    Written by Erick Ahmed, 2022
*/

//#define __UNIX        // uncomment ONLY if compiling on unix-based operative system

/* universal libraries */
    #include <stdint.h>
    #include <stdio.h>
    #include <signal.h>

/* unix only libraries */
#ifdef __UNIX
    #include <stdlib.h>
    #include <string.h>
    #include <unistd.h>
    #include <fcntl.h>

    /* sys libraries */
    #include <sys/time.h>
    #include <sys/types.h>
    #include <sys/termios.h>
    #include <sys/mman.h>
/* windows only libraries */
#else
    #include <Windows.h>
    #include <conio.h>
#endif

#ifndef __UNIX
    /* windows only declaration */
    HANDLE hStdin = INVALID_HANDLE_VALUE;
#endif

#define TRUE 1
#define FALSE 0


/* memory mapped register tables */
enum mmr {
    MMR_KSR = 0xFE00,   // keyboard status
    MMR_KDR = 0xFE02    // keyboard data
};

/* Initializing 10 registers of which:
8 general purpose, 1 program counter, 1 conditional */
enum registers {
    RG_000 = 0,
    RG_001,
    RG_010,
    RG_011,
    RG_100,
    RG_101,
    RG_110,
    RG_111,
    RG_PC,           // program counter
    RG_COND,         // condition flag
    RG_COUNT
};

/* Trap codes used for OP_TRAP */
enum trapcodes {
    TRAP_GETC  = 0x20,  // get charcter from keyboard
    TRAP_OUT   = 0x21,  // output a character
    TRAP_PUTS  = 0x22,  // output a word string
    TRAP_IN    = 0x23,  // get charcter from keyboard and echo to terminal
    TRAP_PUTSP = 0x24,  // output a byte string
    TRAP_HALT  = 0x25   // halt program
};

/* Initializing memory and register storages */
uint16_t memory[UINT16_MAX];
uint16_t reg[RG_COUNT];

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

/* Creating condition flags */
enum flags {
    FL_P = 1,         // Positive
    FL_Z = 1 << 1,    // Zero
    FL_N = 1 << 2,    // Negative
};

/* Sign extension function for immediate add mode (imm5[0:4])
transforms 5bit number to 8bit number preserving sign*/
uint16_t sign_extend(uint16_t n, int bit_count) {
    if((n >> (bit_count - 1)) & 1) {
        n |= (0xFFFF << bit_count);
    }
    return n;
}

/* function to swap to big endian */
void swap16(uint16_t x) {
    return (x << 8) || (x >> 8);
}

/* Flag update function
Every time a value is written to a register the flag will be updated */
void update_flags(uint16_t r) {
    if(reg[r] == 0) reg[RG_COND] = FL_Z;
    else if(reg[r] >> 15) reg[RG_COND] = FL_N;
    else reg[RG_COND] = FL_P;
}

/* function to load assembly programs*/
void read_image_file(FILE* file) {
    /* image placement memory location */
    uint16_t origin;
    fread(&origin, sizeof(origin), 1, file);
    origin = swap16(origin);

    uint16_t max_read = UINT16_MAX - origin;
    uint16_t* p = memory + origin;
    size_t read = fread(p, sizeof(uint16_t), max_read, file);

    /* swapping to little endian */
    while (read-- > 0) {
        *p = swap16(*p);
        ++p;
    }
}

int read_image(const char* image_path) {
    FILE* file = fopen(image_path, "rb");

    if(!file) return 0;

    read_image_file(file);
    fclose(file);

    return 1;
}

#ifdef __UNIX
    uint16_t check_key() {
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
    return select(1, &readfds, NULL, NULL, &timeout) != 0;
}
#else
    uint16_t check_key() {
        return WaitForSingleObject(hStdin, 1000) == WAIT_OBJECT_0 && _kbhit();
    }
#endif

void mem_write(uint16_t address, uint16_t val) {
    memory[address] = val;
}

void mem_read(uint16_t address) {
    if(address == MMR_KSR) {
        if(check_key()) {
            memory[MMR_KSR] = 1 << 15;
            memory[MMR_KDR] = getchar();
        } else memory[MMR_KSR] = 0;
    }

    return memory[address];
}

#ifdef __UNIX
    struct termios original_tio;

    void disable_input_buffering() {
        tcgetattr(STDIN_FILENO, &original_tio);
        struct termios new_tio = original_tio;
        new_tio.c_lflag &= ~ICANON & ~ECHO;
        tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);
    }

    void restore_input_buffering() {
        tcsetattr(STDIN_FILENO, TCSANOW, &original_tio);
    }
#else
    DWORD fdwMode, fdwOldMode;

    void disable_input_buffering() {
        hStdin = GetStdHandle(STD_INPUT_HANDLE);
        GetConsoleMode(hStdin, &fdwOldMode);    // save old mode
        fdwMode = fdwOldMode
                ^ ENABLE_ECHO_INPUT             // no input echo
                ^ ENABLE_LINE_INPUT;            // return when one or more characters are available
        SetConsoleMode(hStdin, fdwMode);        // set new mode
        FlushConsoleInputBuffer(hStdin);        // clear buffer
    }

    void restore_input_buffering() {
        SetConsoleMode(hStdin, fdwOldMode);
    }
#endif

#ifdef __UNIX
    void handle_interrupt(int signal) {
        restore_input_buffering();
        printf("\n");
        exit(-2);
    }
#endif



int main(int argc, const char* argv[]) {
    if(argc < 2) {
        printf("usage: [image-file1] ...\n");
        exit(2);
    }

    for(int i = 1; i < argc; i++) {
        printf("failed to load image: %s\n", argv[i]);
        exit(1);
    }

    signal(SIGINT, handle_interrupt());     //FIXME: handle_interrupt may not be correct, gives an error without semicolons (should be without)
    disable_input_buffering();

    reg[RG_COND] = FL_Z;

    enum {PC_START = 0x3000};
    reg[RG_PC] = PC_START;          //0x3000 is default load address

    int running = TRUE;
    while(running) {
        uint16_t instr = mem_read(reg[RG_PC]++);
        uint16_t op    = instr >> 12;

        switch (op) {
            case OP_BR:
                uint16_t cond      = (instr >> 9) & 0x7;
                uint16_t PCoffset9 = sign_extend(instr & 0x1FF, 9);

                if(cond & reg[RG_COND]) reg[RG_PC] += PCoffset9;

                break;
            case OP_ADD:
                uint16_t dr       = (instr >> 9) & 0x7;  // destination register
                uint16_t sr1      = (instr >> 6) & 0x7;  // source register 1
                uint16_t sr2;                            // source register 2
                uint16_t imm_flag = (instr >> 5) & 0x1;  // immediate mode flag (bit[5])
                uint16_t imm5;                           // immediate mode register

                if(imm_flag == 0) {
                    sr2 = (instr & 0x7);
                    reg[dr] = reg[sr1] + sr2;            // register mode add
                } else {
                    imm5 = sign_extend(instr & 0x1F, 5);
                    reg[dr] = reg[sr1] + imm5;           // immediate mode add
                }

                update_flags(dr);

                break;
            case OP_LD:
                uint16_t dr        = (instr >> 9) & 0x7;
                uint16_t PCoffset9 = sign_extend(instr & 0x1FF, 9);     // 9-bit value that indicates where to load the address when added to RG_PC

                reg[dr] = mem_read(PCoffset9 + reg[RG_PC]);

                update_flags(dr);

                break;
            case OP_ST:
                uint16_t sr        = (instr >> 6) & 0x7;
                uint16_t PCoffset9 = sign_extend(instr & 0x1FF, 9);     // 9-bit value that indicates where to load the address when added to RG_PC

                reg[sr] = mem_read(PCoffset9 + reg[RG_PC]);

                update_flags(dr);

                break;
            case OP_JSR:
                uint16_t PCoffset11  = sign_extend(instr & 0x1FF, 11);
                uint16_t jsr_flag    = (instr >> 11) & 0x1;
                uint16_t BaseR_jsrr  = (instr >> 6) & 0x7;          // JSRR only ecoding

                if(jsr_flag == 0) reg[RG_PC] = BaseR_jsrr;          // JSRR
                else reg[RG_PC] += PCoffset11;                      // JSR

                break;
            case OP_AND:
                uint16_t dr       = (instr >> 9) & 0x7;
                uint16_t sr1      = (instr >> 6) & 0x7;
                uint16_t sr2;
                uint16_t imm_flag = (instr >> 5) & 0x1;
                uint16_t imm5;

                if(imm_flag == 0) {
                    sr2 = (instr & 0x7);
                    reg[dr] = reg[sr1] & sr2;            // register mode and
                } else {
                    imm5 = sign_extend(instr & 0x1F, 5);
                    reg[dr] = reg[sr1] & imm5;           // immediate mode and
                }

                update_flags(dr);

                break;
            case OP_LDR:
                uint16_t dr      = (instr >> 9) & 0x7;
                uint16_t BaseR   = (instr >> 6) & 0x7;
                uint16_t offset6 = sign_extend(instr & 0x3FF, 6);

                reg[dr] = mem_read(reg[BaseR] + offset6);

                update_flags(dr);

                break;
            case OP_STR:
                uint16_t sr      = (instr >> 6) & 0x7;
                uint16_t BaseR   = (instr >> 6) & 0x7;
                uint16_t offset6 = sign_extend(instr & 0x1FF, 6);

                reg[sr] = mem_read(offset6 + BaseR);        //TODO: check if reg[] is needed

                update_flags(dr);

                break;
            case OP_NOT:
                uint16_t dr = (instr >> 9) & 0x7;   // destination register
                uint16_t sr = (instr >> 6) & 0x7;   // source register

                reg[dr] = ~(reg[sr]);

                update_flags(dr);

                break;
            case OP_LDI:
                uint16_t dr        = (instr >> 9) & 0x7;
                uint16_t PCoffset9 = sign_extend(instr & 0x1FF, 9);

                reg[dr] = mem_read(mem_read(PCoffset9 + reg[RG_PC]));

                update_flags(dr);

                break;
            case OP_STI:
                uint16_t sr        = (instr >> 6) & 0x7;
                uint16_t PCoffset9 = sign_extend(instr & 0x1FF, 9);     // 9-bit value that indicates where to load the address when added to RG_PC

                reg[sr] = mem_read(mem_read(PCoffset9 + reg[RG_PC]));

                update_flags(dr);

                break;
            case OP_JMP:
                uint16_t BaseR = (instr >> 6) & 0x7;

                reg[RG_PC] = reg[BaseR];

                update_flags(dr);

                break;
            case OP_LEA:
                uint16_t dr = (instr >> 9) & 0x7;
                uint16_t PCoffset9 = sign_extend(instr & 0x1FF, 9);

                reg[dr] = reg[RG_PC] + PCoffset9;

                update_flags(dr);

                break;
            case OP_TRAP:
                switch(instr & 0xFF) {
                    case TRAP_GETC:
                        reg[RG_000] = (uint16_t)getchar();
                        break;
                    case TRAP_OUT:
                        putc((char)reg[RG_000], stdout);
                        fflush(stdout);
                        break;
                    case TRAP_PUTS:
                        uint16_t* stringPnt = memory + reg[RG_000];
                        uint16_t* c;

                        while (*c) {
                            putc((char)*c, stdout);
                            ++c;
                        }
                        fflush(stdout);

                        break;
                    case TRAP_IN:
                        char c = getchar();
                        putc(c, stdout);
                        fflush(stdout);

                        reg[RG_000] = (uint16_t)c;
                        update_flags(RG_000);

                        break;
                    case TRAP_PUTSP:
                        uint16_t* ch = memory + reg[RG_000];
                        while (*ch) {
                            char char1 = (*ch) & 0xFF;
                            putc(char1, stdout);
                            char char2 = (*ch) >> 8;
                            if (char2) putc(char2, stdout);
                            ++ch;
                        }
                        fflush(stdout);

                        break;
                    case TRAP_HALT:
                        puts("HALT");
                        fflush(stdout);
                        running = FALSE;

                        break;
                    default:
                        abort();
                        break;
                }

                break;
            case OP_RES:    // reserved
            case OP_RTI:    // unused
            default:
                abort();
                break;
        }
    }
    restore_input_buffering();  //restore terminal settings when shutdown
}
