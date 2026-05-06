#include <stdio.h>
#include <math.h>

int zeroFlag;
int carryFlag;
int overflowFlag;
int negativeFlag;
int signFlag;
int pc=0;

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


int updateCarry(int flag,int temp[]){
    if(temp[0]==1){
        flag=1;}
    else{
        flag=0;}
    return flag;
}


int updateOverFlow (int flag, int reg1[],int reg2[],int opcode,int temp[]){
    /*checks if it's addition */
    if(opcode==0){
        if(reg1[0]==reg2[0] && temp[1]!=reg1[0]){
            flag=1;}
        else{
            flag=0;}
    }


    /*checks if subtraction*/
    if(opcode==1){
        if(reg1[0]!=reg2[0] && temp[1]==reg2[0]){
            flag=1;}
        else{
            flag=0;}
    }
    return flag;
}


int updateNegative(int flag,int out){
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

int updateZero(int flag,int out){
    if(out==0){
        flag=1;
    }
    else{
        flag=0;}
        return flag;
}


/* CHANGED: return type changed to int to return the decimal output value */
int Alu (int operandA[6],int operandB[6],int operation[4]){
    int outputD=0;
    
     zeroFlag=0;
     carryFlag=0;
     overflowFlag=0;
     negativeFlag=0;
     signFlag=0;

    int operationD= BtoD(operation,4);
    int operandAD= BtoD(operandA,6);
    int operandBD= BtoD(operandB,6);

    int outputB[6];
    int outputTemp[7];


    int c=0;

     switch(operationD){

        case 0:
            outputD= operandAD+operandBD;

            carryFlag= updateCarry(carryFlag,outputTemp);
            overflowFlag= updateOverFlow(overflowFlag,operandA,operandB,operationD,outputTemp);
            negativeFlag= updateNegative(negativeFlag,outputD);
            signFlag= updateSign(signFlag, negativeFlag, overflowFlag);
            zeroFlag= updateZero(zeroFlag,outputD);
            c=0;
            for (int i=1;i<7;i++)
            {
                outputB[c]= outputTemp[i];
                c++;}
            
            break;

        case 1:
            outputD= operandAD-operandBD;


            carryFlag= updateCarry(carryFlag,outputTemp);
            overflowFlag= updateOverFlow(overflowFlag,operandA,operandB,operationD,outputTemp);
            negativeFlag= updateNegative(negativeFlag,outputD);
            signFlag= updateSign(signFlag, negativeFlag, overflowFlag);
            zeroFlag= updateZero(zeroFlag,outputD);

            
            c=0;
            for (int i=1;i<7;i++)
            {
                outputB[c]= outputTemp[i];
                c++;}
            outputD= BtoD(outputB,6);
            break;

        case 2:
            outputD=operandAD*operandBD;

            negativeFlag= updateNegative(negativeFlag,outputD);
            zeroFlag= updateZero(zeroFlag,outputD);
            break;

        case 3:
            operandAD=operandBD;
            outputD=operandAD;
            break;

        case 4:
            if(operandAD==0){
                pc= pc+1+ operandBD;
            }
            else{
                pc= pc+1;
            }
            outputD=pc;
            break;

        case 5:
            outputD=operandAD&operandBD;
           

            negativeFlag= updateNegative(negativeFlag,outputD);
            zeroFlag= updateZero(zeroFlag,outputD);
            break;
         
         case 6:
            outputD= operandAD|operandBD;
            DtoB(outputD,outputB,6);

            negativeFlag= updateNegative(negativeFlag,outputD);
            zeroFlag= updateZero(zeroFlag,outputD);
            break;
            
        case 7:
            pc= (operandAD<<6)|operandBD;
            outputD=pc;
            break;

        case 8:
            outputD=operandAD<<operandBD;

            negativeFlag= updateNegative(negativeFlag,outputD);
            zeroFlag= updateZero(zeroFlag,outputD);
            break;
        
         case 9:
            outputD=operandAD>>operandBD;

            negativeFlag= updateNegative(negativeFlag,outputD);
            zeroFlag= updateZero(zeroFlag,outputD);
            break; 
            
        case 10:
            operandAD= mem[operandBD];
            outputD=operandAD;
            break;

        case 11:
            mem[operandBD]=operandAD;
            outputD=mem[operandBD];
            break;
     }

return outputD;

}

void printFlags(){
    printf("  Flags -> C:%d  V:%d  N:%d  S:%d  Z:%d\n",
           carryFlag, overflowFlag, negativeFlag, signFlag, zeroFlag);
}
/*ask about how to deal with the PC and figure out their expected return type is it always binary or what?*/
 void main(){
    int result;
 
    /* --------------------------------------------------
       CASE 0 — ADD (operation = 0000)
       operandA = 000101 = 5
       operandB = 000011 = 3
       Expected output: 8
       Expected flags: C=0, V=0, N=0, S=0, Z=0
    -------------------------------------------------- */
    int op0[4]  = {0,0,0,0};
    int a0[6]   = {0,0,0,1,0,1};
    int b0[6]   = {0,0,0,0,1,1};
    result = Alu(a0, b0, op0);
    printf("Case 0 ADD (5+3):     output = %d  (expected 8)\n", result);
    printFlags();
 
    /* --------------------------------------------------
       CASE 0 — ADD with carry
       operandA = 111111 = 63
       operandB = 000001 = 1
       Expected output: 64
       Expected flags: C=1 (bit 7 of 7-bit temp is set), V=0, N=0, S=0, Z=0
    -------------------------------------------------- */
    int a0c[6]  = {1,1,1,1,1,1};
    int b0c[6]  = {0,0,0,0,0,1};
    result = Alu(a0c, b0c, op0);
    printf("Case 0 ADD carry (63+1): output = %d  (expected 64)\n", result);
    printFlags();
 
    /* --------------------------------------------------
       CASE 1 — SUB (operation = 0001)
       operandA = 000111 = 7
       operandB = 000010 = 2
       Expected output: 5
       Expected flags: C=0, V=0, N=0, S=0, Z=0
    -------------------------------------------------- */
    int op1[4]  = {0,0,0,1};
    int a1[6]   = {0,0,0,1,1,1};
    int b1[6]   = {0,0,0,0,1,0};
    result = Alu(a1, b1, op1);
    printf("Case 1 SUB (7-2):     output = %d  (expected 5)\n", result);
    printFlags();
 
    /* --------------------------------------------------
       CASE 1 — SUB resulting in zero
       operandA = 000100 = 4
       operandB = 000100 = 4
       Expected output: 0
       Expected flags: C=0, V=0, N=0, S=0, Z=1
    -------------------------------------------------- */
    int a1z[6]  = {0,0,0,1,0,0};
    int b1z[6]  = {0,0,0,1,0,0};
    result = Alu(a1z, b1z, op1);
    printf("Case 1 SUB zero (4-4): output = %d  (expected 0)\n", result);
    printFlags();
 
    /* --------------------------------------------------
       CASE 2 — MUL (operation = 0010)
       operandA = 000011 = 3
       operandB = 000100 = 4
       Expected output: 12
       Expected flags: N=0, Z=0
    -------------------------------------------------- */
    int op2[4]  = {0,0,1,0};
    int a2[6]   = {0,0,0,0,1,1};
    int b2[6]   = {0,0,0,1,0,0};
    result = Alu(a2, b2, op2);
    printf("Case 2 MUL (3*4):     output = %d  (expected 12)\n", result);
    printFlags();
 
    /* --------------------------------------------------
       CASE 3 — LDI (operation = 0011)
       operandA = anything (ignored, gets overwritten by operandB)
       operandB = 010101 = 21
       Expected output: 21
       No flags updated
    -------------------------------------------------- */
    int op3[4]  = {0,0,1,1};
    int a3[6]   = {0,0,0,0,0,0};
    int b3[6]   = {0,1,0,1,0,1};
    result = Alu(a3, b3, op3);
    printf("Case 3 LDI (load 21): output = %d  (expected 21)\n", result);
    printFlags();
 
    /* --------------------------------------------------
       CASE 4 — BEQZ branch taken (operation = 0100)
       operandA = 000000 = 0  (R1 == 0, so branch is taken)
       operandB = 000101 = 5  (IMM = 5)
       pc starts at 0
       Expected output: pc = 0 + 1 + 5 = 6
       No flags updated
    -------------------------------------------------- */
    pc = 0;
    int op4[4]  = {0,1,0,0};
    int a4[6]   = {0,0,0,0,0,0};
    int b4[6]   = {0,0,0,1,0,1};
    result = Alu(a4, b4, op4);
    printf("Case 4 BEQZ taken (pc=0, R1=0, IMM=5): output = %d  (expected 6)\n", result);
    printFlags();
 
    /* --------------------------------------------------
       CASE 4 — BEQZ branch not taken
       operandA = 000001 = 1  (R1 != 0, branch not taken)
       operandB = 000101 = 5  (IMM = 5, ignored)
       pc starts at 6 (continuing from previous test)
       Expected output: pc = 6 + 1 = 7
    -------------------------------------------------- */
    int a4nt[6] = {0,0,0,0,0,1};
    result = Alu(a4nt, b4, op4);
    printf("Case 4 BEQZ not taken (pc=6, R1=1): output = %d  (expected 7)\n", result);
    printFlags();
 
    /* --------------------------------------------------
       CASE 5 — AND (operation = 0101)
       operandA = 001111 = 15  (binary 001111)
       operandB = 000110 =  6  (binary 000110)
       15 & 6  = 000110 & 001111 = 000110 = 6
       Expected output: 6
       Expected flags: N=0, Z=0
    -------------------------------------------------- */
    int op5[4]  = {0,1,0,1};
    int a5[6]   = {0,0,1,1,1,1};
    int b5[6]   = {0,0,0,1,1,0};
    result = Alu(a5, b5, op5);
    printf("Case 5 AND (15 & 6):  output = %d  (expected 6)\n", result);
    printFlags();
 
    /* --------------------------------------------------
       CASE 6 — OR (operation = 0110)
       operandA = 001010 = 10  (binary 001010)
       operandB = 000101 =  5  (binary 000101)
       10 | 5  = 001010 | 000101 = 001111 = 15
       Expected output: 15
       Expected flags: N=0, Z=0
    -------------------------------------------------- */
    int op6[4]  = {0,1,1,0};
    int a6[6]   = {0,0,1,0,1,0};
    int b6[6]   = {0,0,0,1,0,1};
    result = Alu(a6, b6, op6);
    printf("Case 6 OR  (10 | 5):  output = %d  (expected 15)\n", result);
    printFlags();
 
    /* --------------------------------------------------
       CASE 7 — JR (operation = 0111)
       operandA = 000010 = 2
       operandB = 000011 = 3
       Concatenation: 000010 || 000011 = 000010000011 = 131 in decimal
       (2 << 6) | 3 = 128 + 3 = 131
       Expected output: 131 (also stored in pc)
    -------------------------------------------------- */
    int op7[4]  = {0,1,1,1};
    int a7[6]   = {0,0,0,0,1,0};
    int b7[6]   = {0,0,0,0,1,1};
    result = Alu(a7, b7, op7);
    printf("Case 7 JR  (2||3):    output = %d  (expected 131)\n", result);
    printFlags();
 
    /* --------------------------------------------------
       CASE 8 — SAL (operation = 1000)
       operandA = 000011 = 3
       operandB = 000010 = 2  (shift amount)
       3 << 2 = 12
       Expected output: 12
       Expected flags: N=0, Z=0
    -------------------------------------------------- */
    int op8[4]  = {1,0,0,0};
    int a8[6]   = {0,0,0,0,1,1};
    int b8[6]   = {0,0,0,0,1,0};
    result = Alu(a8, b8, op8);
    printf("Case 8 SAL (3<<2):    output = %d  (expected 12)\n", result);
    printFlags();
 
    /* --------------------------------------------------
       CASE 9 — SAR (operation = 1001)
       operandA = 001100 = 12
       operandB = 000010 = 2  (shift amount)
       12 >> 2 = 3
       Expected output: 3
       Expected flags: N=0, Z=0
    -------------------------------------------------- */
    int op9[4]  = {1,0,0,1};
    int a9[6]   = {0,0,1,1,0,0};
    int b9[6]   = {0,0,0,0,1,0};
    result = Alu(a9, b9, op9);
    printf("Case 9 SAR (12>>2):   output = %d  (expected 3)\n", result);
    printFlags();
 
    /* --------------------------------------------------
       CASE 10 — LB (operation = 1010)
       mem[5] is pre-loaded with value 42
       operandA = anything (destination, not used here)
       operandB = 000101 = 5  (address)
       Expected output: 42
       No flags updated
    -------------------------------------------------- */
    mem[5] = 42;
    int op10[4] = {1,0,1,0};
    int a10[6]  = {0,0,0,0,0,0};
    int b10[6]  = {0,0,0,1,0,1};
    result = Alu(a10, b10, op10);
    printf("Case 10 LB (mem[5]=42): output = %d  (expected 42)\n", result);
    printFlags();
 
    /* --------------------------------------------------
       CASE 11 — SB (operation = 1011)
       operandA = 011010 = 26  (value to store)
       operandB = 000011 = 3   (address)
       Stores 26 into mem[3], returns mem[3]
       Expected output: 26
       No flags updated
    -------------------------------------------------- */
    int op11[4] = {1,0,1,1};
    int a11[6]  = {0,1,1,0,1,0};
    int b11[6]  = {0,0,0,0,1,1};
    result = Alu(a11, b11, op11);
    printf("Case 11 SB (mem[3]=26): output = %d  (expected 26)\n", result);
    printFlags();
 
    return;
  }