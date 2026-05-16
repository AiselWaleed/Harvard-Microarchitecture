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

PipelineStage IF= {0};
PipelineStage ID= {0};
PipelineStage IE= {0};

void init_pipeline(){
    clock =0;
    // global_pc = 0;
    no_of_instructions = 0;
    current_instruction = 0;
    end_of_instructions = 0;
    printf("init_pipeline: Pipelining initiated \n");
    //TEMP: sample parsed insts, later to be taken from memory
    // instruction_memory [0] = (0b1111000011110000);
    // instruction_memory [1] = (0b1101010011110000);    
    // instruction_memory [2] = (0b1111001100110000);
    // instruction_memory [3] = (0b1100111011110000);
    // instruction_memory [4] = (0b1000110011110000);
    printf("init_pipeline: Instructions in place \n");
}

void print_binary16(uint16_t num) {
    // Loop from the most significant bit (bit 15) down to the least (bit 0)
    for (int i = 15; i >= 0; i--) {
        // Shift 1 to the i-th position and check if that bit is set
        int bit = (num >> i) & 1;
        printf("%d", bit);
        
        // Optional: Add a space every 4 bits for readability
        if (i % 4 == 0) printf(" ");
    }
    printf("\n");
}
// int get_no_of_inst(short int inst_mem[]){
//     int size = 0;
//     int i=0;
//     while (i<INST_MEM_SIZE){
//         if (inst_mem[i]!=0)
//             size++;
//         i++;
//     }

//     printf("get_no_of_inst: size of array = %d \n", size);
//     return size;
// }

void fetch_inst(){
    printf("fetch_inst: Current pc = %d \n", get_pc());

    short int fetched_instruction = fetch_instruction();
        if (fetched_instruction == -1 || current_instruction==get_no_of_instructions()){
            printf("fetch_inst: No more instructions to fetch");
            IF.valid =0;
            end_of_instructions = 1;
        }
        else{
            IF.instruction = fetched_instruction;
            IF.pc = get_pc()-1;
            IF.valid = 1;
            IF.inst_id = ++current_instruction;
            print_binary16((short int)fetched_instruction);
        }
    //data check?
}

void decode(){
    if (!ID.valid)
        return;
    printf("this is the decode method, decoding instruction %d \n", ID.inst_id);
    int current_opcode = (ID.instruction >> 12) & 0b1111 ;
    ID.opcode = current_opcode;
    printf("decode: opcode = %d\n", ID.opcode);
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
            printf("decode: r1 = Register %d = %d \n", ID.r1, (int) ID.val1);
            printf("decode: r2 = Register %d = %d \n", ID.r2, (int) ID.val2);
        break;
        case 3:
        case 4:
        case 5:
        case 8:
        case 9:
            ID.r1 = (ID.instruction >> 6) & 0b111111;
            ID.val1 = read_reg(ID.r1);
            // int8_t raw_imm = ID.imm & 0x3F;
            int8_t raw_imm = (ID.instruction) & (0b111111);
            ID.imm = (raw_imm & 0x20) ? (int8_t)(raw_imm | ~0x3F) : (int8_t)raw_imm;          
            printf("decode: r1 = Register %d = %d \n", ID.r1, (int) ID.val1);
            printf("decode: immediate = %d", ID.imm);
        break;
        default:
            ID.r1 = (ID.instruction >> 6) & 0b111111;
            ID.val1 = read_reg(ID.r1);
            ID.imm = (ID.instruction) & (0b111111);
            printf("decode: r1 = Register %d = %d \n", ID.r1, (int) ID.val1);
            printf("decode: address = %d\n", ID.imm);
        break;
    }
    // }
    // ID.r1 = (ID.instruction >> 6) & 0b111111;
    // ID.r2 = (ID.instruction) & (0b111111);
    // // ID.imm = (ID.instruction) & (0b111111);
    // int8_t raw_imm = IE.imm & 0x3F;
    // ID.imm = (raw_imm & 0x20) ? (int8_t)(raw_imm | ~0x3F) : (int8_t)raw_imm;
    // ID.r1 = ID.instruction & (0b111111 << 6);
    // ID.r2 = ID.instruction & (0b111111);
    // ID.imm = ID.instruction & (0b111111);
    printf("decode : Instruction %d decoded\n", ID.inst_id);
}


void execute(){
    if (!IE.valid)
        return;
    printf("this is the execute method, executing instruction %d \n", IE.inst_id);
    switch(IE.opcode){
        case 3:
            write_reg(IE.r1, IE.imm);
            printf("execute: value %d moved immediately into Register %d\n",read_reg(IE.r1), IE.r1);
        break;
        case 10:
            int8_t load_value = load_data(IE.imm);
            write_reg(IE.r1, load_value);
            printf("execute: value %d loaded into Register %d\n", read_reg(IE.r1), IE.r1);
        break;
        case 11:
            int8_t store_value = IE.val1;
            store_data(store_value, IE.imm);
            printf("execute: value %d from Register %d stored in memory\n", read_reg(IE.r1), IE.r1);
        break;
        case 4:
            set_pc(Alu(IE.val1, IE.val2,IE.opcode, IE.imm));
            printf("execute: BEQZ executed\n");
        break;
        case 7:
            set_pc(Alu(IE.val1, IE.val2,IE.opcode, IE.imm));
            printf("execute: BR executed\n");
        break;
        default:
            write_reg(IE.r1,Alu(IE.val1, IE.val2,IE.opcode, IE.imm));
            printf("execute: value inside Register %d updated\n",IE.r1);
        break;
    }
    IE.result = Alu(IE.val1, IE.val2,IE.opcode, IE.imm);
    //TEMP
    // IE.val1=66;
    // IE.val2=88;
    // IE.result = 44;
    IE.valid=0;//commented in deb pipeline
    printf("execute: Instruction %d Executed\n", IE.inst_id);
}

void run_program(){
    loadProgram("program1.txt");
    no_of_instructions = get_no_of_instructions();
    if (no_of_instructions==0)
        return;

    while (!(end_of_instructions && !IE.valid && !ID.valid && !IF.valid )) {
        printf("......run_program: Cycle %d ...... \n", clock);

        if(IE.valid){
            execute ();
            //IE is then invalidated
            printf("run_program: instruction %d executed \n", IE.inst_id);
            if ((IE.opcode == 4 && IE.val1 == 0) || (IE.opcode == 7)) {
                
                    printf("CONTROL HAZARD: Branch Taken! Flushing IF and ID buffers.\n");
                    
                    // 1. Destroy the wrong instructions
                    IF.valid = 0;
                    ID.valid = 0;
                    
                    // 2. Force the Memory's PC to the new branch target
                    // IE.result holds the new PC calculated by the ALU
                    // pc= IE.result; // get current PC
                    uint16_t new_pc = (0x00FF) & IE.result;
                    set_pc(new_pc);
            }

        }

        if(ID.valid){
            decode ();
            printf("run_program: instruction %d decoded \n", ID.inst_id);
        }
        if (!IF.valid && !end_of_instructions){
            fetch_inst();
            if (IF.valid){
                printf("run_program: instruction %d is fetched \n", IF.inst_id);
            }
        }


        if (!IE.valid && ID.valid){
            // int hazard = is_data_hazard();
            // if (!hazard){
            IE = ID;
            IE.valid = 1;
            ID.valid = 0;
            printf("instruction %d is passed to execute phase \n", IE.inst_id);
            // }
            // else{ //HAZARD!
            //     printf("run_program: Stalling pipeline due to hazard in instruction %d \n", ID.inst_id);
            //     IE.valid = 0; //stall IE
            // }
           
        }
        if (!ID.valid && IF.valid){
            ID = IF;
            ID.valid = 1;
            IF.valid = 0;
            printf("instruction %d is passed to decode phase \n", ID.inst_id);
        }
        
        
        fflush(stdout); // ensures text appears before sleeping
        SLEEP_SECOND(); 

        clock++;
    }
}


int main (){
    init_pipeline();
    run_program();
    return 0;
}
