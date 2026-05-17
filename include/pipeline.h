#ifndef PIPELINE_H
#define PIPELINE_H
#define INST_MEM_SIZE 1024
#include <stdint.h>
#include <stdio.h>

//clock
#ifdef _WIN32
#include <Windows.h>
#define SLEEP_SECOND() Sleep(1000) // Windows uses milliseconds

#endif

typedef struct {
    int inst_id;
    short int instruction;
    int opcode;
    int r1, r2;
    int8_t imm;
    int8_t val1, val2; //in case needed later in ALU
    int8_t result;
    uint16_t pc;
    int valid;
    int branch_taken; // Flag to indicate if a branch was taken
} PipelineStage;
extern PipelineStage IF;
extern PipelineStage ID;
extern PipelineStage IE;
// Function declarations
int is_data_hazard(void);
void run_program(void);

#endif