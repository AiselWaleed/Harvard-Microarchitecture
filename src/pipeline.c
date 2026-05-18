#include <stdint.h>
#include <stdio.h>
#define INST_MEM_SIZE 1024
#include "../include/memory.h"
#include "../include/pipeline.h"
#include "../include/alu.h"
#include "../include/parser.h"
int clock;
// int global_pc;
int no_of_instructions;
int end_of_instructions;
int countf;
PipelineStage IF= {0};
PipelineStage ID= {0};
PipelineStage IE= {0};

void init_pipeline(){
    clock =1;
    no_of_instructions = 0;
    current_instruction = 0;
    end_of_instructions = 0;
    countf=-1;
    printf("Pipelining initiated \n");
    printf("Instructions in place \n");
}

void print_binary16(uint16_t num) {
    for (int i = 15; i >= 0; i--) {
        int bit = (num >> i) & 1;
        printf("%d", bit);
        if (i % 4 == 0)
            printf(" ");
    }
    printf("\n");
}

void fetch_inst() {
    printf("\033[1mFETCH\033[0m\n");
    printf("Fetching \033[1mInstruction %d\033[0m\n", get_pc()+1);
    uint16_t current_pc = get_pc();
    printf("\033[1mCurrent PC =\033[0m %d\n", current_pc);

    // Stop fetching if PC exceeds the number of instructions loaded during parsing
    if (current_pc >= no_of_instructions) {
        printf("No more instructions to fetch!\n");
        countf++;
        IF.valid = 0; 
        if (countf==1){
            end_of_instructions=1;
        }
        return;
    }

    short int fetched_instruction = read_instruction_memory(current_pc);
    
    if (fetched_instruction == -1) {
        IF.valid = 0;
        return;
    }
    IF.instruction = fetched_instruction;
    IF.pc = current_pc;
    IF.valid = 1;
    IF.inst_id = current_pc+1;
    print_binary16(fetched_instruction);

    set_pc(current_pc + 1); 
}

void decode(){
    if (!ID.valid)
        return;

    printf("\033[1mDECODE\033[0m\n");
    printf("Decoding \033[1mInstruction %d\033[0m \n", ID.inst_id);

    int current_opcode = (ID.instruction >> 12) & 0b1111 ;
    ID.opcode = current_opcode;
    printf("Opcode = %d\n", ID.opcode);
    switch(current_opcode){
        case 0:
        case 1:
        case 2:
        case 6:
        case 7:
            ID.r1 = (ID.instruction >> 6) & 0b111111;
            ID.r2 = (ID.instruction) & (0b111111);
            ID.val1 = read_reg(ID.r1);
            ID.val2 = read_reg(ID.r2);
            printf("r1 --> Register %d, value = %d \n", ID.r1, (int) ID.val1);
            printf("r2 --> Register %d, value = %d \n", ID.r2, (int) ID.val2);
        break;
        case 3:
        case 4:
        case 5:
        case 8:
        case 9:
            ID.r1 = (ID.instruction >> 6) & 0b111111;
            ID.val1 = read_reg(ID.r1);
            int8_t raw_imm = (ID.instruction) & (0b111111);
            ID.imm = (raw_imm & 0x20) ? (int8_t)(raw_imm | ~0x3F) : (int8_t)raw_imm;          
            printf("r1 --> Register %d, value = %d \n", ID.r1, (int) ID.val1);
            printf("Immediate = %d \n", ID.imm);
        break;
        default:
            ID.r1 = (ID.instruction >> 6) & 0b111111;
            ID.val1 = read_reg(ID.r1);
            ID.imm = (ID.instruction) & (0b111111);
            printf("r1 --> Register %d, value = %d \n", ID.r1, (int) ID.val1);
            printf("Memory Address = %d\n", ID.imm);
        break;
    }
    
    printf("Instruction %d decoded\n", ID.inst_id);
}


void execute(){
    if (!IE.valid)
        return;
    clear_flags();
    printf("\033[1mEXECUTE\033[0m\n");
    printf("Executing \033[1mInstruction %d\033[0m\n", IE.inst_id);
    switch(IE.opcode){
        case 3:{
            IE.result = IE.imm;
            if (IE.r1!=0){
                write_reg(IE.r1, IE.imm);
                printf("MOVI: value %d moved into Register %d\n",read_reg(IE.r1), IE.r1);
            }
            else
                printf("MOVI instruction failed, R0 cannot be overwritten \n");
        break;}
        case 10:{
            int8_t load_value = load_data(IE.imm);
            if (IE.r1!=0){
                write_reg(IE.r1, load_value);
                printf("LDR: value %d loaded into Register %d\n", read_reg(IE.r1), IE.r1);
            }
            else
                printf("LDR instruction failed, R0 cannot be overwritten",read_reg(IE.r1), IE.r1);
        break;}
        case 11:{
            int8_t store_value = IE.val1;
            store_data(store_value, IE.imm);
            printf("STR: value %d from Register %d stored in data memory[%d]\n", read_reg(IE.r1), IE.r1, IE.imm);
        break;}
        case 4:{
            IE.result = Alu(IE.val1, IE.val2,IE.opcode, IE.imm);
            printf("ALU result = %d\n", IE.result);
            if (IE.val1 == 0){
                IE.branch_taken = 1;
                set_pc(IE.result);
                printf("BEQZ executed\n");
            }
        break;
        }
        case 7:{
            IE.branch_taken = 1;
            IE.result = Alu(IE.val1, IE.val2,IE.opcode, IE.imm);
            set_pc(IE.result);
            printf("ALU result = %d\n", IE.result);
            printf("BR executed\n");
        break;
        }
        default:{
            IE.result = Alu(IE.val1, IE.val2,IE.opcode, IE.imm);  
            printf("ALU result = %d\n", IE.result);
            if (IE.r1!=0){
                write_reg(IE.r1,IE.result);
                printf("Value inside Register %d updated\n",IE.r1);
            }
            else
                printf("Execution failed. R0 cannot be overwritten.\n");
        break;
        }
    }

    IE.valid=0;//commented in deb pipeline
    printf("Instruction %d Executed\n", IE.inst_id);
    print_nonzero_gprs();
    print_nonzero_data();
    printFlags();
}

void run_program(){
    loadProgram("program5.txt");
    no_of_instructions = get_no_of_instructions();
    if (no_of_instructions==0)
        return;

    while (!(end_of_instructions && !IE.valid && !ID.valid && !IF.valid )) {
        printf("------------------\033[3m\033[1mCycle %d\033[0m\033[0m------------------\n", clock);

        if(IE.valid){
            execute ();

            if(IE.branch_taken){
                
                    printf("CONTROL HAZARD: Branch Taken! Flushing IF and ID buffers.\n");
                    
                    //Destroy the wrong instructions (flush)
                    IF.valid = 0;
                    ID.valid = 0;
                    countf=-1;
                    IE.branch_taken = 0; // Reset branch taken flag for the next instruction
                    clock++;
                    continue;

            }
            printf("End of EXECUTE stage.\n", IE.inst_id);
            printf("\n");
        }

        if(ID.valid){
            decode ();
            printf("End of DECODE stage. \n", ID.inst_id);
            printf("\n");
        }
        if (!IF.valid && !end_of_instructions){
            fetch_inst();
            printf("End of FETCH stage\n");
            printf("\n");
        }

        printf("Transitions, if available: \n");
        if (!IE.valid && ID.valid){
            IE = ID;
            IE.valid = 1; 
            ID.valid = 0;
            printf("Instruction %d : passed to EXECUTE stage \n", IE.inst_id);
        }
        if (!ID.valid && IF.valid){
            ID = IF;
            ID.valid = 1;
            IF.valid = 0;
            printf("Instruction %d : passed to DECODE stage \n", ID.inst_id);
        }
        
        
        fflush(stdout); // ensures text appears before sleeping
        SLEEP_SECOND(); 

        clock++;
    }
}


extern int carryFlag;
extern int overflowFlag;
extern int negativeFlag;
extern int signFlag;
extern int zeroFlag;

void print_final_state() {
    printf("\n.........................................\n");
    printf("         \033[1mFINAL PROCESSOR STATE\033[0m\n");
    printf(".........................................\n");
    printf("\033[1mProgram Counter (PC):\033[0m %d\n", get_pc());

    //Status Register (SREG)
    printf("\033[1mStatus Register (SREG) Flags:\033[0m\n");
    // printf("  Carry: %d | Overflow: %d | Negative: %d | Sign: %d | Zero: %d\n", 
    //         carryFlag, overflowFlag, negativeFlag, signFlag, zeroFlag);
    printFlags();

    //Register File
    printf("\n... \033[1mGeneral Purpose Registers\033[0m ...\n");
    for (int i = 0; i < 64; i++) {
        int8_t val = read_reg(i);
        printf("R%d: %d\n", i, val);
    }

    //Data Memory
    printf("\n... \033[1mData Memory\033[0m ...\n");
    for (int i = 0; i < 1024; i++) {
        int8_t val = load_data(i);
        if (val != 0) { // Only printing non-zero memory 34an keda hanroo7 libya 
            printf("Address [%d] = %d\n", i, val);
        }
    }

    printf(".........................................\n\n");
}





int main (){
    init_pipeline();
    run_program();
    // printf("final pc= %d", get_pc());
    print_final_state();
    print_instruction_memory();

    return 0;
}