/* initialization of registers and memory

/* memory mapped register tables */
enum mmr {
    MMR_KSR = 0xFE00,   // keyboard status
    MMR_KDR = 0xFE02    // keyboard data
};

/* Initializing 10 registers of which:
8 general purpose, 1 program counter, 1 conditional */
enum registers {
    R0 = 0,
    R1,
    R2,
    R3,
    R4,
    R5,
    R6,
    R7,
    RG_PC,           // program counter
    RG_COND,         // condition flag
    RG_COUNT
};

/* Creating instruction set opcodes */
enum opcodes {      // [name, 8-bit value]
    BR = 0,      // branch, 0000
    ADD,         // add, 0001
    LD,          // load, 0010
    ST,          // store, 0011
    JSR,         // jump register, 0100
    AND,         // bitwise and, 0101
    LDR,         // load register, 0110
    STR,         // store register, 0111
    RTI,         // unused opcode
    NOT,         // bitwise not, 1001
    LDI,         // indirect load, 1010
    STI,         // indirect store, 1011
    JMP,         // jump, 1100
    RES,         // reserved opcode,
    LEA,         // load effective address, 1110
    TRAP,        // execute trap, 1111
};

/* Trap codes used for OP_TRAP */
enum trapcodes {
    GETC  = 0x20,  // get charcter from keyboard
    OUTP   = 0x21,  // output a character
    PUTS  = 0x22,  // output a word string
    INP    = 0x23,  // get charcter from keyboard and echo to terminal
    PUTSP = 0x24,  // output a byte string
    HALT  = 0x25   // halt program
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