#ifndef ALU_H
#define ALU_H

#include <stdint.h>

// Function declarations
int8_t Alu(int8_t operandA, int8_t operandB, int opcode, int8_t imm);
void printFlags();
int updateCarry(int flag, int8_t reg1, int8_t reg2, int opcode);
int updateOverFlow(int flag, int8_t reg1, int8_t reg2, int opcode, int8_t result);
int updateNegative(int flag, int8_t out);   
#endif
