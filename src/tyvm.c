/*
    TYVM is a virtual machine based on LC-3 architecture for educational purposes.
    Copyright (c) 2022 Erick Ahmed
    Open-source software distributed under MIT license
*/

#include "includes.h"
#include "registers.h"

#ifndef __UNIX
    HANDLE hStdin = INVALID_HANDLE_VALUE;
#endif

#define TRUE 1
#define FALSE 0


/* Sign extension function for immediate add mode (imm5[0:4])
transforms 5bit number to 8bit number preserving sign*/
uint16_t sign_extend(uint16_t n, int bit_count) {
    if((n >> (bit_count - 1)) & 1) {
        n |= (0xFFFF << bit_count);
    }
    return n;
}

/* function to swap to big endian */
int swap16(uint16_t x) {
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

int mem_read(uint16_t address) {
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

void handle_interrupt(int signal) {
    restore_input_buffering();
    printf("\n");
    exit(-2);
}


int main(int argc, const char* argv[]) {
    if(argc < 2) {
        printf("usage: [image-file1] ...\n");
        exit(2);
    }

    for(int i = 1; i < argc; i++) {
        printf("failed to load image: %s\n", argv[i]);
        exit(1);
    }

    signal(SIGINT, handle_interrupt);     //FIXME: handle_interrupt may not be correct, check lc3 docs
    disable_input_buffering();

    reg[RG_COND] = FL_Z;

    enum {PC_START = 0x3000};
    reg[RG_PC] = PC_START;          //0x3000 is default load address

    int running = TRUE;
    while(running) {
        uint16_t instr = mem_read(reg[RG_PC]++);
        uint16_t op    = instr >> 12;

        uint16_t cond;
        uint16_t PCoffset9;     // 9-bit value that indicates where to load the address when added to RG_PC
        uint16_t PCoffset11;
        uint16_t dr;            // destination register
        uint16_t sr;            // source register
        uint16_t sr1;           // source register 1
        uint16_t sr2;           // source register 2
        uint16_t imm_flag;      // immediate mode flag (bit[5])
        uint16_t imm5;
        uint16_t jsr_flag;
        uint16_t BaseR_jsr;
        uint16_t BaseR_jsrr;
        uint16_t BaseR;
        uint16_t offset6;

        uint16_t* stringPnt;
        uint16_t* c;
        uint16_t* ch;

        switch (op) {
            case OP_BR:
                cond      = (instr >> 9) & 0x7;
                PCoffset9 = sign_extend(instr & 0x1FF, 9);

                if(cond & reg[RG_COND]) reg[RG_PC] += PCoffset9;

                break;
            case OP_ADD:
                dr       = (instr >> 9) & 0x7;
                sr1      = (instr >> 6) & 0x7;
                imm_flag = (instr >> 5) & 0x1;

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
                dr        = (instr >> 9) & 0x7;
                PCoffset9 = sign_extend(instr & 0x1FF, 9);

                reg[dr] = mem_read(PCoffset9 + reg[RG_PC]);

                update_flags(dr);

                break;
            case OP_ST:
                sr        = (instr >> 6) & 0x7;
                PCoffset9 = sign_extend(instr & 0x1FF, 9);     // 9-bit value that indicates where to load the address when added to RG_PC

                reg[sr] = mem_read(PCoffset9 + reg[RG_PC]);

                update_flags(dr);

                break;
            case OP_JSR:
                PCoffset11  = sign_extend(instr & 0x1FF, 11);
                jsr_flag    = (instr >> 11) & 0x1;
                BaseR_jsrr  = (instr >> 6) & 0x7;          // JSRR only ecoding

                if(jsr_flag == 0) reg[RG_PC] = BaseR_jsrr;          // JSRR
                else reg[RG_PC] += PCoffset11;                      // JSR

                break;
            case OP_AND:
                dr       = (instr >> 9) & 0x7;
                sr1      = (instr >> 6) & 0x7;
                imm_flag = (instr >> 5) & 0x1;

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
                dr      = (instr >> 9) & 0x7;
                BaseR   = (instr >> 6) & 0x7;
                offset6 = sign_extend(instr & 0x3FF, 6);

                reg[dr] = mem_read(reg[BaseR] + offset6);

                update_flags(dr);

                break;
            case OP_STR:
                sr      = (instr >> 6) & 0x7;
                BaseR   = (instr >> 6) & 0x7;
                offset6 = sign_extend(instr & 0x1FF, 6);

                reg[sr] = mem_read(offset6 + BaseR);        //TODO: check if reg[] is needed

                update_flags(dr);

                break;
            case OP_NOT:
                dr = (instr >> 9) & 0x7;   // destination register
                sr = (instr >> 6) & 0x7;   // source register

                reg[dr] = ~(reg[sr]);

                update_flags(dr);

                break;
            case OP_LDI:
                dr        = (instr >> 9) & 0x7;
                PCoffset9 = sign_extend(instr & 0x1FF, 9);

                reg[dr] = mem_read(mem_read(PCoffset9 + reg[RG_PC]));

                update_flags(dr);

                break;
            case OP_STI:
                sr        = (instr >> 6) & 0x7;
                PCoffset9 = sign_extend(instr & 0x1FF, 9);     // 9-bit value that indicates where to load the address when added to RG_PC

                reg[sr] = mem_read(mem_read(PCoffset9 + reg[RG_PC]));

                update_flags(dr);

                break;
            case OP_JMP:
                BaseR = (instr >> 6) & 0x7;

                reg[RG_PC] = reg[BaseR];

                update_flags(dr);

                break;
            case OP_LEA:
                dr = (instr >> 9) & 0x7;
                PCoffset9 = sign_extend(instr & 0x1FF, 9);

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
                        stringPnt = memory + reg[RG_000];

                        while (*c) {
                            putc((char)*c, stdout);
                            ++c;
                        }
                        fflush(stdout);

                        break;
                    case TRAP_IN:
                        printf("Enter a character: ");
                        char c = getchar();
                        putc(c, stdout);
                        fflush(stdout);

                        reg[RG_000] = (uint16_t)c;
                        update_flags(RG_000);

                        break;
                    case TRAP_PUTSP:
                        ch = memory + reg[RG_000];
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
