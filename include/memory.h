#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>

// Function declarations
void init_memory(void);
void write_instruction(short int instruction);
uint16_t get_pc(void);
void set_pc(uint16_t new_pc);
int get_no_of_instructions(void);
short int fetch_instruction(void);
// int8_t load_data(int8_t index);
int8_t load_data(uint16_t index);

// void store_data(int8_t data);
void store_data(int8_t data, uint16_t index);
int8_t read_reg(int index);
void write_reg(int index, int8_t data);
void print_nonzero_gprs(void);

// Global variables
extern short int instruction_memory[1024];
extern int counti;
extern int current_instruction;
extern int countd;
extern uint8_t data_memory[2048];
extern int8_t gpr[64];
extern uint8_t sreg;
extern uint16_t pc;

#endif
