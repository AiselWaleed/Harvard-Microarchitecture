#include <stdio.h>
#include <math.h>
#include <stdint.h>
#define INST_MEM_SIZE 1024
#include "../include/memory.h"
#include "../include/pipeline.h"
#include "../include/alu.h"
int zeroFlag;
int carryFlag;
int overflowFlag;
int negativeFlag;
int signFlag;

int mem[64];

int BtoD(int b[], int size){
    int dec=0, c=0;
    for(int i=size-1; i>=0; i--){
        dec = dec + (b[i]*pow(2,c));
        c++;
    }
    return dec;
}

int* DtoB(int decimal, int result[], int size){
    for(int i=size-1; i>=0; i--){
        result[i] = decimal % 2;
        decimal = decimal / 2;
    }
    return result;
}

int updateCarry(int flag, int8_t a, int8_t b, int opcode){
    if(opcode == 0){ //ADD
        uint16_t s = (uint16_t)((uint8_t)a) + (uint16_t)((uint8_t)b);
        // flag = s > 0xFF;
        flag = (s >> 8) & 1;
    }

    else{
        flag = 0;
    }
    return flag;
}
void clear_flags(){
    zeroFlag     = 0;
    carryFlag    = 0;
    overflowFlag = 0;
    negativeFlag = 0;
    signFlag     = 0;
}

// int updateCarry(int flag, int8_t a, int8_t b, int opcode){
//     if(opcode == 0){ //ADD
//         uint16_t s = (uint16_t)(uint8_t)a + (uint16_t)(uint8_t)b;
//         // flag = s > 0x7F;
//         flag = (s & 0x0100 == 0x0100);
//     }
//     // else if(opcode == 1){
//     //     flag = ((uint8_t)a < (uint8_t)b) ? 1 : 0;
//     // }
//     // else{
//     //     flag = 0;
//     // }
//     return flag;
// }


int updateOverFlow(int flag, int8_t reg1, int8_t reg2, int opcode, int8_t result){
    if(opcode == 0){
        int s1  = (reg1   >> 7) & 1;
        int s2  = (reg2   >> 7) & 1;
        int sr  = (result >> 7) & 1;
        flag = (s1 == s2 && sr != s1) ? 1 : 0;
    }
    else if(opcode == 1){
        int s1  = (reg1   >> 7) & 1;
        int s2  = (reg2   >> 7) & 1;
        int sr  = (result >> 7) & 1;
        flag = (s1 != s2 && sr != s1) ? 1 : 0;
    }
    else{
        flag = 0;
    }
    return flag;
}

int updateNegative(int flag, int8_t out){
    flag = (out < 0) ? 1 : 0;
    return flag;
}

//A: should be updated whenever either is
int updateSign(int signFlag, int negativeFlag, int overflowFlag){
    signFlag = (negativeFlag != overflowFlag) ? 1 : 0;
    return signFlag;
}

int updateZero(int flag, int8_t out){
    flag = (out == 0) ? 1 : 0;
    return flag;
}

int8_t Alu(int8_t operandA, int8_t operandB, int opcode, int8_t imm){
    int8_t outputD = 0;

    zeroFlag     = 0;
    carryFlag    = 0;
    overflowFlag = 0;
    negativeFlag = 0;
    signFlag     = 0;

    switch(opcode){

        /* 0: ADD — R1 = R1 + R2 */
        case 0:
            outputD      = operandA + operandB;
            carryFlag    = updateCarry(carryFlag, operandA, operandB, 0);
            overflowFlag = updateOverFlow(overflowFlag, operandA, operandB, 0, outputD);
            negativeFlag = updateNegative(negativeFlag, outputD);
            signFlag     = updateSign(signFlag, negativeFlag, overflowFlag);
            zeroFlag     = updateZero(zeroFlag, outputD);
            break;

        /* 1: SUB — R1 = R1 - R2  (C not updated per spec; only V, N, S, Z) */
        case 1:
            outputD      = operandA - operandB;
            overflowFlag = updateOverFlow(overflowFlag, operandA, operandB, 1, outputD);
            negativeFlag = updateNegative(negativeFlag, outputD);
            signFlag     = updateSign(signFlag, negativeFlag, overflowFlag);
            zeroFlag     = updateZero(zeroFlag, outputD);
            break;

        /* 2: MUL — R1 = R1 * R2 */
        case 2:
            outputD      = operandA * operandB;
            negativeFlag = updateNegative(negativeFlag, outputD);
            zeroFlag     = updateZero(zeroFlag, outputD);
            break;

        /* 3: MOVI — R1 = IMM */
        case 3:
            outputD = imm;
            break;

        /* 4: BEQZ — if R1==0: PC = PC+1+IMM */
        case 4:
        //EDIT PC
            if(operandA==0){
                // pc= pc+1+ imm;
                outputD=(int8_t)(IE.pc+1+imm);
            }
            else{
                // pc= pc+1;
                outputD=(int8_t)(get_pc());
            }
            // outputD=pc;
            break;

        /* 5: ANDI — R1 = R1 & IMM  (was wrongly R1 & R2) */
        case 5:
            outputD      = operandA & imm;
            negativeFlag = updateNegative(negativeFlag, outputD);
            zeroFlag     = updateZero(zeroFlag, outputD);
            break;

        /* 6: EOR — R1 = R1 XOR R2  (was wrongly OR) */
        case 6:
            outputD      = operandA ^ operandB;
            negativeFlag = updateNegative(negativeFlag, outputD);
            zeroFlag     = updateZero(zeroFlag, outputD);
            break;

        /* 7: BR — PC = R1 || R2 (concatenation, caller reads pc) */
        case 7:
            outputD      = ((int)(uint8_t)operandA << 8) | (uint8_t)operandB;//A: CHEECK IN EXECUTE()
            // outputD = 0;   /* result too wide for int8_t; caller reads pc */
            //pc      = ((int)(uint8_t)operandA << 8) | (uint8_t)operandB;//A: CHEECK IN EXECUTE()
            outputD = ((int)(uint8_t)operandA << 8) | (uint8_t)operandB;
            break;

        /* 8: SLC — R1 = R1 << IMM | R1 >>> (8-IMM)  circular left shift */
        case 8: {
            uint8_t ua  = (uint8_t)operandA;
            uint8_t amt = (uint8_t)imm & 7;          /* clamp to 0-7 bits */
            outputD      = (int8_t)((ua << amt) | (ua >> (8 - amt)));
            negativeFlag = updateNegative(negativeFlag, outputD);
            zeroFlag     = updateZero(zeroFlag, outputD);
            break;
        }

        /* 9: SRC — R1 = R1 >>> IMM | R1 << (8-IMM)  circular right shift */
        case 9: {
            uint8_t ua  = (uint8_t)operandA;
            uint8_t amt = (uint8_t)imm & 7;
            outputD      = (int8_t)((ua >> amt) | (ua << (8 - amt)));
            negativeFlag = updateNegative(negativeFlag, outputD);
            zeroFlag     = updateZero(zeroFlag, outputD);
            break;
        }

        /* 10: LDR — R1 = MEM[ADDRESS] */
        case 10:
            outputD = load_data(imm);
            break;

        /* 11: STR — MEM[ADDRESS] = R1, then read back */
        case 11:
            store_data(operandA, imm);
            outputD = load_data(imm);
            break;
    }

    return outputD;
}

void printFlags(){
    printf("Flags -> C:%d  V:%d  N:%d  S:%d  Z:%d\n",carryFlag, overflowFlag, negativeFlag, signFlag, zeroFlag);
}

// int main(){
//     int8_t result;

//     /* ── CASE 0: ADD ── */
//     result = Alu((int8_t)5, (int8_t)3, 0, (int8_t)0);
//     printf("Case 0 ADD (5+3):                output = %d  (expected 8)\n", result);
//     printFlags();

//     result = Alu((int8_t)127, (int8_t)1, 0, (int8_t)0);
//     printf("Case 0 ADD carry (127+1):        output = %d  (expected -128, C=1, V=1)\n", result);
//     printFlags();

//     result = Alu((int8_t)-10, (int8_t)-20, 0, (int8_t)0);
//     printf("Case 0 ADD neg (-10+-20):        output = %d  (expected -30, C=1, N=1, S=1)\n", result);
//     printFlags();

//     /* ── CASE 1: SUB ── */
//     result = Alu((int8_t)7, (int8_t)2, 1, (int8_t)0);
//     printf("Case 1 SUB (7-2):                output = %d  (expected 5)\n", result);
//     printFlags();

//     result = Alu((int8_t)4, (int8_t)4, 1, (int8_t)0);
//     printf("Case 1 SUB zero (4-4):           output = %d  (expected 0, Z=1)\n", result);
//     printFlags();

//     result = Alu((int8_t)3, (int8_t)10, 1, (int8_t)0);
//     printf("Case 1 SUB neg (3-10):           output = %d  (expected -7, N=1, S=1)\n", result);
//     printFlags();

//     result = Alu((int8_t)0, (int8_t)1, 1, (int8_t)0);
//     printf("Case 1 SUB borrow (0-1):         output = %d  (expected -1, N=1, S=1)\n", result);
//     printFlags();

//     /* ── CASE 2: MUL ── */
//     result = Alu((int8_t)3, (int8_t)4, 2, (int8_t)0);
//     printf("Case 2 MUL (3*4):                output = %d  (expected 12)\n", result);
//     printFlags();

//     result = Alu((int8_t)5, (int8_t)0, 2, (int8_t)0);
//     printf("Case 2 MUL zero (5*0):           output = %d  (expected 0, Z=1)\n", result);
//     printFlags();

//     result = Alu((int8_t)-3, (int8_t)4, 2, (int8_t)0);
//     printf("Case 2 MUL neg (-3*4):           output = %d  (expected -12, N=1)\n", result);
//     printFlags();

//     /* ── CASE 3: MOVI ── */
//     result = Alu((int8_t)0, (int8_t)0, 3, (int8_t)21);
//     printf("Case 3 MOVI (load 21):           output = %d  (expected 21)\n", result);
//     printFlags();

//     result = Alu((int8_t)0, (int8_t)0, 3, (int8_t)-5);
//     printf("Case 3 MOVI (load -5):           output = %d  (expected -5)\n", result);
//     printFlags();

//     /* ── CASE 4: BEQZ ── */
//     pc = 0;
//     result = Alu((int8_t)0, (int8_t)0, 4, (int8_t)5);
//     printf("Case 4 BEQZ taken (R1=0,IMM=5):  output = %d  (expected 6)\n", result);
//     printFlags();

//     result = Alu((int8_t)1, (int8_t)0, 4, (int8_t)5);
//     printf("Case 4 BEQZ not taken (R1=1):    output = %d  (expected 7)\n", result);
//     printFlags();

//     result = Alu((int8_t)0, (int8_t)0, 4, (int8_t)10);
//     printf("Case 4 BEQZ taken (IMM=10):      output = %d  (expected 18)\n", result);
//     printFlags();

//     /* ── CASE 5: ANDI — R1 & IMM ── */
//     result = Alu((int8_t)15, (int8_t)0, 5, (int8_t)6);
//     printf("Case 5 ANDI (15 & IMM=6):        output = %d  (expected 6)\n", result);
//     printFlags();

//     result = Alu((int8_t)0b1010, (int8_t)0, 5, (int8_t)0b0101);
//     printf("Case 5 ANDI zero (10 & IMM=5):   output = %d  (expected 0, Z=1)\n", result);
//     printFlags();

//     /* ── CASE 6: EOR — R1 XOR R2 ── */
//     result = Alu((int8_t)12, (int8_t)10, 6, (int8_t)0);
//     printf("Case 6 EOR (12 ^ 10):            output = %d  (expected 6)\n", result);
//     printFlags();

//     result = Alu((int8_t)7, (int8_t)7, 6, (int8_t)0);
//     printf("Case 6 EOR same (7 ^ 7):         output = %d  (expected 0, Z=1)\n", result);
//     printFlags();

//     /* ── CASE 7: BR — PC = R1 || R2 ── */
//     Alu((int8_t)2, (int8_t)3, 7, (int8_t)0);
//     printf("Case 7 BR  (2||3):               pc = %d  (expected 131)\n", pc);
//     printFlags();

//     Alu((int8_t)1, (int8_t)0, 7, (int8_t)0);
//     printf("Case 7 BR  (1||0):               pc = %d  (expected 64)\n", pc);
//     printFlags();

//     /* ── CASE 8: SLC — circular left shift ── */
//     /* 0b00000011 << 2 circular = 0b00001100 = 12 */
//     result = Alu((int8_t)0b00000011, (int8_t)0, 8, (int8_t)2);
//     printf("Case 8 SLC (3 <<< 2):            output = %d  (expected 12)\n", result);
//     printFlags();

//     /* 0b10000001 << 1 circular = 0b00000011 = 3 */
//     result = Alu((int8_t)0b10000001, (int8_t)0, 8, (int8_t)1);
//     printf("Case 8 SLC (0x81 <<< 1):         output = %d  (expected 3)\n", result);
//     printFlags();

//     /* ── CASE 9: SRC — circular right shift ── */
//     /* 0b00001100 >> 2 circular = 0b00000011 = 3 */
//     result = Alu((int8_t)0b00001100, (int8_t)0, 9, (int8_t)2);
//     printf("Case 9 SRC (12 >>> 2):           output = %d  (expected 3)\n", result);
//     printFlags();

//     /* 0b00000011 >> 1 circular = 0b10000001 = -127 */
//     result = Alu((int8_t)0b00000011, (int8_t)0, 9, (int8_t)1);
//     printf("Case 9 SRC (3 >>> 1):            output = %d  (expected -127)\n", result);
//     printFlags();

//     /* ── CASE 10: LDR — R1 = MEM[ADDRESS] ── */
//     /* store first via STR so load_data/store_data share the same buffer */
//     Alu((int8_t)42, (int8_t)0, 11, (int8_t)0);   /* STR 42 → countd=0 */
//     result = Alu((int8_t)0, (int8_t)0, 10, (int8_t)0);
//     printf("Case 10 LDR (MEM[0]=42):         output = %d  (expected 42)\n", result);
//     printFlags();

//     Alu((int8_t)99, (int8_t)0, 11, (int8_t)1);   /* STR 99 → countd=1 */
//     result = Alu((int8_t)0, (int8_t)0, 10, (int8_t)1);
//     printf("Case 10 LDR (MEM[1]=99):         output = %d  (expected 99)\n", result);
//     printFlags();

//     /* ── CASE 11: STR — MEM[ADDRESS] = R1 ── */
//     result = Alu((int8_t)26, (int8_t)0, 11, (int8_t)2);
//     printf("Case 11 STR (store 26 → MEM[2]): output = %d  (expected 26)\n", result);
//     printFlags();

//     result = Alu((int8_t)-15, (int8_t)0, 11, (int8_t)3);
//     printf("Case 11 STR (store -15→ MEM[3]): output = %d  (expected -15)\n", result);
//     printFlags();

//     return 0;
// }