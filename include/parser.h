#ifndef PARSER_H
#define PARSER_H

#include <stdint.h>

void extractOperands(const char *instruction, int *R1, int *R2, int imm);
void parseInstruction(const char *instruction);
void loadProgram(const char *filename);

#endif 