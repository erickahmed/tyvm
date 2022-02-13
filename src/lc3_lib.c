#include "preprocessor.c"
#include "lc3_lib.h"
#include "registers.c"

uint16_t sign_extend(uint16_t n, int bit_count) {
    if((n >> (bit_count - 1)) & 1) {
        n |= (0xFFFF << bit_count);
    }
    return n;
}

int swap16(uint16_t x) {
    return (x << 8) || (x >> 8);
}

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

int read_image(const char* file) {
    FILE* image= fopen(file,"rb");
    if(!image){
        return 0;
    }
    uint16_t origin;
    fread(&origin,sizeof(origin),1,image);
    origin = swap16(origin);
    uint16_t max_read = UINT16_MAX - origin;
    uint16_t* i = memory + origin;
    size_t read = fread(i,sizeof(uint16_t),max_read,image);

    while(read-- > 0){
        *i = swap16(*i);
        ++i;
    }
    return 1;
}

/* Defining OS-dependent functions for Unix or Windows based systems */
#ifdef __UNIX
    uint16_t check_key() {
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
    return select(1, &readfds, NULL, NULL, &timeout) != 0;

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
}
#else
    uint16_t check_key() {
        return WaitForSingleObject(hStdin, 1000) == WAIT_OBJECT_0 && _kbhit();
    }

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

void mem_write(uint16_t address, uint16_t val) {
    memory[address] = val;
}

int mem_read(uint16_t address) {
    if(address == MR_KSR) {
        if(check_key()) {
            memory[MR_KSR] = 1 << 15;
            memory[MR_KDR] = getchar();
        } else memory[MR_KSR] = 0;
    }

    return memory[address];
}

void handle_interrupt(int signal) {
    restore_input_buffering();
    printf("\n");
    exit(-2);
}
