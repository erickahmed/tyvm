/*
    TYVM is a virtual machine based on LC-3 architecture for educational purposes.
    Copyright (c) 2022 Erick Ahmed
    Open-source software distributed under GNU GPL v.3 license
*/

#include "preprocessor.c"
#include "registers.c"
#include "lc3_lib.h"

int main(int argc, const char* argv[]) {
    if(argc != 2) {
        printf("loading image: %s\n", argv[2]);
        exit(2);
    }

    if(!read_image(argv[1])) {
        printf("failed to load image: %s\n", argv[2]);
        exit(1);
    }

    signal(SIGINT, handle_interrupt);
    disable_input_buffering();

    reg[RG_COND] = FL_Z;

    enum {PC_START = 0x3000};
    reg[RG_PC] = PC_START;          //0x3000 is default load address

    int running = TRUE;
    while(running) {
        const uint16_t instr = mem_read(reg[RG_PC]++);
        const uint16_t op    = instr >> 12;

        static uint16_t cond;   // condition flag status
        static uint16_t PCoffset9;     // 9-bit value that indicates where to load the address when added to PC register
        static uint16_t PCoffset11;    // 11-bit value that indicates where to load the address when added to PC register
        static uint16_t dr;            // destination register
        static uint16_t sr;            // source register
        static uint16_t sr1;           // source register 1
        static uint16_t sr2;           // source register 2
        static uint16_t imm_flag;      // immediate mode flag (bit[5])
        static uint16_t imm5;          // immediate mode 5 bit value
        static uint16_t jsr_flag;      // JSR flag
        static uint16_t BaseR_jsr;
        static uint16_t BaseR_jsrr;
        static uint16_t BaseR;
        static uint16_t offset6;       // 6-bit offset value

        static uint16_t* stringPnt;
        static uint16_t* c;
        static uint16_t* ch;

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
                    case TC_GETC:
                        reg[RG_R0] = (uint16_t)getchar();
                        break;
                    case TC_OUT:
                        putc((char)reg[RG_R0], stdout);
                        fflush(stdout);
                        break;
                    case TC_PUTS:
                        stringPnt = memory + reg[RG_R0];

                        while (*c) {
                            putc((char)*c, stdout);
                            ++c;
                        }
                        fflush(stdout);

                        break;
                    case TC_IN:
                        printf("Enter a character: ");
                        char c = getchar();
                        putc(c, stdout);
                        fflush(stdout);

                        reg[RG_R0] = (uint16_t)c;
                        update_flags(RG_R0);

                        break;
                    case TC_PUTSP:
                        ch = memory + reg[RG_R0];
                        while (*ch) {
                            char char1 = (*ch) & 0xFF;
                            putc(char1, stdout);
                            char char2 = (*ch) >> 8;
                            if (char2) putc(char2, stdout);
                            ++ch;
                        }
                        fflush(stdout);

                        break;
                    case TC_HALT:
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