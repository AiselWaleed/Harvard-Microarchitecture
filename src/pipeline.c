#include <stdint.h>
#include <stdio.h>
#define INST_MEM_SIZE 1024
#include "../include/memory.h"
#include "../include/pipeline.h"


int clock;
int global_pc;
int current_instruction;
int no_of_instructions;
int end_of_instructions;

PipelineStage IF= {0};
PipelineStage ID= {0};
PipelineStage IE= {0};

//TEMP: to be implemented in memory
uint16_t instruction_memory [INST_MEM_SIZE];

void init_pipeline(){
    clock =0;
    global_pc = 0;
    no_of_instructions = 0;
    current_instruction = 0;
    end_of_instructions = 0;
    printf("init_pipeline: Pipelining initiated \n");
    //TEMP: sample parsed insts, later to be taken from memory
    instruction_memory [0] = (0b1111000011110000);
    instruction_memory [1] = (0b1101010011110000);    
    instruction_memory [2] = (0b1111001100110000);
    instruction_memory [3] = (0b1100111011110000);
    instruction_memory [4] = (0b1000110011110000);
    printf("init_pipeline: Instructions in place \n");
}

void increment_pc(){
    global_pc++;
}

int get_no_of_inst(uint16_t inst_mem[]){
    int size = 0;
    int i=0;
    while (i<INST_MEM_SIZE){
        if (inst_mem[i]!=0)
            size++;
        i++;
    }

    printf("get_no_of_inst: size of array = %d \n", size);
    return size;
}

void fetch_inst(){
    printf("fetch_inst: Current pc = %d \n", global_pc);

    if (global_pc >= 1023)
        return;

    //Changed later to FFFF or a similarly invalid value.
    if (instruction_memory[global_pc] == 0 || current_instruction==no_of_instructions){
        printf("fetch_inst: No more instructions to fetch");
        IF.valid =0;
        end_of_instructions = 1;
        return;
    }

    printf("fetch_inst: this is safely inside the fetch_inst method \n");

    uint16_t fetched_instruction = instruction_memory[global_pc];
    //data check?
    IF.pc = global_pc;
    IF.instruction = fetched_instruction;
    IF.valid = 1;
    IF.inst_id = ++current_instruction;
    increment_pc();
}

void decode(){
    if (!ID.valid)
        return;
    ID.opcode = (ID.instruction >> 12) & 0b1111 ;
    ID.r1 = (ID.instruction >> 6) & 0b111111;
    ID.r2 = (ID.instruction) & (0b111111);
    ID.imm = (ID.instruction) & (0b111111);
    // ID.r1 = ID.instruction & (0b111111 << 6);
    // ID.r2 = ID.instruction & (0b111111);
    // ID.imm = ID.instruction & (0b111111);
    printf("this is the decode method, decoding instruction %d \n", ID.inst_id);
    printf("decode: opcode = %d\n", ID.opcode);
    printf("decode: r1 = Register %d \n", ID.r1);
    printf("decode: r2 = Register %d \n", ID.r2);
}


void execute(){
    if (!IE.valid)
        return;

    //TEMP
    IE.val1=66;
    IE.val2=88;
    IE.result = 44;
    IE.valid=0;
    printf("this is the execute method, executing instruction %d \n", IE.inst_id);
    printf("execute: val1 = %d\n", IE.val1);
    printf("execute: val2 = %d \n", IE.val2);
    printf("execute: result = %d \n", IE.result);
}

void run_program(){
    no_of_instructions = get_no_of_inst(instruction_memory);
    if (no_of_instructions==0)
        return;

    while (!(end_of_instructions && !IE.valid && !ID.valid && !IF.valid )) {
        printf("run_program: Cycle2 %d \n", clock);

        if(IE.valid){
            execute ();
            //IE is then invalidated
            printf("run_program: instruction %d executed \n", IE.inst_id);
        }
        if(ID.valid){
            decode ();
            printf("run_program: instruction %d decoded \n", ID.inst_id);
        }
        //
        // if (!IF.valid && !end_of_instructions){
        //     fetch_inst();
        //     if (IF.valid){
        //         printf("run_program: instruction %d is fetched \n", IF.inst_id);
        //     }
        // } 
        //

        if (!IE.valid && ID.valid){
            int hazard = is_data_hazard();
            if (!hazard){
            IE = ID;
            IE.valid = 1;
            ID.valid = 0;
            printf("instruction %d is passed to execute phase \n", IE.inst_id);
            }
            else{ //HAZARD!
                printf("run_program: Stalling pipeline due to hazard in instruction %d \n", ID.inst_id);
                IE.valid = 0; //stall IE
            }
           
        }
        if (!ID.valid && IF.valid){
            ID = IF;
            ID.valid = 1;
            IF.valid = 0;
            printf("instruction %d is passed to decode phase \n", ID.inst_id);
        }
        if (!IF.valid && !end_of_instructions){
            fetch_inst();
            if (IF.valid){
                printf("run_program: instruction %d is fetched \n", IF.inst_id);
            }
        }
        
        fflush(stdout); // ensures text appears before sleeping
        SLEEP_SECOND(); 

        clock++;
    }
}


int is_data_hazard(){
    if (!ID.valid || !IE.valid) return 0;
    int ie_opcode = IE.opcode;
    int id_opcode = ID.opcode;

    if (ie_opcode==4 || ie_opcode==7||ie_opcode==11){
        //4(BEQZ), 7(BR)(flush), 11(STR) No registers are written, so they can't cause hazards
        //hazards are only caused by instructions that write to registers, so we can skip these
        return 0;
    }
    int ex_dest = IE.r1; 
    //check if ID reads R2
    int id_reads_r1=(id_opcode!=3 && id_opcode!=10); //these inst overwrite R1
    int id_reads_r2 = (id_opcode == 0 || id_opcode == 1 || id_opcode == 2 || id_opcode == 6 || id_opcode == 7 );
    
    if(id_reads_r1 && (ID.r1 == ex_dest)) {
        printf("HAZARD DETECTED: Instruction %d depends on Instruction %d (R1)\n", ID.inst_id, IE.inst_id);
        return 1;
    }
    if (id_reads_r2 && (ID.r2 == ex_dest)) {
        printf("HAZARD DETECTED: Instruction %d depends on Instruction %d (R2)\n", ID.inst_id, IE.inst_id);
        return 1;
    }
    // if (ID.r1 == ex_dest || ID.r1 == ex_dest ) {
    //         printf("HAZARD DETECTED: Instruction %d depends on Instruction %d\n", ID.inst_id, IE.inst_id);
    //         return 1;
    //     }
    
    return 0;
    
}

int main (){
    init_pipeline();
    run_program();
    return 0;
}
