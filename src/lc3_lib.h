/* Library of functions used in tyvm.c */


/* Sign extension function for immediate add mode (imm5[0:4])
transforms 5bit number to 8bit number preserving sign*/
uint16_t sign_extend(uint16_t n, int bit_count);

/* Swap to big endian */
int swap16(uint16_t x);

/* Flag update function
Every time a value is written to a register the flag will be updated */
void update_flags(uint16_t r);

/* Load assembly file*/
void read_image_file(FILE* file);

/* Read loaded assembly file */
int read_image(const char* file);

/* Input buffering */
void disable_input_buffering();
void restore_input_buffering();

/* check key - unix or win */
uint16_t check_key();

/* Write to memory address */
void mem_write(uint16_t address, uint16_t val);

/* Read memory address */
int mem_read(uint16_t address);

/* Handle interrupt */
void handle_interrupt(int signal);
