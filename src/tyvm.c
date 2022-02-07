/*
    TYVM is a virtual machine based on LC-3 architecture for educational purposes.
    Copyright (c) 2022 Erick Ahmed
    Open-source software distributed under GNU GPL v.3 license
*/

#include "preprocessor.h"
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
            case BR:
                cond      = (instr >> 9) & 0x7;
                PCoffset9 = sign_extend(instr & 0x1FF, 9);

                if(cond & reg[RG_COND]) reg[RG_PC] += PCoffset9;

                break;
            case ADD:
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
            case LD:
                dr        = (instr >> 9) & 0x7;
                PCoffset9 = sign_extend(instr & 0x1FF, 9);

                reg[dr] = mem_read(PCoffset9 + reg[RG_PC]);

                update_flags(dr);

                break;
            case ST:
                sr        = (instr >> 6) & 0x7;
                PCoffset9 = sign_extend(instr & 0x1FF, 9);     // 9-bit value that indicates where to load the address when added to RG_PC

                reg[sr] = mem_read(PCoffset9 + reg[RG_PC]);

                update_flags(dr);

                break;
            case JSR:
                PCoffset11  = sign_extend(instr & 0x1FF, 11);
                jsr_flag    = (instr >> 11) & 0x1;
                BaseR_jsrr  = (instr >> 6) & 0x7;          // JSRR only ecoding

                if(jsr_flag == 0) reg[RG_PC] = BaseR_jsrr;          // JSRR
                else reg[RG_PC] += PCoffset11;                      // JSR

                break;
            case AND:
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
            case LDR:
                dr      = (instr >> 9) & 0x7;
                BaseR   = (instr >> 6) & 0x7;
                offset6 = sign_extend(instr & 0x3FF, 6);

                reg[dr] = mem_read(reg[BaseR] + offset6);

                update_flags(dr);

                break;
            case STR:
                sr      = (instr >> 6) & 0x7;
                BaseR   = (instr >> 6) & 0x7;
                offset6 = sign_extend(instr & 0x1FF, 6);

                reg[sr] = mem_read(offset6 + BaseR);        //TODO: check if reg[] is needed

                update_flags(dr);

                break;
            case NOT:
                dr = (instr >> 9) & 0x7;   // destination register
                sr = (instr >> 6) & 0x7;   // source register

                reg[dr] = ~(reg[sr]);

                update_flags(dr);

                break;
            case LDI:
                dr        = (instr >> 9) & 0x7;
                PCoffset9 = sign_extend(instr & 0x1FF, 9);

                reg[dr] = mem_read(mem_read(PCoffset9 + reg[RG_PC]));

                update_flags(dr);

                break;
            case STI:
                sr        = (instr >> 6) & 0x7;
                PCoffset9 = sign_extend(instr & 0x1FF, 9);     // 9-bit value that indicates where to load the address when added to RG_PC

                reg[sr] = mem_read(mem_read(PCoffset9 + reg[RG_PC]));

                update_flags(dr);

                break;
            case JMP:
                BaseR = (instr >> 6) & 0x7;

                reg[RG_PC] = reg[BaseR];

                update_flags(dr);

                break;
            case LEA:
                dr = (instr >> 9) & 0x7;
                PCoffset9 = sign_extend(instr & 0x1FF, 9);

                reg[dr] = reg[RG_PC] + PCoffset9;

                update_flags(dr);

                break;
            case TRAP:
                switch(instr & 0xFF) {
                    case GETC:
                        reg[R0] = (uint16_t)getchar();
                        break;
                    case OUTP:
                        putc((char)reg[R0], stdout);
                        fflush(stdout);
                        break;
                    case PUTS:
                        stringPnt = memory + reg[R0];

                        while (*c) {
                            putc((char)*c, stdout);
                            ++c;
                        }
                        fflush(stdout);

                        break;
                    case INP:
                        printf("Enter a character: ");
                        char c = getchar();
                        putc(c, stdout);
                        fflush(stdout);

                        reg[R0] = (uint16_t)c;
                        update_flags(R0);

                        break;
                    case PUTSP:
                        ch = memory + reg[R0];
                        while (*ch) {
                            char char1 = (*ch) & 0xFF;
                            putc(char1, stdout);
                            char char2 = (*ch) >> 8;
                            if (char2) putc(char2, stdout);
                            ++ch;
                        }
                        fflush(stdout);

                        break;
                    case HALT:
                        puts("HALT");
                        fflush(stdout);
                        running = FALSE;

                        break;
                    default:
                        abort();
                        break;
                }
                break;
            case RES:    // reserved
            case RTI:    // unused
            default:
                abort();
                break;
        }
    }
    restore_input_buffering();  //restore terminal settings when shutdown
}