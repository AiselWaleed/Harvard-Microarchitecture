
uint16_t instruction_memory[1024];
int counti=0;
int countd=0;
uint8_t data_memory[2048];
int8_t gpr[64]={0};
uint8_t sreg;
uint16_t pc;

void init_memory(){
    for(int i=0;i<1024;i++){
        instruction_memory[i]=0;
    }
    for(int i=0;i<2048;i++){
        data_memory[i]=0;
    }
}

void write_instruction(uint16_t instruction){
    instruction_memory[counti]=instruction;
    counti++;
}

uint16_t fetch_instruction(){
    return instruction_memory[pc++];
}

uint8_t load_data(int index){
    return data_memory[index];
}

void store_data(uint8_t data){
    data_memory[countd++]=data;
}

int8_t read_reg(int index){
    return gpr[index];
}

void write_reg(int index, int8_t data){
    gpr[index]=data;
}