#include <stdio.h>

#include "include/machine_state.h"
#include "include/opcodes.h"

// Destination Addressing

void setup_pc_dest_addressing(machine_state_t *machine)
{
    uint8_t mode = (machine->IR & 070) >> 3;
    switch (mode) {
        // From register.
        case 0:
            machine->DEST = &(machine->R[(machine->IR & 007)]);
            break;

        // From immediate.
        case 2:
            machine->PC += 2;
            machine->MBR = machine->PC;
            machine->DEST = &(machine->MEMORY[machine->MBR]);
            break;

        // From absolute.
        case 3:
            machine->PC += 2;
            machine->MBR = machine->PC;
            machine->MAR = machine->MEMORY[machine->MBR];
            machine->DEST = &(machine->MEMORY[machine->MAR]);
            break;

    }
}

void setup_sp_dest_addressing(machine_state_t *machine)
{
    return;
}

void setup_general_dest_addressing(machine_state_t *machine)
{
    uint8_t mode = (machine->IR & 070) >> 3;
    uint8_t reg = machine->IR & 007;
    switch (mode) {
        // Register.
        case 0:
            machine->DEST = &(machine->R[reg]);
            break;

        // Deferred.
        case 1:
            machine->MAR = machine->R[reg];
            machine->DEST = &(machine->MEMORY[machine->MAR]);
            break;

        // Autoincrement.
        //
        // NOTE: (incremented by one or two) to address the next word or
        // byte depending upon whether the instruction **operates on byte or word data**
        case 2:
            machine->MAR = machine->R[reg];
            machine->R[reg] += 2; // XXX word advance.
            machine->DEST = &(machine->MEMORY[machine->MAR]);
            break;

        // Deferred autoincrement.
        case 3:
            uint16_t deferred = machine->R[reg];
            machine->MAR = machine->MEMORY[deferred];
            machine->MEMORY[deferred] += 2;
            machine->DEST = &(machine->MEMORY[machine->MAR]);
            break;
        
        // Autodecrement.
        //
        // NOTE: (decremented by one or two) to address the next word or
        // byte depending upon whether the instruction **operates on byte or word data**
        case 4:
            machine->MAR = machine->R[reg];
            machine->R[reg] -= 2; // XXX word advance.
            machine->DEST = &(machine->MEMORY[machine->MAR]);
            break;

        // Deferred autodecrement.
        case 5:
            uint16_t deferred = machine->R[reg];
            machine->MAR = machine->MEMORY[deferred];
            machine->MEMORY[deferred] -= 2;
            machine->DEST = &(machine->MEMORY[machine->MAR]);
            break;

        // Indexed.
        case 6:
            machine->PC += 2;
            machine->MAR = machine->PC;
            uint16_t index = machine->MEMORY[machine->MAR] | (machine->MEMORY[machine->MAR + 1] << 8);
            machine->DEST = &(machine->MEMORY[machine->R[reg] + index]);
            break;

        // Deferred indexed.
        case 7:
            machine->PC += 2;
            machine->MAR = machine->PC;
            uint16_t index = machine->MEMORY[machine->MAR] | (machine->MEMORY[machine->MAR + 1] << 8);
            machine->MAR = machine->R[reg];
            machine->DEST = &(machine->MEMORY[machine->MEMORY[machine->MAR] + index]);
            break;
    }
}

void setup_dest_addressing(machine_state_t *machine)
{
    uint8_t reg = machine->IR & 007;
    switch (reg) {
        case 7:
            setup_pc_dest_addressing(machine);
            break;
        case 6:
            setup_sp_dest_addressing(machine);
            break;
        default:
            setup_general_dest_addressing(machine);
    }
}


void setup_pc_dest_byte_addressing(machine_state_t *machine)
{
    uint8_t mode = (machine->IR & 070) >> 3;
    switch (mode) {
        // From register.
        case 0:
            machine->DESTB = &(machine->R[(machine->IR & 007)]);
            break;

        // From immediate.
        case 2:
            machine->PC += 2;
            machine->MBR = machine->PC;
            machine->DESTB = &(machine->MEMORY[machine->MBR]);
            break;

        // From absolute.
        case 3:
            machine->PC += 2;
            machine->MBR = machine->PC;
            machine->MAR = machine->MEMORY[machine->MBR];
            machine->DESTB = &(machine->MEMORY[machine->MAR]);
            break;

    }
}

void setup_sp_dest_byte_addressing(machine_state_t *machine)
{
    return;
}

void setup_general_dest_byte_addressing(machine_state_t *machine)
{
    uint8_t mode = (machine->IR & 070) >> 3;
    uint8_t reg = machine->IR & 007;
    switch (mode) {
        // Register.
        case 0:
            machine->DESTB = &(machine->R[reg]);
            break;

        // Deferred.
        case 1:
            machine->MAR = machine->R[reg];
            machine->DESTB = &(machine->MEMORY[machine->MAR]);
            break;

        // Autoincrement.
        //
        // NOTE: (incremented by one or two) to address the next word or
        // byte depending upon whether the instruction **operates on byte or word data**
        case 2:
            machine->MAR = machine->R[reg];
            machine->R[reg]++; // XXX byte advance.
            machine->DESTB = &(machine->MEMORY[machine->MAR]);
            break;

        // Deferred autoincrement.
        case 3:
            uint16_t deferred = machine->R[reg];
            machine->MAR = machine->MEMORY[deferred];
            machine->MEMORY[deferred]++;
            machine->DESTB = &(machine->MEMORY[machine->MAR]);
            break;
        
        // Autodecrement.
        //
        // NOTE: (decremented by one or two) to address the next word or
        // byte depending upon whether the instruction **operates on byte or word data**
        case 4:
            machine->MAR = machine->R[reg];
            machine->R[reg]--; // XXX byte advance.
            machine->DESTB = &(machine->MEMORY[machine->MAR]);
            break;

        // Deferred autodecrement.
        case 5:
            uint16_t deferred = machine->R[reg];
            machine->MAR = machine->MEMORY[deferred];
            machine->MEMORY[deferred]--;
            machine->DESTB = &(machine->MEMORY[machine->MAR]);
            break;

        // Indexed.
        case 6:
            machine->PC += 2;
            machine->MAR = machine->PC;
            uint16_t index = machine->MEMORY[machine->MAR] | (machine->MEMORY[machine->MAR + 1] << 8);
            machine->DESTB = &(machine->MEMORY[machine->R[reg] + index]);
            break;

        // Deferred indexed.
        case 7:
            machine->PC += 2;
            machine->MAR = machine->PC;
            uint16_t index = machine->MEMORY[machine->MAR] | (machine->MEMORY[machine->MAR + 1] << 8);
            machine->MAR = machine->R[reg];
            machine->DESTB = &(machine->MEMORY[machine->MEMORY[machine->MAR] + index]);
            break;
    }
}

void setup_dest_byte_addressing(machine_state_t *machine)
{
    uint8_t reg = machine->IR & 007;
    switch (reg) {
        case 7:
            setup_pc_dest_byte_addressing(machine);
            break;
        case 6:
            setup_sp_dest_byte_addressing(machine);
            break;
        default:
            setup_general_dest_byte_addressing(machine);
    }
}


// Source Addressing

void setup_pc_src_addressing(machine_state_t *machine)
{
    uint8_t mode = (machine->IR & 07000) >> 9;
    switch (mode) {
        // From register.
        case 0:
            machine->SRC = &(machine->R[(machine->IR & 007)]);
            break;

        // From immediate.
        case 2:
            machine->PC += 2;
            machine->MBR = machine->PC;
            machine->SRC = &(machine->MEMORY[machine->MBR]);
            break;

        // From absolute.
        case 3:
            machine->PC += 2;
            machine->MBR = machine->PC;
            machine->MAR = machine->MEMORY[machine->MBR];
            machine->SRC = &(machine->MEMORY[machine->MAR]);
            break;

    }
}

void setup_sp_src_addressing(machine_state_t *machine)
{
    return;
}

void setup_general_src_addressing(machine_state_t *machine)
{
    uint8_t mode = (machine->IR & 07000) >> 9;
    uint8_t reg = (machine->IR & 00700) >> 6;
    switch (mode) {
        // Register.
        case 0:
            machine->SRC = &(machine->R[reg]);
            break;

        // Deferred.
        case 1:
            machine->MAR = machine->R[reg];
            machine->SRC = &(machine->MEMORY[machine->MAR]);
            break;

        // Autoincrement.
        //
        // NOTE: (incremented by one or two) to address the next word or
        // byte depending upon whether the instruction **operates on byte or word data**
        case 2:
            machine->MAR = machine->R[reg];
            machine->R[reg] += 2; // XXX word advance.
            machine->SRC = &(machine->MEMORY[machine->MAR]);
            break;

        // Deferred autoincrement.
        case 3:
            uint16_t deferred = machine->R[reg];
            machine->MAR = machine->MEMORY[deferred];
            machine->MEMORY[deferred] += 2;
            machine->SRC = &(machine->MEMORY[machine->MAR]);
            break;
        
        // Autodecrement.
        //
        // NOTE: (decremented by one or two) to address the next word or
        // byte depending upon whether the instruction **operates on byte or word data**
        case 4:
            machine->MAR = machine->R[reg];
            machine->R[reg] -= 2; // XXX word advance.
            machine->SRC = &(machine->MEMORY[machine->MAR]);
            break;

        // Deferred autodecrement.
        case 5:
            uint16_t deferred = machine->R[reg];
            machine->MAR = machine->MEMORY[deferred];
            machine->MEMORY[deferred] -= 2;
            machine->SRC = &(machine->MEMORY[machine->MAR]);
            break;

        // Indexed.
        case 6:
            machine->PC += 2;
            machine->MAR = machine->PC;
            uint16_t index = machine->MEMORY[machine->MAR] | (machine->MEMORY[machine->MAR + 1] << 8);
            machine->SRC = &(machine->MEMORY[machine->R[reg] + index]);
            break;

        // Deferred indexed.
        case 7:
            machine->PC += 2;
            machine->MAR = machine->PC;
            uint16_t index = machine->MEMORY[machine->MAR] | (machine->MEMORY[machine->MAR + 1] << 8);
            machine->MAR = machine->R[reg];
            machine->SRC = &(machine->MEMORY[machine->MEMORY[machine->MAR] + index]);
            break;
    }
}

void setup_src_addressing(machine_state_t *machine)
{
    uint8_t reg = (machine->IR & 00700) >> 6;
    switch (reg) {
        case 7:
            setup_pc_src_addressing(machine);
            break;
        case 6:
            setup_sp_src_addressing(machine);
            break;
        default:
            setup_general_src_addressing(machine);
    }
}



void setup_pc_src_byte_addressing(machine_state_t *machine)
{
    uint8_t mode = (machine->IR & 07000) >> 9;
    switch (mode) {
        // From register.
        case 0:
            machine->SRCB = &(machine->R[(machine->IR & 007)]);
            break;

        // From immediate.
        case 2:
            machine->PC += 2;
            machine->MBR = machine->PC;
            machine->SRCB = &(machine->MEMORY[machine->MBR]);
            break;

        // From absolute.
        case 3:
            machine->PC += 2;
            machine->MBR = machine->PC;
            machine->MAR = machine->MEMORY[machine->MBR];
            machine->SRCB = &(machine->MEMORY[machine->MAR]);
            break;

    }
}

void setup_sp_src_byte_addressing(machine_state_t *machine)
{
    return;
}

void setup_general_src_byte_addressing(machine_state_t *machine)
{
    uint8_t mode = (machine->IR & 07000) >> 9;
    uint8_t reg = (machine->IR & 00700) >> 6;
    switch (mode) {
        // Register.
        case 0:
            machine->SRCB = &(machine->R[reg]);
            break;

        // Deferred.
        case 1:
            machine->MAR = machine->R[reg];
            machine->SRCB = &(machine->MEMORY[machine->MAR]);
            break;

        // Autoincrement.
        //
        // NOTE: (incremented by one or two) to address the next word or
        // byte depending upon whether the instruction **operates on byte or word data**
        case 2:
            machine->MAR = machine->R[reg];
            machine->R[reg]++; // XXX byte advance.
            machine->SRCB = &(machine->MEMORY[machine->MAR]);
            break;

        // Deferred autoincrement.
        case 3:
            uint16_t deferred = machine->R[reg];
            machine->MAR = machine->MEMORY[deferred];
            machine->MEMORY[deferred]++;
            machine->SRCB = &(machine->MEMORY[machine->MAR]);
            break;
        
        // Autodecrement.
        //
        // NOTE: (decremented by one or two) to address the next word or
        // byte depending upon whether the instruction **operates on byte or word data**
        case 4:
            machine->MAR = machine->R[reg];
            machine->R[reg]--; // XXX byte advance.
            machine->SRCB = &(machine->MEMORY[machine->MAR]);
            break;

        // Deferred autodecrement.
        case 5:
            uint16_t deferred = machine->R[reg];
            machine->MAR = machine->MEMORY[deferred];
            machine->MEMORY[deferred]--;
            machine->SRCB = &(machine->MEMORY[machine->MAR]);
            break;

        // Indexed.
        case 6:
            machine->PC += 2;
            machine->MAR = machine->PC;
            uint16_t index = machine->MEMORY[machine->MAR] | (machine->MEMORY[machine->MAR + 1] << 8);
            machine->SRCB = &(machine->MEMORY[machine->R[reg] + index]);
            break;

        // Deferred indexed.
        case 7:
            machine->PC += 2;
            machine->MAR = machine->PC;
            uint16_t index = machine->MEMORY[machine->MAR] | (machine->MEMORY[machine->MAR + 1] << 8);
            machine->MAR = machine->R[reg];
            machine->SRCB = &(machine->MEMORY[machine->MEMORY[machine->MAR] + index]);
            break;
    }
}

void setup_src_byte_addressing(machine_state_t *machine)
{
    uint8_t reg = (machine->IR & 00700) >> 6;
    switch (reg) {
        case 7:
            setup_pc_src_byte_addressing(machine);
            break;
        case 6:
            setup_sp_src_byte_addressing(machine);
            break;
        default:
            setup_general_src_byte_addressing(machine);
    }
}

void set_zero_flag(machine_state_t *machine, uint16_t value)
{
    if (value == 0) {
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
    machine->PC += 2;
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
    if (machine->ALU == 0)
        machine->PC += (2 * OFFSET);
    else
        machine->PC += 2;
}

void BEQ(machine_state_t *machine)
{
    int8_t OFFSET = get_branch_offset(machine);
    machine->ALU = machine->PSW & ZEROFLAG;
    if (machine->ALU)
        machine->PC += 2 * OFFSET;
    else
        machine->PC += 2;
}

void BGE(machine_state_t *machine)
{
    int8_t OFFSET = get_branch_offset(machine);
    machine->ALU = (machine->PSW & NEGATIVEFLAG) >> 3;
    machine->ALU ^= (machine->PSW & OVERFLOWFLAG) >> 1;
    if (machine->ALU == 0)
        machine->PC += 2 * OFFSET;
    else
        machine->PC += 2;
}

void BLT(machine_state_t *machine)
{
    int8_t OFFSET = get_branch_offset(machine);
    machine->ALU = (machine->PSW & NEGATIVEFLAG) >> 3;
    machine->ALU ^= (machine->PSW & OVERFLOWFLAG) >> 1;
    if (machine->ALU)
        machine->PC += 2 * OFFSET;
    else
        machine->PC += 2;
}

void BGT(machine_state_t *machine)
{
    int8_t OFFSET = get_branch_offset(machine);
    machine->ALU = (machine->PSW & ZEROFLAG) >> 2;
    machine->ALU |= ((machine->PSW & NEGATIVEFLAG) >> 3) ^ ((machine->PSW & OVERFLOWFLAG) >> 1); 
    if (machine->ALU == 0)
        machine->PC += 2 * OFFSET;
    else
        machine->PC += 2;
}

void BLE(machine_state_t *machine)
{
    int8_t OFFSET = get_branch_offset(machine);
    machine->ALU = (machine->PSW & ZEROFLAG) >> 2;
    machine->ALU |= ((machine->PSW & NEGATIVEFLAG) >> 3) ^ ((machine->PSW & OVERFLOWFLAG) >> 1); 
    if (machine->ALU)
        machine->PC += 2 * OFFSET;
    else
        machine->PC += 2;
}

void BPL(machine_state_t *machine)
{
    int8_t OFFSET = get_branch_offset(machine);
    if ((machine->PSW & NEGATIVEFLAG) == 0)
        machine->PC += 2 * OFFSET;
    else
        machine->PC += 2;
}

void BMI(machine_state_t *machine)
{
    int8_t OFFSET = get_branch_offset(machine);
    if (machine->PSW & NEGATIVEFLAG)
        machine->PC += 2 * OFFSET;
    else
        machine->PC += 2;
}

void BHI(machine_state_t *machine)
{
    int8_t OFFSET = get_branch_offset(machine);
    machine->ALU = machine->PSW & CARRYFLAG;
    machine->ALU |= (machine->PSW & ZEROFLAG) >> 2;
    if (machine->ALU == 0)
        machine->PC += 2 * OFFSET;
    else
        machine->PC += 2;
}

void BLOS(machine_state_t *machine)
{
    int8_t OFFSET = get_branch_offset(machine);
    machine->ALU = machine->PSW & CARRYFLAG;
    machine->ALU |= (machine->PSW & ZEROFLAG) >> 2;
    if (machine->ALU)
        machine->PC += 2 * OFFSET;
    else
        machine->PC += 2;
}

void BVC(machine_state_t *machine)
{
    int8_t OFFSET = get_branch_offset(machine);
    machine->ALU = machine->PSW & OVERFLOWFLAG;
    if (machine->ALU == 0)
        machine->PC += 2 * OFFSET;
    else
        machine->PC += 2;
}

void BVS(machine_state_t *machine)
{
    int8_t OFFSET = get_branch_offset(machine);
    machine->ALU = machine->PSW & OVERFLOWFLAG;
    if (machine->ALU)
        machine->PC += 2 * OFFSET;
    else
        machine->PC += 2;
}

void BCC(machine_state_t *machine)
{
    int8_t OFFSET = get_branch_offset(machine);
    machine->ALU = machine->PSW & CARRYFLAG;
    if (machine->ALU == 0)
        machine->PC += 2 * OFFSET;
    else
        machine->PC += 2;
}

void BCS(machine_state_t *machine)
{
    int8_t OFFSET = get_branch_offset(machine);
    machine->ALU = machine->PSW & CARRYFLAG;
    if (machine->ALU)
        machine->PC += 2 * OFFSET;
    else
        machine->PC += 2;
}

void MOV(machine_state_t *machine)
{
    setup_src_addressing(machine);
    setup_dest_addressing(machine);

    *machine->DEST = *machine->SRC;
    machine->PC += 2;
    set_zero_flag(machine, *machine->SRC);
}

void MOVB(machine_state_t *machine)
{
    setup_src_byte_addressing(machine);
    setup_dest_byte_addressing(machine);

    *machine->DESTB = *machine->SRCB;
    machine->PC += 2;
    set_zero_flag(machine, *machine->SRCB);
}

void CMP(machine_state_t *machine)
{
    setup_src_addressing(machine);
    setup_dest_addressing(machine);

    machine->ALU = *machine->DEST;
    machine->ALU -= *machine->SRC;
    machine->PC += 2;
    set_zero_flag(machine, machine->ALU);
}

void CMPB(machine_state_t *machine)
{
    setup_src_byte_addressing(machine);
    setup_dest_byte_addressing(machine);

    machine->ALU = *machine->DESTB;
    machine->ALU -= *machine->SRCB;
    machine->PC += 2;
    set_zero_flag(machine, machine->ALU);
}

void BIT(machine_state_t *machine)
{
    setup_src_addressing(machine);
    setup_dest_addressing(machine);

    machine->ALU = *machine->DEST;
    machine->ALU &= *machine->SRC;
    *machine->DEST = machine->ALU;
    set_zero_flag(machine, machine->ALU);
    machine->PC += 2;
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
    machine->PC += 2;
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
    machine->PC += 2;
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
    machine->PC += 2;
}

void SUB(machine_state_t *machine)
{
    setup_src_addressing(machine);
    setup_dest_addressing(machine);

    machine->ALU = *machine->DEST;
    machine->ALU -= *machine->SRC;
    *machine->DEST = machine->ALU;
    machine->PC += 2;
    set_zero_flag(machine, machine->ALU);
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
    machine->PC += 2;
}

void CLR(machine_state_t *machine)
{
    setup_dest_addressing(machine);
    *machine->DEST = 0;
    machine->PC += 2;
}

void CLRB(machine_state_t *machine)
{
    setup_dest_addressing(machine);
    machine->MBR = machine->MAR & 077;
    *machine->DEST = (*machine->DEST & 0177400);
    machine->PC += 2;
}

void COM(machine_state_t *machine)
{
    setup_dest_addressing(machine);
    machine->MBR = machine->MAR & 077;
    *machine->DEST = ~(*machine->DEST);
    machine->PC += 2;
}

void COMB(machine_state_t *machine)
{
    setup_dest_addressing(machine);
    machine->MBR = machine->MAR & 077;
    *machine->DEST = ~(*machine->DEST & 000377) | (*machine->DEST & 0177400);
    machine->PC += 2;
}

void INC(machine_state_t *machine)
{
    setup_dest_addressing(machine);
    (*machine->DEST)++;
    machine->PC += 2;
}

void INCB(machine_state_t *machine)
{
    setup_dest_addressing(machine);
    machine->MBR = machine->MAR & 077;
    *machine->DEST = ((*machine->DEST & 000377) + 1) | (*machine->DEST & 0177400);
    machine->PC += 2;
}

void DEC(machine_state_t *machine)
{
    setup_dest_addressing(machine);
    (*machine->DEST)--;
    machine->PC += 2;
    set_zero_flag(machine, *machine->DEST);
}

void DECB(machine_state_t *machine)
{
    setup_dest_byte_addressing(machine);
    (*machine->DESTB)--;
    machine->PC += 2;
    set_zero_flag(machine, *machine->DESTB);
}

void NEG(machine_state_t *machine)
{
    setup_dest_addressing(machine);
    machine->MBR = machine->MAR & 077;
    *machine->DEST = ~(*machine->DEST) + 1;
    machine->PC += 2;
    set_zero_flag(machine, *machine->DEST);
}

void NEGB(machine_state_t *machine)
{
    setup_dest_byte_addressing(machine);
    machine->MBR = machine->MAR & 077;
    *machine->DESTB = ~(*machine->DESTB) + 1;
    machine->PC += 2;
    set_zero_flag(machine, *machine->DESTB);
}

void ADC(machine_state_t *machine)
{
    setup_dest_addressing(machine);
    machine->MBR = machine->MAR & 077;
    *machine->DEST += (machine->PSW & CARRYFLAG);
    machine->PC += 2;
    set_zero_flag(machine, *machine->DEST);
}

void ADCB(machine_state_t *machine)
{
    setup_dest_byte_addressing(machine);
    *machine->DESTB += (machine->PSW & CARRYFLAG);
    machine->PC += 2;
    set_zero_flag(machine, *machine->DESTB);
}

void SBC(machine_state_t *machine)
{
    setup_dest_addressing(machine);
    *machine->DEST = *machine->DEST - (machine->PSW & CARRYFLAG);
    machine->PC += 2;
    set_zero_flag(machine, *machine->DEST);
}

void SBCB(machine_state_t *machine)
{
    setup_dest_byte_addressing(machine);
    *machine->DESTB = *machine->DESTB - (machine->PSW & CARRYFLAG);
    machine->PC += 2;
    set_zero_flag(machine, *machine->DESTB);
}

void TST(machine_state_t *machine)
{
    setup_dest_addressing(machine);
    set_zero_flag(machine, *machine->DEST);
}

void TSTB(machine_state_t *machine)
{
    setup_dest_byte_addressing(machine);
    set_zero_flag(machine, *machine->DESTB);
}

void ROR(machine_state_t *machine)
{
    setup_dest_addressing(machine);
    *machine->DEST = (*machine->DEST << 1) | (*machine->DEST >> 15);
    machine->PC += 2;
    set_zero_flag(machine, *machine->DEST);
}

void RORB(machine_state_t *machine)
{
    setup_dest_byte_addressing(machine);
    *machine->DESTB = (*machine->DESTB<< 1) | (*machine->DESTB >> 7);
    machine->PC += 2;
    set_zero_flag(machine, *machine->DESTB);
}

void ROL(machine_state_t *machine)
{
    setup_dest_addressing(machine);
    *machine->DEST = (*machine->DEST << 15) | (*machine->DEST >> 1);
    machine->PC += 2;
    set_zero_flag(machine, *machine->DEST);
}

void ROLB(machine_state_t *machine)
{
    setup_dest_byte_addressing(machine);
    *machine->DESTB = (*machine->DESTB << 7) | (*machine->DESTB >> 1);
    machine->PC += 2;
    set_zero_flag(machine, *machine->DESTB);
}

void ASR(machine_state_t *machine)
{
    setup_dest_addressing(machine);
    machine->ALU = *machine->DEST;
    machine->ALU >>= 1;
    *machine->DEST = machine->ALU;
    machine->PC += 2;
    set_zero_flag(machine, *machine->DEST);
}

void ASRB(machine_state_t *machine)
{
    setup_dest_byte_addressing(machine);
    *machine->DESTB >>= 1;
    machine->PC += 2;
    set_zero_flag(machine, *machine->DESTB);
}

void ASL(machine_state_t *machine)
{
    setup_dest_addressing(machine);
    machine->ALU = *machine->DEST;
    machine->ALU <<= 1;
    *machine->DEST = machine->ALU;
    machine->PC += 2;
    set_zero_flag(machine, *machine->DEST);
}

void ASLB(machine_state_t *machine)
{
    setup_dest_byte_addressing(machine);
    *machine->DESTB <<= 1;
    machine->PC += 2;
    set_zero_flag(machine, *machine->DESTB);
}

void MTPS(machine_state_t *machine)
{
    setup_dest_addressing(machine);
    machine->MBR = machine->MAR & 077;
    machine->PSW = *machine->DEST;
    machine->PC += 2;
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
    machine->PC += 2;
}

void MFPS(machine_state_t *machine)
{
    setup_dest_addressing(machine);
    machine->MBR = machine->MAR & 077;
    *machine->DEST = machine->PSW;
    machine->PC += 2;
}

void MUL(machine_state_t *machine)
{
    uint16_t *REG = &(machine->R[(machine->IR & 0700) >> 6]);
    setup_dest_addressing(machine);

    machine->ALU = *machine->DEST;
    machine->ALU *= *REG;
    *REG = machine->ALU;
    machine->PC += 2;
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
    machine->PC += 2;
}

void HALT(machine_state_t *machine)
{
    machine->HALTED = true;
}

void exec_instruction(machine_state_t *machine)
{
    switch (machine->IR & 0177400) {
    // Branch OPS
    case BR_op:
        BR(machine);
        break;
    case BNE_op:
        BNE(machine);
        break;
    case BEQ_op:
        BEQ(machine);
        break;
    case BGE_op:
        BGE(machine);
        break;
    case BLT_op:
        BLT(machine);
        break;
    case BGT_op:
        BGT(machine);
        break;
    case BLE_op:
        BLE(machine);
        break;
    case BPL_op:
        BPL(machine);
        break;
    case BMI_op:
        BMI(machine);
        break;
    case BHI_op:
        BHI(machine);
        break;
    case BLOS_op:
        BLOS(machine);
        break;
    case BVC_op:
        BVC(machine);
        break;
    case BVS_op:
        BVS(machine);
        break;
    case BCC_op:
        BCC(machine);
        break;
    case BCS_op:
        BCS(machine);
        break;
    
    default:
        // Double OP Register Source
        switch (machine->IR & 0177000) {       
        case MUL_op:
            MUL(machine);
            break;
        case DIV_op:
            DIV(machine);
            break;
        case ASH_op:
            ASH(machine);
            break;
        case ASHC_op:
            ASHC(machine);
            break;
        case XOR_op:
            XOR(machine);
            break;
        default:
            switch (machine->IR & 0170000) {
            // Double OPs
            case MOV_op:
                MOV(machine);
                break;
            case MOVB_op:
                MOVB(machine);
                break;
            case CMP_op:
                CMP(machine);
                break;
            case CMPB_op:
                CMPB(machine);
                break;
            case BIT_op:
                BIT(machine);
                break;
            case BITB_op:
                BITB(machine);
                break;
            case BIC_op:
                BIC(machine);
                break;
            case BICB_op:
                BICB(machine);
                break;
            case BIS_op:
                BIS(machine);
                break;
            case BISB_op:
                BISB(machine);
                break;
            case ADD_op:
                ADD(machine);
                break;
            case SUB_op:
                SUB(machine);
                break;
            default:
                // Single OPs
                switch (machine->IR & 0177700) {
                case JMP_op:
                    JMP(machine);
                    break;
                case SWAB_op:
                    SWAB(machine);
                    break;
                case CLR_op:
                    CLR(machine);
                    break;
                case CLRB_op:
                    CLRB(machine);
                    break;
                case COM_op:
                    COM(machine);
                    break;
                case COMB_op:
                    COMB(machine);
                    break;
                case INC_op:
                    INC(machine);
                    break;
                case INCB_op:
                    INCB(machine);
                    break;
                case DEC_op:
                    DEC(machine);
                    break;
                case DECB_op:
                    DECB(machine);
                    break;
                case NEG_op:
                    NEG(machine);
                    break;
                case NEGB_op:
                    NEGB(machine);
                    break;
                case ADC_op:
                    ADC(machine);
                    break;
                case ADCB_op:
                    ADCB(machine);
                    break;
                case SBC_op:
                    SBC(machine);
                    break;
                case SBCB_op:
                    SBCB(machine);
                    break;
                case TST_op:
                    TST(machine);
                    break;
                case TSTB_op:
                    TSTB(machine);
                    break;
                case ROR_op:
                    ROR(machine);
                    break;
                case RORB_op:
                    RORB(machine);
                    break;
                case ROL_op:
                    ROL(machine);
                    break;
                case ROLB_op:
                    ROLB(machine);
                    break;
                case ASR_op:
                    ASR(machine);
                    break;
                case ASRB_op:
                    ASRB(machine);
                    break;
                case ASL_op:
                    ASL(machine);
                    break;
                case ASLB_op:
                    ASLB(machine);
                    break;
                case MTPS_op:
                    MTPS(machine);
                    break;
                case MFPI_op:
                    MFPI(machine);
                    break;
                case MFPD_op:
                    MFPD(machine);
                    break;
                case MTPI_op:
                    MTPI(machine);
                    break;
                case MTPD_op:
                    MTPD(machine);
                    break;
                case SXT_op:
                    SXT(machine);
                    break;
                case MFPS_op:
                    MFPS(machine);
                    break;
                default:
                    HALT(machine);
                    break;
                }
            }
        }
    }
}
