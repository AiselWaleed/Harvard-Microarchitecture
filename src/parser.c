#include "../includes/parser.h"
#include "../includes/memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

static uint16_t parsedInstruction = 0;
static int R1 = 0;
static int R2 = 0;
static int imm = 0;

void extractOperands(const char *instruction, int *r1, int *r2, int i)
{
    char buffer[256];
    strncpy(buffer, instruction, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';

    strtok(buffer, " "); // skip mnemonic
    char *r1str = strtok(NULL, " ");
    char *r2str = strtok(NULL, " \n");

    *r1 = atoi(r1str + 1);                   // skip 'R'
    *r2 = i ? atoi(r2str) : atoi(r2str + 1); // if imm is non-zero, treat r2 as an immediate value, otherwise as a register
    imm = i;
}

// parser
void parseInstruction(const char *instruction)
{
    if (instruction == NULL)
    {
        fprintf(stderr, "Error: Null instruction string\n");
        exit(EXIT_FAILURE);
    }

    parsedInstruction = 0; // reset parsed instruction before parsing a new one

    // ADD - opcode 0
    if (strncmp(instruction, "ADD", 3) == 0)
    {
        extractOperands(instruction, &R1, &R2, 0);
        parsedInstruction |= (0 & 0xF) << 12;
    }
    // SUB - opcode 1
    else if (strncmp(instruction, "SUB", 3) == 0)
    {
        extractOperands(instruction, &R1, &R2, 0);
        parsedInstruction |= (0b0001 << 12);
    }
    // MUL - opcode 2
    else if (strncmp(instruction, "MUL", 3) == 0)
    {
        extractOperands(instruction, &R1, &R2, 0);
        parsedInstruction |= (0b0010 << 12);
    }
    // MOVI - opcode 3
    else if (strncmp(instruction, "MOVI", 4) == 0)
    {
        extractOperands(instruction, &R1, &R2, 1);
        parsedInstruction |= (0b0011 << 12);
    }
    // BEQZ - opcode 4
    else if (strncmp(instruction, "BEQZ", 4) == 0)
    {
        extractOperands(instruction, &R1, &R2, 1);
        parsedInstruction |= (0b0100 << 12);
    }
    // ANDI - opcode 5
    else if (strncmp(instruction, "ANDI", 4) == 0)
    {
        extractOperands(instruction, &R1, &R2, 1);
        parsedInstruction |= (0b0101 << 12);
    }
    // EOR - opcode 6
    else if (strncmp(instruction, "EOR", 3) == 0)
    {
        extractOperands(instruction, &R1, &R2, 0);
        parsedInstruction |= (0b0110 << 12);
    }
    // BR - opcode 7
    else if (strncmp(instruction, "BR", 2) == 0)
    {
        extractOperands(instruction, &R1, &R2, 0);
        parsedInstruction |= (0b0111 << 12);
    }
    // handle if imm shift value results in overflow if greater than 6 bits (i.e. greater than 63)
    // SLC - opcode 8
    else if (strncmp(instruction, "SLC", 3) == 0)
    {
        extractOperands(instruction, &R1, &R2, 1);
        if (R2 < 0)
        {
            fprintf(stderr, "Error: Shift value for SLC cannot be negative\n");
            exit(EXIT_FAILURE);
        }
        parsedInstruction |= (0b1000 << 12);
    }
    // SRC - opcode 9
    else if (strncmp(instruction, "SRC", 3) == 0)
    {
        extractOperands(instruction, &R1, &R2, 1);
        if (R2 < 0)
        {
            fprintf(stderr, "Error: Shift value for SRC cannot be negative\n");
            exit(EXIT_FAILURE);
        }
        parsedInstruction |= (0b1001 << 12);
    }
    // LDR - opcode 10
    else if (strncmp(instruction, "LDR", 3) == 0)
    {
        extractOperands(instruction, &R1, &R2, 1);
        parsedInstruction |= (0b1010 << 12);
    }
    // STR - opcode 11
    else if (strncmp(instruction, "STR", 3) == 0)
    {
        extractOperands(instruction, &R1, &R2, 1);
        parsedInstruction |= (0b1011 << 12);
    }
    else
    {
        fprintf(stderr, "Error: Unrecognized instruction '%s'\n", instruction);
        exit(EXIT_FAILURE);
    }

    // error handling for value bounds
    if (R1 < 0 || R1 > 63)
    {
        fprintf(stderr, "Error: Register R%d out of bounds (must be between 0 and 63)\n", R1);
        exit(EXIT_FAILURE);
    }
    if (imm == 0 && (R2 < 0 || R2 > 63))
    {
        fprintf(stderr, "Error: Register R%d out of bounds (must be between 0 and 63)\n", R2);
        exit(EXIT_FAILURE);
    }
    if (imm == 1 && (R2 > 31 || R2 < -32))
    {
        fprintf(stderr, "Error: Immediate value %d out of bounds (must be between -32 and 31)\n", R2);
        exit(EXIT_FAILURE);
    }

    parsedInstruction |= (R1 & 0x3F) << 6;
    parsedInstruction |= (R2 & 0x3F);

    // write to instruction memory
    writeInstruction(parsedInstruction);
}

// read program from file and parse each instruction
void loadProgram(const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        fprintf(stderr, "Error: Could not open file '%s'\n", filename);
        exit(EXIT_FAILURE);
    }

    char line[256];
    while (fgets(line, sizeof(line), file))
    {
        // skip empty lines and comments
        if (line[0] == '\n' || line[0] == '#')
        {
            continue;
        }
        parseInstruction(line);
    }

    fclose(file);
}