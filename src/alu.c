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


/* added a global memory array because cases 10 and 11 (LB and SB)
   use mem[] but it was never declared anywhere in the original code.
   64 entries is enough for a basic simulator. */
int mem[64];

/*helper function to make the value of binary to decimal*/
int BtoD (int b[],int size){
    int dec=0;
    int c=0;
    for(int i=size-1;i>=0; i--){
        dec= dec + (b[i]*pow(2,c));
        c++;
    }
    return dec;
}

/*helper function to make the value of decimal to binary*/

int* DtoB(int decimal,int result[], int size){
    for(int i = size - 1; i >= 0; i--){
        result[i] = decimal % 2;
        decimal = decimal / 2;
    }
 
    return result;
}


int updateCarry(int flag, int8_t a, int8_t b, int opcode) {
    /*
     * For ADD: carry occurs when the unsigned sum exceeds 8 bits.
     * Cast to uint8_t (unsigned) before widening to uint16_t so we
     * capture the true carry-out without sign-extension artefacts.
     */
    if (opcode == 0) {
        uint16_t sum = (uint16_t)(uint8_t)a + (uint16_t)(uint8_t)b;
        flag = (sum > 0xFF) ? 1 : 0;
    }
    /*
     * For SUB: a borrow (carry-out of the complemented add) occurs
     * when the unsigned value of a is less than that of b.
     */
    else if (opcode == 1) {
        flag = ((uint8_t)a < (uint8_t)b) ? 1 : 0;
    }
    else {
        flag = 0;
    }
    return flag;
}

int updateOverFlow(int flag, int8_t reg1, int8_t reg2, int opcode, int8_t result) {
    /*
     * Overflow on ADD: result sign differs from both inputs' sign
     * (both positive → negative, or both negative → positive).
     * Extracts the MSB (sign bit) via >> 7 and & 1.
     */
    if (opcode == 0) {
        int s1  = (reg1   >> 7) & 1;
        int s2  = (reg2   >> 7) & 1;
        int sr  = (result >> 7) & 1;
        flag = (s1 == s2 && sr != s1) ? 1 : 0;
    }
    /*
     * Overflow on SUB: result sign differs from minuend's sign when
     * the two operands had opposite signs (positive − negative or
     * negative − positive wrapped around).
     */
    else if (opcode == 1) {
        int s1  = (reg1   >> 7) & 1;
        int s2  = (reg2   >> 7) & 1;
        int sr  = (result >> 7) & 1;
        flag = (s1 != s2 && sr != s1) ? 1 : 0;
    }
    else {
        flag = 0;
    }
    return flag;
}


int updateNegative(int flag,int8_t out){
    if(out<0){
        flag=1;}
    else{
        flag=0;}
    return flag;
}

int updateSign(int signFlag, int negativeFlag, int overflowFlag){
    /*XORing the negative and overflow Flags*/
    if(negativeFlag!=overflowFlag){
        signFlag=1;}
    else{
        signFlag=0;}
    return signFlag;
}

int updateZero(int flag,int8_t out){
    if(out==0){
        flag=1;
    }
    else{
        flag=0;}
        return flag;
}


/* CHANGED: return type changed to int to return the decimal output value */
int8_t Alu (int8_t operandA,int8_t operandB,int opcode,int8_t imm){
    int8_t outputD=0;
    
     zeroFlag=0;
     carryFlag=0;
     overflowFlag=0;
     negativeFlag=0;
     signFlag=0;


     switch(opcode){

        case 0:
             outputD = operandA + operandB;
            carryFlag    = updateCarry(carryFlag, operandA, operandB, 0);
            overflowFlag = updateOverFlow(overflowFlag, operandA, operandB, 0, outputD);
            negativeFlag = updateNegative(negativeFlag, outputD);
            signFlag     = updateSign(signFlag, negativeFlag, overflowFlag);
            zeroFlag     = updateZero(zeroFlag, outputD);
        break;

       case 1:
            outputD = operandA - operandB;
            carryFlag    = updateCarry(carryFlag, operandA, operandB, 1);
            overflowFlag = updateOverFlow(overflowFlag, operandA, operandB, 1, outputD);
            negativeFlag = updateNegative(negativeFlag, outputD);
            signFlag     = updateSign(signFlag, negativeFlag, overflowFlag);
            zeroFlag     = updateZero(zeroFlag, outputD);

    break;

        case 2:
            outputD=operandA*operandB;

            negativeFlag= updateNegative(negativeFlag,outputD);
            zeroFlag= updateZero(zeroFlag,outputD);
            break;

        case 3:
            operandA=imm;
            outputD=operandA;
            break;

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

        case 5:
            outputD=operandA&imm;
            negativeFlag= updateNegative(negativeFlag,outputD);
            zeroFlag= updateZero(zeroFlag,outputD);
            break;
         
         case 6:
            outputD= operandA^operandB;

            negativeFlag= updateNegative(negativeFlag,outputD);
            zeroFlag= updateZero(zeroFlag,outputD);
            break;
            
        case 7:
            // pc= (operandA<<6)|operandB;
            outputD=(operandA<<6)|operandB;
            break;

        case 8:
            outputD=operandA<<imm;

            negativeFlag= updateNegative(negativeFlag,outputD);
            zeroFlag= updateZero(zeroFlag,outputD);
            break;
        
         case 9:
            outputD=operandA>>imm;

            negativeFlag= updateNegative(negativeFlag,outputD);
            zeroFlag= updateZero(zeroFlag,outputD);
            break; 
            
        case 10:
            operandA= load_data(imm);
            outputD=operandA;
            break;

        case 11:
            store_data(operandA, (uint16_t)imm);
            outputD = load_data((uint16_t)imm);
            break;
     }

return outputD;

}

void printFlags(){
    printf("  Flags -> C:%d  V:%d  N:%d  S:%d  Z:%d\n",
           carryFlag, overflowFlag, negativeFlag, signFlag, zeroFlag);
}
/*ask about how to deal with the PC and figure out their expected return type is it always binary or what?*/
// //  int main() {
//     int8_t result;

//     /* CASE 0 — ADD: 5 + 3 = 8 */
//     result = Alu((int8_t)5, (int8_t)3, 0, (int8_t)0);
//     printf("Case 0 ADD (5+3):        output = %d  (expected 8)\n", result);
//     printFlags();

//     /* CASE 0 — ADD carry: 127 + 1 = -128 in int8_t, carry=1 */
//     result = Alu((int8_t)127, (int8_t)1, 0, (int8_t)0);
//     printf("Case 0 ADD carry(127+1): output = %d  (expected -128, C=1, V=1)\n", result);
//     printFlags();

//     /* CASE 1 — SUB: 7 - 2 = 5 */
//     result = Alu((int8_t)7, (int8_t)2, 1, (int8_t)0);
//     printf("Case 1 SUB (7-2):        output = %d  (expected 5)\n", result);
//     printFlags();

//     /* CASE 1 — SUB zero: 4 - 4 = 0, Z=1 */
//     result = Alu((int8_t)4, (int8_t)4, 1, (int8_t)0);
//     printf("Case 1 SUB zero (4-4):   output = %d  (expected 0, Z=1)\n", result);
//     printFlags();

//     /* CASE 2 — MUL: 3 * 4 = 12 */
//     result = Alu((int8_t)3, (int8_t)4, 2, (int8_t)0);
//     printf("Case 2 MUL (3*4):        output = %d  (expected 12)\n", result);
//     printFlags();

//     /* CASE 3 — LDI: load immediate 21 into output */
//     result = Alu((int8_t)0, (int8_t)0, 3, (int8_t)21);
//     printf("Case 3 LDI (load 21):    output = %d  (expected 21)\n", result);
//     printFlags();

//     /* CASE 4 — BEQZ taken: R1=0, IMM=5, pc=0 → pc = 0+1+5 = 6 */
//     pc = 0;
//     result = Alu((int8_t)0, (int8_t)0, 4, (int8_t)5);
//     printf("Case 4 BEQZ taken:       output = %d  (expected 6)\n", result);
//     printFlags();

//     /* CASE 4 — BEQZ not taken: R1=1, pc=6 → pc = 6+1 = 7 */
//     result = Alu((int8_t)1, (int8_t)0, 4, (int8_t)5);
//     printf("Case 4 BEQZ not taken:   output = %d  (expected 7)\n", result);
//     printFlags();

//     /* CASE 5 — AND: 15 & 6 = 6 */
//     result = Alu((int8_t)15, (int8_t)6, 5, (int8_t)0);
//     printf("Case 5 AND (15 & 6):     output = %d  (expected 6)\n", result);
//     printFlags();

//     /* CASE 6 — OR: 10 | 5 = 15 */
//     result = Alu((int8_t)10, (int8_t)5, 6, (int8_t)0);
//     printf("Case 6 OR  (10 | 5):     output = %d  (expected 15)\n", result);
//     printFlags();

//     /* CASE 7 — JR: (2 << 6) | 3 = 131 */
//     result = Alu((int8_t)2, (int8_t)3, 7, (int8_t)0);
//     printf("Case 7 JR  (2||3):       output = %d  (expected 131 in pc)\n", result);
//     printFlags();

//     /* CASE 8 — SAL: 3 << 2 = 12 */
//     result = Alu((int8_t)3, (int8_t)0, 8, (int8_t)2);
//     printf("Case 8 SAL (3<<2):       output = %d  (expected 12)\n", result);
//     printFlags();

//     /* CASE 9 — SAR: 12 >> 2 = 3 */
//     result = Alu((int8_t)12, (int8_t)0, 9, (int8_t)2);
//     printf("Case 9 SAR (12>>2):      output = %d  (expected 3)\n", result);
//     printFlags();

//     /* CASE 10 — LB: load from mem[5] = 42 */
//     mem[5] = 42;
//     result = Alu((int8_t)0, (int8_t)0, 10, (int8_t)5);
//     printf("Case 10 LB (mem[5]=42):  output = %d  (expected 42)\n", result);
//     printFlags();

//     /* CASE 11 — SB: store 26 to mem[3], read back */
//     result = Alu((int8_t)26, (int8_t)0, 11, (int8_t)3);
//     printf("Case 11 SB (mem[3]=26):  output = %d  (expected 26)\n", result);
//     printFlags();

//     return 0;
// }