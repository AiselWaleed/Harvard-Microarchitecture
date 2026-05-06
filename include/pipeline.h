#ifndef pipeline.h
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
    uint16_t instruction;
    int opcode;
    int r1, r2;
    int8_t imm;
    int val1, val2; //in case needed later in ALU
    int result;
    int pc;
    int valid;
} PipelineStage;

extern PipelineStage IF= {0};
extern PipelineStage ID= {0};
extern PipelineStage IE= {0};

#endif