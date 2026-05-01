#ifndef PARSER_H
#define PARSER_H

void extractOperands(const char *instruction, int *R1, int *R2, int imm);
void parseInstruction(uint16_t instruction);
void loadProgram(const char *filename);

#endif 
