
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "../include/memory.h"

short int instruction_memory[1024];
int counti=0;
int current_instruction=0;
int countd=0;
uint8_t data_memory[2048];
int8_t gpr[64]={0};
uint8_t sreg;
uint16_t pc;

void init_memory(){
    for(int i=0;i<1024;i++){
        instruction_memory[i]=0xFFFF;
    }
    for(int i=0;i<2048;i++){
        data_memory[i]=0xFF;
    }
}

void write_instruction(short int instruction){
    instruction_memory[counti]=instruction;
    counti++;
}

uint16_t get_pc(void){
    return pc;
}
void set_pc(uint16_t new_pc){
    pc = new_pc;
}

int get_no_of_instructions(){
    return counti;
}
// short int fetch_instruction(){
//     if (get_pc() >= 1023)
//         return -1;
//     if (get_pc() >= get_no_of_instructions() || instruction_memory[pc] == 0xFFFF){
//         printf("fetch_instruction: No more instructions to fetch");
//         return -1;
//         //is that okay?
//     }
//     //UNIFY PC INCREMENT
//     return instruction_memory[pc++];
// }

short int fetch_instruction(){
    if (pc >= 1023)
        return -1;
    if (instruction_memory[pc] == 0xFFFF){
        printf("fetch_instruction: No more instructions to fetch");
        return -1;
        //is that okay?
    }
    //UNIFY PC INCREMENT
    return instruction_memory[pc++];
}

int8_t load_data(uint16_t index){
    return data_memory[index];
}
//void store_data (int8_t data){
//    data_memory[countd++]=data;
//}
void store_data(int8_t data, uint16_t index){
    data_memory[index]=data;
}

int8_t read_reg(int index){
    return gpr[index];
}

void write_reg(int index, int8_t data){
    gpr[index]=data;
}

void print_nonzero_gprs(void){
    int printed = 0;

    for (int i = 0; i < 64; i++) {
        if (gpr[i] != 0) {
            printf("R%d = %d\n", i, gpr[i]);
            printed = 1;
        }
    }

    if (!printed) {
        printf("All GPRs are zero.\n");
    }
}