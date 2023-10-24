#include <stdio.h>
#include "include/machine_state.h"

void setup_dest_addressing(machine_state_t *machine)
{
    // From immediate.
    if ((machine->IR & 077) == 027) {
        machine->PC++;
        machine->MBR = machine->PC;
        machine->DEST = &(machine->MEMORY[machine->MBR]);

    // From absolute.
    } else if ((machine->IR & 077) == 037) {
        machine->PC++;
        machine->MBR = machine->PC;
        machine->MAR = machine->MEMORY[machine->MBR];
        machine->DEST = &(machine->MEMORY[machine->MAR]);

    // From register.
    } else if ((machine->IR & 070) == 00) {
        machine->DEST = &(machine->R[(machine->IR & 007)]);
    }
}

void setup_src_addressing(machine_state_t *machine)
{
    // From immediate address.
    if ((machine->IR & 07700) == 02700) {
        machine->PC++;
        machine->MBR = machine->PC;
        machine->SRC = &(machine->MEMORY[machine->MBR]);

    // From absolute address.
    } else if ((machine->IR & 03700) == 03700) {
        machine->PC++;
        machine->MBR = machine->PC;
        machine->MAR = machine->MEMORY[machine->MBR];
        machine->SRC = &(machine->MEMORY[machine->MAR]);

    // From register.
    } else if ((machine->IR & 07000) == 0) {
        machine->SRC = &(machine->R[(machine->IR & 00700) >> 6]);
    }
}

void set_zero_flag(machine_state_t *machine)
{
    if (machine->ALU == 0) {
        machine->PSW |= ZEROFLAG;
    } else {
        machine->PSW = (machine->PSW & ZEROFLAG) ? (machine->PSW ^ ZEROFLAG) : machine->PSW;
    }
}

int8_t get_branch_offset(machine_state_t *machine)
{
    return (int8_t) (machine->IR & 0377);
}

void NOP(machine_state_t *machine)
{
    machine->PC++;
}

void BR(machine_state_t *machine)
{
    int8_t OFFSET = get_branch_offset(machine);
    machine->PC += (int8_t) (2 * OFFSET);
}

void BNE(machine_state_t *machine)
{
    int8_t OFFSET = get_branch_offset(machine);
    machine->ALU = machine->PSW & ZEROFLAG;
    if (machine->ALU == 0) machine->PC += (2 * OFFSET);
}

void BEQ(machine_state_t *machine)
{
    int8_t OFFSET = get_branch_offset(machine);
    machine->ALU = machine->PSW & ZEROFLAG;
    if (machine->ALU) machine->PC += 2 * OFFSET;
}

void BGE(machine_state_t *machine)
{
    int8_t OFFSET = get_branch_offset(machine);
    machine->ALU = (machine->PSW & NEGATIVEFLAG) >> 3;
    machine->ALU ^= (machine->PSW & OVERFLOWFLAG) >> 1;
    if (machine->ALU == 0) {
        machine->PC += 2 * OFFSET;
    }
}

void BLT(machine_state_t *machine)
{
    int8_t OFFSET = get_branch_offset(machine);
    machine->ALU = (machine->PSW & NEGATIVEFLAG) >> 3;
    machine->ALU ^= (machine->PSW & OVERFLOWFLAG) >> 1;
    if (machine->ALU) {
        machine->PC += 2 * OFFSET;
    }
}

void BGT(machine_state_t *machine)
{
    int8_t OFFSET = get_branch_offset(machine);
    machine->ALU = (machine->PSW & ZEROFLAG) >> 2;
    machine->ALU |= ((machine->PSW & NEGATIVEFLAG) >> 3) ^ ((machine->PSW & OVERFLOWFLAG) >> 1); 
    if (machine->ALU == 0) {
        machine->PC += 2 * OFFSET;
    }
}

void BLE(machine_state_t *machine)
{
    int8_t OFFSET = get_branch_offset(machine);
    machine->ALU = (machine->PSW & ZEROFLAG) >> 2;
    machine->ALU |= ((machine->PSW & NEGATIVEFLAG) >> 3) ^ ((machine->PSW & OVERFLOWFLAG) >> 1); 
    if (machine->ALU) {
        machine->PC += 2 * OFFSET;
    }
}

void BPL(machine_state_t *machine)
{
    int8_t OFFSET = get_branch_offset(machine);
    if ((machine->PSW & NEGATIVEFLAG) == 0) {
        machine->PC += 2 * OFFSET;
    }
}

void BMI(machine_state_t *machine)
{
    int8_t OFFSET = get_branch_offset(machine);
    if (machine->PSW & NEGATIVEFLAG) {
        machine->PC += 2 * OFFSET;
    }
}

void BHI(machine_state_t *machine)
{
    int8_t OFFSET = get_branch_offset(machine);
    machine->ALU = machine->PSW & CARRYFLAG;
    machine->ALU |= (machine->PSW & ZEROFLAG) >> 2;
    if (machine->ALU == 0) {
        machine->PC += 2 * OFFSET;
    }
}

void BLOS(machine_state_t *machine)
{
    int8_t OFFSET = get_branch_offset(machine);
    machine->ALU = machine->PSW & CARRYFLAG;
    machine->ALU |= (machine->PSW & ZEROFLAG) >> 2;
    if (machine->ALU) {
        machine->PC += 2 * OFFSET;
    }
}

void BVC(machine_state_t *machine)
{
    int8_t OFFSET = get_branch_offset(machine);
    machine->ALU = machine->PSW & OVERFLOWFLAG;
    if (machine->ALU == 0) {
        machine->PC += 2 * OFFSET;
    }
}

void BVS(machine_state_t *machine)
{
    int8_t OFFSET = get_branch_offset(machine);
    machine->ALU = machine->PSW & OVERFLOWFLAG;
    if (machine->ALU) {
        machine->PC += 2 * OFFSET;
    }
}

void BCC(machine_state_t *machine)
{
    int8_t OFFSET = get_branch_offset(machine);
    machine->ALU = machine->PSW & CARRYFLAG;
    if (machine->ALU == 0) {
        machine->PC += 2 * OFFSET;
    }
}

void BCS(machine_state_t *machine)
{
    int8_t OFFSET = get_branch_offset(machine);
    machine->ALU = machine->PSW & CARRYFLAG;
    if (machine->ALU) {
        machine->PC += 2 * OFFSET;
    }
}

void MOV(machine_state_t *machine)
{
    setup_src_addressing(machine);
    setup_dest_addressing(machine);

    *machine->DEST = *machine->SRC;
    machine->PC++;
}

void MOVB(machine_state_t *machine)
{
    setup_src_addressing(machine);
    setup_dest_addressing(machine);

    *machine->DEST = (*machine->SRC & 0377);
    machine->PC++;
}

void CMP(machine_state_t *machine)
{
    setup_src_addressing(machine);
    setup_dest_addressing(machine);

    machine->ALU = *machine->DEST;
    machine->ALU -= *machine->SRC;
    machine->PC++;
    set_zero_flag(machine);
}

void CMPB(machine_state_t *machine)
{
    NOP(machine);
}

void BIT(machine_state_t *machine)
{
    setup_src_addressing(machine);
    setup_dest_addressing(machine);

    machine->ALU = *machine->DEST;
    machine->ALU &= *machine->SRC;
    *machine->DEST = machine->ALU;
    set_zero_flag(machine);
    machine->PC++;
}

void BITB(machine_state_t *machine)
{
    NOP(machine);
}

void BIC(machine_state_t *machine)
{
    setup_src_addressing(machine);
    setup_dest_addressing(machine);

    machine->ALU = *machine->DEST;
    machine->ALU &= ~(*machine->SRC);
    *machine->DEST = machine->ALU;
    machine->PC++;
}

void BICB(machine_state_t *machine)
{
    NOP(machine);
}

void BIS(machine_state_t *machine)
{
    setup_src_addressing(machine);
    setup_dest_addressing(machine);

    machine->ALU = *machine->DEST;
    machine->ALU |= *machine->SRC;
    *machine->DEST = machine->ALU;
    machine->PC++;
}

void BISB(machine_state_t *machine)
{
    NOP(machine);
}

void ADD(machine_state_t *machine)
{
    setup_src_addressing(machine);
    setup_dest_addressing(machine);

    machine->ALU = *machine->DEST;
    machine->ALU += *machine->SRC;
    *machine->DEST = machine->ALU;
    machine->PC++;
}

void SUB(machine_state_t *machine)
{
    setup_src_addressing(machine);
    setup_dest_addressing(machine);

    machine->ALU = *machine->DEST;
    machine->ALU -= *machine->SRC;
    *machine->DEST = machine->ALU;
    machine->PC++;
    set_zero_flag(machine);
}

void JMP(machine_state_t *machine)
{
    setup_dest_addressing(machine);
    machine->PC = *machine->DEST;
}

void SWAB(machine_state_t *machine)
{
    setup_dest_addressing(machine);
    machine->MBR = machine->MAR & 077;
    *machine->DEST = ((*machine->DEST & 0177400) >> 8) | ((*machine->DEST & 000377) << 8);
    machine->PC++;
}

void CLR(machine_state_t *machine)
{
    setup_dest_addressing(machine);
    *machine->DEST = 0;
    machine->PC++;
}

void CLRB(machine_state_t *machine)
{
    setup_dest_addressing(machine);
    machine->MBR = machine->MAR & 077;
    *machine->DEST = (*machine->DEST & 0177400);
    machine->PC++;
}

void COM(machine_state_t *machine)
{
    setup_dest_addressing(machine);
    machine->MBR = machine->MAR & 077;
    *machine->DEST = ~(*machine->DEST);
    machine->PC++;
}

void COMB(machine_state_t *machine)
{
    setup_dest_addressing(machine);
    machine->MBR = machine->MAR & 077;
    *machine->DEST = ~(*machine->DEST & 000377) | (*machine->DEST & 0177400);
    machine->PC++;
}

void INC(machine_state_t *machine)
{
    setup_dest_addressing(machine);
    (*machine->DEST)++;
    machine->PC++;
}

void INCB(machine_state_t *machine)
{
    setup_dest_addressing(machine);
    machine->MBR = machine->MAR & 077;
    *machine->DEST = ((*machine->DEST & 000377) + 1) | (*machine->DEST & 0177400);
    machine->PC++;
}

void DEC(machine_state_t *machine)
{
    setup_dest_addressing(machine);
    (*machine->DEST)--;
    machine->PC++;
    set_zero_flag(machine);
}

void DECB(machine_state_t *machine)
{
    setup_dest_addressing(machine);
    machine->MBR = machine->MAR & 077;
    *machine->DEST = ((*machine->DEST & 000377) - 1) | (*machine->DEST & 0177400);
    machine->PC++;
}

void NEG(machine_state_t *machine)
{
    setup_dest_addressing(machine);
    machine->MBR = machine->MAR & 077;
    *machine->DEST = ~(*machine->DEST) + 1;
    machine->PC++;
}

void NEGB(machine_state_t *machine)
{
    setup_dest_addressing(machine);
    machine->MBR = machine->MAR & 077;
    *machine->DEST = ((~(*machine->DEST & 000377) + 1) & 000377) | (*machine->DEST & 0177400);
    machine->PC++;
}

void ADC(machine_state_t *machine)
{
    setup_dest_addressing(machine);
    machine->MBR = machine->MAR & 077;
    *machine->DEST = *machine->DEST + (machine->PSW & CARRYFLAG);
    machine->PC++;
}

void ADCB(machine_state_t *machine)
{
    setup_dest_addressing(machine);
    machine->MBR = machine->MAR & 077;
    *machine->DEST = ((*machine->DEST & 000377) + (machine->PSW & CARRYFLAG)) | (*machine->DEST & 0177400);
    machine->PC++;
}

void SBC(machine_state_t *machine)
{
    setup_dest_addressing(machine);
    machine->MBR = machine->MAR & 077;
    *machine->DEST = *machine->DEST - (machine->PSW & CARRYFLAG);
    machine->PC++;
}

void SBCB(machine_state_t *machine)
{
    setup_dest_addressing(machine);
    machine->MBR = machine->MAR & 077;
    *machine->DEST = ((*machine->DEST & 000377) - (machine->PSW & CARRYFLAG)) | (*machine->DEST & 0177400);
    machine->PC++;
}

void TST(machine_state_t *machine)
{
    NOP(machine);
}

void TSTB(machine_state_t *machine)
{
    NOP(machine);
}

void ROR(machine_state_t *machine)
{
    setup_dest_addressing(machine);
    machine->MBR = machine->MAR & 077;
    *machine->DEST = (*machine->DEST << 1) | (*machine->DEST >> 15);
    machine->PC++;
}

void RORB(machine_state_t *machine)
{
    setup_dest_addressing(machine);
    machine->MBR = machine->MAR & 077;
    *machine->DEST = (((*machine->DEST & 000377) << 1) | ((*machine->DEST & 000377) >> 7)) | (*machine->DEST & 0177400);
    machine->PC++;
}

void ROL(machine_state_t *machine)
{
    setup_dest_addressing(machine);
    machine->MBR = machine->MAR & 077;
    *machine->DEST = (*machine->DEST << 15) | (*machine->DEST >> 1);
    machine->PC++;
}

void ROLB(machine_state_t *machine)
{
    setup_dest_addressing(machine);
    machine->MBR = machine->MAR & 077;
    *machine->DEST = (((*machine->DEST & 000377) << 7) | ((*machine->DEST & 000377) >> 1)) | (*machine->DEST & 0177400);
    machine->PC++;
}

void ASR(machine_state_t *machine)
{
    setup_dest_addressing(machine);
    machine->MBR = machine->MAR & 077;
    machine->ALU = *machine->DEST;
    machine->ALU >>= 1;
    *machine->DEST = machine->ALU;
    machine->PC++;
}

void ASRB(machine_state_t *machine)
{
    setup_dest_addressing(machine);
    machine->MBR = machine->MAR & 077;
    *machine->DEST = ((*machine->DEST & 000377) >> 1) | (*machine->DEST & 0177400);
    machine->PC++;
}

void ASL(machine_state_t *machine)
{
    setup_dest_addressing(machine);
    machine->MBR = machine->MAR & 077;
    machine->ALU = *machine->DEST;
    machine->ALU <<= 1;
    *machine->DEST = machine->ALU;
    machine->PC++;
}

void ASLB(machine_state_t *machine)
{
    setup_dest_addressing(machine);
    machine->MBR = machine->MAR & 077;
    *machine->DEST = ((*machine->DEST & 000377) << 1) | (*machine->DEST & 0177400);
    machine->PC++;
}

void MTPS(machine_state_t *machine)
{
    setup_dest_addressing(machine);
    machine->MBR = machine->MAR & 077;
    machine->PSW = *machine->DEST;
    machine->PC++;
}

void MFPI(machine_state_t *machine)
{
    NOP(machine);
}

void MFPD(machine_state_t *machine)
{
    NOP(machine);
}

void MTPI(machine_state_t *machine)
{
    NOP(machine);
}

void MTPD(machine_state_t *machine)
{
    NOP(machine);
}

void SXT(machine_state_t *machine)
{
    setup_dest_addressing(machine);
    if (machine->PSW & NEGATIVEFLAG) {
        *machine->DEST = -1;
    } else {
        *machine->DEST = 0;
    }
    machine->PC++;
}

void MFPS(machine_state_t *machine)
{
    setup_dest_addressing(machine);
    machine->MBR = machine->MAR & 077;
    *machine->DEST = machine->PSW;
    machine->PC++;
}

void MUL(machine_state_t *machine)
{
    uint16_t *REG = &(machine->R[(machine->IR & 0700) >> 6]);
    setup_dest_addressing(machine);

    machine->ALU = *machine->DEST;
    machine->ALU *= *REG;
    *REG = machine->ALU;
    machine->PC++;
}

void DIV(machine_state_t *machine)
{
    NOP(machine);
}

void ASH(machine_state_t *machine)
{
    NOP(machine);
}

void ASHC(machine_state_t *machine)
{
    NOP(machine);
}

void XOR(machine_state_t *machine)
{
    uint16_t *REG = &(machine->R[(machine->IR & 0700) >> 6]);
    setup_dest_addressing(machine);

    machine->ALU = *REG;
    machine->ALU ^= *machine->DEST;
    *REG = machine->ALU;
    machine->PC++;
}
