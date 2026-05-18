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

int BtoD(int b[], int size)
{
    int dec = 0, c = 0;
    for (int i = size - 1; i >= 0; i--)
    {
        dec = dec + (b[i] * pow(2, c));
        c++;
    }
    return dec;
}

int *DtoB(int decimal, int result[], int size)
{
    for (int i = size - 1; i >= 0; i--)
    {
        result[i] = decimal % 2;
        decimal = decimal / 2;
    }
    return result;
}

int updateCarry(int flag, int8_t a, int8_t b, int opcode)
{
    if (opcode == 0)
    { // ADD
        uint16_t s = (uint16_t)((uint8_t)a) + (uint16_t)((uint8_t)b);
        // flag = s > 0xFF;
        flag = (s >> 8) & 1;
    }

    else
    {
        flag = 0;
    }
    return flag;
}
void clear_flags()
{
    zeroFlag = 0;
    carryFlag = 0;
    overflowFlag = 0;
    negativeFlag = 0;
    signFlag = 0;
}

int updateOverFlow(int flag, int8_t reg1, int8_t reg2, int opcode, int8_t result)
{
    if (opcode == 0)
    {
        int s1 = (reg1 >> 7) & 1;
        int s2 = (reg2 >> 7) & 1;
        int sr = (result >> 7) & 1;
        flag = (s1 == s2 && sr != s1) ? 1 : 0;
    }
    else if (opcode == 1)
    {
        int s1 = (reg1 >> 7) & 1;
        int s2 = (reg2 >> 7) & 1;
        int sr = (result >> 7) & 1;
        flag = (s1 != s2 && sr != s1) ? 1 : 0;
    }
    else
    {
        flag = 0;
    }
    return flag;
}

int updateNegative(int flag, int8_t out)
{
    flag = (out < 0) ? 1 : 0;
    return flag;
}

// A: should be updated whenever either is
int updateSign(int signFlag, int negativeFlag, int overflowFlag)
{
    signFlag = (negativeFlag != overflowFlag) ? 1 : 0;
    return signFlag;
}

int updateZero(int flag, int8_t out)
{
    flag = (out == 0) ? 1 : 0;
    return flag;
}

int8_t Alu(int8_t operandA, int8_t operandB, int opcode, int8_t imm)
{
    int8_t outputD = 0;

    zeroFlag = 0;
    carryFlag = 0;
    overflowFlag = 0;
    negativeFlag = 0;
    signFlag = 0;

    switch (opcode)
    {

    /* 0: ADD — R1 = R1 + R2 */
    case 0:
        outputD = operandA + operandB;
        carryFlag = updateCarry(carryFlag, operandA, operandB, 0);
        overflowFlag = updateOverFlow(overflowFlag, operandA, operandB, 0, outputD);
        negativeFlag = updateNegative(negativeFlag, outputD);
        signFlag = updateSign(signFlag, negativeFlag, overflowFlag);
        zeroFlag = updateZero(zeroFlag, outputD);
        break;

    /* 1: SUB — R1 = R1 - R2  (C not updated per spec; only V, N, S, Z) */
    case 1:
        outputD = operandA - operandB;
        overflowFlag = updateOverFlow(overflowFlag, operandA, operandB, 1, outputD);
        negativeFlag = updateNegative(negativeFlag, outputD);
        signFlag = updateSign(signFlag, negativeFlag, overflowFlag);
        zeroFlag = updateZero(zeroFlag, outputD);
        break;

    /* 2: MUL — R1 = R1 * R2 */
    case 2:
        outputD = operandA * operandB;
        negativeFlag = updateNegative(negativeFlag, outputD);
        zeroFlag = updateZero(zeroFlag, outputD);
        break;

    /* 3: MOVI — R1 = IMM */
    case 3:
        outputD = imm;
        break;

    /* 4: BEQZ — if R1==0: PC = PC+1+IMM */
    case 4:
        // EDIT PC
        if (operandA == 0)
        {
            // pc= pc+1+ imm;
            outputD = (int8_t)(IE.pc + 1 + imm);
        }
        else
        {
            // pc= pc+1;
            outputD = (int8_t)(get_pc());
        }
        // outputD=pc;
        break;

    /* 5: ANDI — R1 = R1 & IMM  (was wrongly R1 & R2) */
    case 5:
        outputD = operandA & imm;
        negativeFlag = updateNegative(negativeFlag, outputD);
        zeroFlag = updateZero(zeroFlag, outputD);
        break;

    /* 6: EOR — R1 = R1 XOR R2  (was wrongly OR) */
    case 6:
        outputD = operandA ^ operandB;
        negativeFlag = updateNegative(negativeFlag, outputD);
        zeroFlag = updateZero(zeroFlag, outputD);
        break;

    /* 7: BR — PC = R1 || R2 (concatenation, caller reads pc) */
    case 7:
        outputD = ((int)(uint8_t)operandA << 8) | (uint8_t)operandB; // A: CHEECK IN EXECUTE()
        // outputD = 0;   /* result too wide for int8_t; caller reads pc */
        // pc      = ((int)(uint8_t)operandA << 8) | (uint8_t)operandB;//A: CHEECK IN EXECUTE()
        outputD = ((int)(uint8_t)operandA << 8) | (uint8_t)operandB;
        break;

    /* 8: SLC — R1 = R1 << IMM | R1 >>> (8-IMM)  circular left shift */
    case 8:
    {
        uint8_t ua = (uint8_t)operandA;
        uint8_t amt = (uint8_t)imm & 7; /* clamp to 0-7 bits */
        outputD = (int8_t)((ua << amt) | (ua >> (8 - amt)));
        negativeFlag = updateNegative(negativeFlag, outputD);
        zeroFlag = updateZero(zeroFlag, outputD);
        break;
    }

    /* 9: SRC — R1 = R1 >>> IMM | R1 << (8-IMM)  circular right shift */
    case 9:
    {
        uint8_t ua = (uint8_t)operandA;
        uint8_t amt = (uint8_t)imm & 7;
        outputD = (int8_t)((ua >> amt) | (ua << (8 - amt)));
        negativeFlag = updateNegative(negativeFlag, outputD);
        zeroFlag = updateZero(zeroFlag, outputD);
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

void printFlags()
{
    printf("Flags -> C:%d  V:%d  N:%d  S:%d  Z:%d\n", carryFlag, overflowFlag, negativeFlag, signFlag, zeroFlag);
}
