#include <stdio.h>

#include "include/machine_state.h"
#include "include/opcodes.h"

// Destination Addressing

void setup_pc_dest_addressing(machine_state_t *machine)
{
    uint16_t relative = 0;
    uint16_t pc = 0;
    uint8_t mode = (machine->IR & 070) >> 3;
    switch (mode) {
        // From register.
        case 0:
            machine->memory->dest = translate_register(machine->IR & 007);
            break;

        // From immediate.
        case 2:
            pc = machine->memory->get_r(machine->memory, R_PC);
            machine->memory->word_advance_r(machine->memory, R_PC);
            machine->memory->dest = pc;
            break;

        // From absolute.
        case 3:
            pc = machine->memory->get_r(machine->memory, R_PC);
            machine->memory->word_advance_r(machine->memory, R_PC);
            machine->memory->dest = machine->memory->direct_read_word(machine->memory, pc);
            break;

        // Relative
        case 6:
            pc = machine->memory->get_r(machine->memory, R_PC);
            relative = machine->memory->direct_read_word(machine->memory, pc);
            machine->memory->word_advance_r(machine->memory, R_PC);
            pc = machine->memory->get_r(machine->memory, R_PC);
            machine->memory->dest = pc + relative;
            break;
    }
}

void setup_sp_dest_addressing(machine_state_t *machine)
{
    uint16_t pc = 0;
    uint16_t index = 0;
    uint16_t addr = 0;
    uint16_t deferred = 0;
    uint16_t mode = (machine->IR & 070) >> 3;
    uint16_t reg = machine->IR & 007;
    switch (mode) {
        // Register: SP
        case 0:
            machine->memory->dest = translate_register(reg);
            break;

        // Deferred: (SP), top of the stack
        case 1:
            machine->memory->dest = machine->memory->get_r(machine->memory, reg);
            break;

        // Autoincrement: (Rn)+, get top of stack and then pop it
        case 2:
            machine->memory->dest = machine->memory->get_r(machine->memory, reg);
            machine->memory->word_advance_r(machine->memory, reg);
            break;

        // Deferred autoincrement: @(SP)+, top of stack is a pointer to a value,
        // then pop it.
        case 3:
            deferred = machine->memory->get_r(machine->memory, reg);
            addr = machine->memory->direct_read_word(machine->memory, deferred);
            machine->memory->word_advance_r(machine->memory, reg);
            machine->memory->dest = addr;
            break;

        // Autodecrement: -(SP), push value onto stack.
        case 4:
            machine->memory->word_decrease_r(machine->memory, reg);
            addr = machine->memory->get_r(machine->memory, reg);
            machine->memory->dest = addr;
            break;

        // Indexed: X(SP), access item X on the stack
        case 6:
            pc = machine->memory->get_r(machine->memory, R_PC);
            machine->memory->word_advance_r(machine->memory, R_PC);
            index = machine->memory->direct_read_word(machine->memory, pc);
            addr = machine->memory->get_r(machine->memory, reg);
            machine->memory->dest = addr + index;
            break;

        // Deferred index: @X(SP), access item pointed to by item X on the stack
        case 7:
            pc = machine->memory->get_r(machine->memory, R_PC);
            machine->memory->word_advance_r(machine->memory, R_PC);
            index = machine->memory->direct_read_word(machine->memory, pc);
            addr = machine->memory->get_r(machine->memory, reg);
            machine->memory->dest = machine->memory->direct_read_word(machine->memory, addr + index);
            break;
    }
}

void setup_general_dest_addressing(machine_state_t *machine)
{
    uint16_t pc = 0;
    uint16_t index = 0;
    uint16_t deferred = 0;
    uint16_t mode = (machine->IR & 070) >> 3;
    uint16_t reg = machine->IR & 007;
    switch (mode) {
        // Register: Rn
        case 0:
            machine->memory->dest = translate_register(reg);
            break;

        // Deferred: (Rn)
        case 1:
            machine->memory->dest = machine->memory->get_r(machine->memory, reg);
            break;

        // Autoincrement: (Rn)+
        //
        // NOTE: (incremented by one or two) to address the next word or
        // byte depending upon whether the instruction **operates on byte or word data**
        case 2:
            machine->memory->dest = machine->memory->get_r(machine->memory, reg);
            machine->memory->word_advance_r(machine->memory, reg);
            break;

        // Deferred autoincrement: @(Rn)+
        case 3:
            deferred = machine->memory->get_r(machine->memory, reg);
            index = machine->memory->direct_read_word(machine->memory, deferred);
            machine->memory->word_advance(machine->memory, deferred);
            machine->memory->dest = machine->memory->direct_read_word(machine->memory, index);
            break;

        // Autodecrement: -(Rn)
        //
        // NOTE: (decremented by one or two) to address the next word or
        // byte depending upon whether the instruction **operates on byte or word data**
        case 4:
            machine->memory->dest = machine->memory->get_r(machine->memory, reg);
            machine->memory->word_decrease_r(machine->memory, reg);
            break;

        // Deferred autodecrement: @-(Rn)
        case 5:
            deferred = machine->memory->get_r(machine->memory, reg);
            index = machine->memory->direct_read_word(machine->memory, deferred);
            machine->memory->dest = machine->memory->direct_read_word(machine->memory, index);
            machine->memory->word_decrease(machine->memory, deferred);
            break;

        // Indexed: X(Rn)
        case 6:
            pc = machine->memory->get_r(machine->memory, R_PC);
            machine->memory->word_advance_r(machine->memory, R_PC);
            index = machine->memory->direct_read_word(machine->memory, pc);
            machine->memory->dest = machine->memory->direct_read_word(
                machine->memory,
                machine->memory->get_r(machine->memory, reg) + index
            );
            break;

        // Deferred indexed: @X(Rn)
        case 7:
            pc = machine->memory->get_r(machine->memory, R_PC);
            machine->memory->word_advance_r(machine->memory, R_PC);
            index = machine->memory->direct_read_word(machine->memory, pc);
            deferred = machine->memory->get_r(machine->memory, reg);
            machine->memory->dest = machine->memory->direct_read_word(
                machine->memory,
                machine->memory->direct_read_word(machine->memory, deferred) + index
            );
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
    uint16_t relative = 0;
    uint16_t pc = 0;
    uint8_t mode = (machine->IR & 070) >> 3;
    switch (mode) {
        // From register: Rn
        case 0:
            machine->memory->dest = translate_register(machine->IR & 007);
            break;

        // From immediate: #LABEL or #VALUE
        case 2:
            pc = machine->memory->get_r(machine->memory, R_PC);
            machine->memory->word_advance_r(machine->memory, R_PC);
            machine->memory->dest = pc;
            break;

        // From absolute: @#LABEL
        case 3:
            pc = machine->memory->get_r(machine->memory, R_PC);
            machine->memory->word_advance_r(machine->memory, R_PC);
            machine->memory->dest = machine->memory->direct_read_word(machine->memory, pc);
            break;

        // Relative: LABEL
        case 6:
            pc = machine->memory->get_r(machine->memory, R_PC);
            relative = machine->memory->direct_read_word(machine->memory, pc);
            machine->memory->word_advance_r(machine->memory, R_PC);
            pc = machine->memory->get_r(machine->memory, R_PC);
            machine->memory->dest = pc + relative;
            break;

    }
}

void setup_general_dest_byte_addressing(machine_state_t *machine)
{
    uint16_t pc = 0;
    uint16_t index = 0;
    uint16_t deferred = 0;
    uint8_t mode = (machine->IR & 070) >> 3;
    uint8_t reg = machine->IR & 007;
    switch (mode) {
        // Register: Rn
        case 0:
            machine->memory->dest = translate_register(reg);
            break;

        // Deferred: (Rn)
        case 1:
            machine->memory->dest = machine->memory->get_r(machine->memory, reg);
            break;

        // Autoincrement: (Rn)+
        //
        // NOTE: (incremented by one or two) to address the next word or
        // byte depending upon whether the instruction **operates on byte or word data**
        case 2:
            machine->memory->dest = machine->memory->get_r(machine->memory, reg);
            machine->memory->byte_advance_r(machine->memory, reg);
            break;

        // Deferred autoincrement: @(Rn)+
        case 3:
            deferred = machine->memory->get_r(machine->memory, reg);
            index = machine->memory->direct_read_byte(machine->memory, deferred);
            machine->memory->byte_advance(machine->memory, deferred);
            machine->memory->dest = machine->memory->direct_read_byte(machine->memory, index);
            break;

        // Autodecrement: -(Rn)
        //
        // NOTE: (decremented by one or two) to address the next word or
        // byte depending upon whether the instruction **operates on byte or word data**
        case 4:
            machine->memory->dest = machine->memory->get_r(machine->memory, reg);
            machine->memory->byte_decrease_r(machine->memory, reg);
            break;

        // Deferred autodecrement: @-(Rn)
        case 5:
            deferred = machine->memory->get_r(machine->memory, reg);
            index = machine->memory->direct_read_byte(machine->memory, deferred);
            machine->memory->dest = machine->memory->direct_read_byte(machine->memory, index);
            machine->memory->byte_decrease(machine->memory, deferred);
            break;

        // Indexed: X(Rn)
        case 6:
            pc = machine->memory->get_r(machine->memory, R_PC);
            machine->memory->word_advance_r(machine->memory, R_PC);
            index = machine->memory->direct_read_byte(machine->memory, pc);
            machine->memory->dest = machine->memory->direct_read_byte(
                machine->memory,
                machine->memory->get_r(machine->memory, reg) + index
            );
            break;

        // Deferred indexed: @X(Rn)
        case 7:
            pc = machine->memory->get_r(machine->memory, R_PC);
            machine->memory->word_advance_r(machine->memory, R_PC);
            index = machine->memory->direct_read_byte(machine->memory, pc);
            deferred = machine->memory->get_r(machine->memory, reg);
            machine->memory->dest = machine->memory->direct_read_byte(
                machine->memory,
                machine->memory->direct_read_byte(machine->memory, deferred) + index
            );
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
            setup_sp_dest_addressing(machine);
            break;
        default:
            setup_general_dest_byte_addressing(machine);
    }
}


// Source Addressing

void setup_pc_src_addressing(machine_state_t *machine)
{
    uint16_t relative = 0;
    uint16_t pc = 0;
    uint8_t mode = (machine->IR & 07000) >> 9;
    switch (mode) {
        // From register.
        case 0:
            machine->memory->src = translate_register(machine->IR & 007);
            break;

        // From immediate.
        case 2:
            pc = machine->memory->get_r(machine->memory, R_PC);
            machine->memory->word_advance_r(machine->memory, R_PC);
            machine->memory->src = pc;
            break;

        // From absolute.
        case 3:
            pc = machine->memory->get_r(machine->memory, R_PC);
            machine->memory->word_advance_r(machine->memory, R_PC);
            machine->memory->src = machine->memory->direct_read_word(machine->memory, pc);
            break;

        // Relative.
        case 6:
            pc = machine->memory->get_r(machine->memory, R_PC);
            relative = machine->memory->direct_read_word(machine->memory, pc);
            machine->memory->word_advance_r(machine->memory, R_PC);
            pc = machine->memory->get_r(machine->memory, R_PC);
            machine->memory->src = pc + relative;
            break;

    }
}

void setup_sp_src_addressing(machine_state_t *machine)
{
    uint16_t pc = 0;
    uint16_t index = 0;
    uint16_t addr = 0;
    uint16_t deferred = 0;
    uint16_t mode = (machine->IR & 07000) >> 9;
    uint16_t reg = (machine->IR & 00700) >> 6;
    switch (mode) {
        // Register: SP
        case 0:
            machine->memory->src = translate_register(reg);
            break;

        // Deferred: (SP), top of the stack
        case 1:
            machine->memory->src = machine->memory->get_r(machine->memory, reg);
            break;

        // Autoincrement: (Rn)+, get top of stack and then pop it
        case 2:
            machine->memory->src = machine->memory->get_r(machine->memory, reg);
            machine->memory->word_advance_r(machine->memory, reg);
            break;

        // Deferred autoincrement: @(SP)+, top of stack is a pointer to a value,
        // then pop it.
        case 3:
            deferred = machine->memory->get_r(machine->memory, reg);
            addr = machine->memory->direct_read_word(machine->memory, deferred);
            machine->memory->word_advance_r(machine->memory, reg);
            machine->memory->src = addr;
            break;

        // Autodecrement: -(SP), push value onto stack.
        case 4:
            machine->memory->word_decrease_r(machine->memory, reg);
            addr = machine->memory->get_r(machine->memory, reg);
            machine->memory->src = addr;
            break;

        // Indexed: X(SP), access item X on the stack
        case 6:

            pc = machine->memory->get_r(machine->memory, R_PC);
            index = machine->memory->direct_read_word(machine->memory, pc);
            addr = machine->memory->get_r(machine->memory, reg);
            machine->memory->src = addr + index;
            break;

        // Deferred index: @X(SP), access item pointed to by item X on the stack
        case 7:

            pc = machine->memory->get_r(machine->memory, R_PC);
            index = machine->memory->direct_read_word(machine->memory, pc);
            addr = machine->memory->get_r(machine->memory, reg);
            machine->memory->src = machine->memory->direct_read_word(machine->memory, addr + index);
            break;
    }
}

void setup_general_src_addressing(machine_state_t *machine)
{
    uint16_t pc = 0;
    uint16_t index = 0;
    uint16_t deferred = 0;
    uint8_t mode = (machine->IR & 07000) >> 9;
    uint8_t reg = (machine->IR & 00700) >> 6;
    switch (mode) {
        // Register: Rn
        case 0:
            machine->memory->src = translate_register(reg);
            break;

        // Deferred: (Rn)
        case 1:
            machine->memory->src = machine->memory->get_r(machine->memory, reg);
            break;

        // Autoincrement: (Rn)+
        //
        // NOTE: (incremented by one or two) to address the next word or
        // byte depending upon whether the instruction **operates on byte or word data**
        case 2:
            machine->memory->src = machine->memory->get_r(machine->memory, reg);
            machine->memory->word_advance_r(machine->memory, reg);
            break;

        // Deferred autoincrement: @(Rn)+
        case 3:
            deferred = machine->memory->get_r(machine->memory, reg);
            index = machine->memory->direct_read_word(machine->memory, deferred);
            machine->memory->word_advance(machine->memory, deferred);
            machine->memory->src = machine->memory->direct_read_word(machine->memory, index);
            break;

        // Autodecrement: -(Rn)
        //
        // NOTE: (decremented by one or two) to address the next word or
        // byte depending upon whether the instruction **operates on byte or word data**
        case 4:
            machine->memory->src = machine->memory->get_r(machine->memory, reg);
            machine->memory->word_decrease_r(machine->memory, reg);
            break;

        // Deferred autodecrement: @-(Rn)
        case 5:
            machine->memory->word_decrease_r(machine->memory, reg);
            deferred = machine->memory->get_r(machine->memory, reg);
            index = machine->memory->direct_read_word(machine->memory, deferred);
            machine->memory->src = index;
            break;

        // Indexed: X(Rn)
        case 6:
            pc = machine->memory->get_r(machine->memory, R_PC);
            machine->memory->word_advance_r(machine->memory, R_PC);
            index = machine->memory->direct_read_word(machine->memory, pc);
            machine->memory->src = machine->memory->direct_read_word(
                machine->memory,
                machine->memory->get_r(machine->memory, reg) + index
            );
            break;

        // Deferred indexed: @X(Rn)
        case 7:
            pc = machine->memory->get_r(machine->memory, R_PC);
            machine->memory->word_advance_r(machine->memory, R_PC);
            index = machine->memory->direct_read_word(machine->memory, pc);
            deferred = machine->memory->get_r(machine->memory, reg);
            machine->memory->src = machine->memory->direct_read_word(
                machine->memory,
                machine->memory->direct_read_word(machine->memory, deferred) + index
            );
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
    uint16_t relative = 0;
    uint16_t pc = 0;
    uint8_t mode = (machine->IR & 07000) >> 9;
    switch (mode) {
        // From register.
        case 0:
            machine->memory->src = translate_register(machine->IR & 007);
            break;

        // From immediate.
        case 2:
            pc = machine->memory->get_r(machine->memory, R_PC);
            machine->memory->word_advance_r(machine->memory, R_PC);
            machine->memory->src = pc;
            break;

        // From absolute.
        case 3:
            pc = machine->memory->get_r(machine->memory, R_PC);
            machine->memory->word_advance_r(machine->memory, R_PC);
            machine->memory->src = machine->memory->direct_read_word(machine->memory, pc);
            break;

        // Relative.
        case 6:
            pc = machine->memory->get_r(machine->memory, R_PC);
            relative = machine->memory->direct_read_word(machine->memory, pc);
            machine->memory->word_advance_r(machine->memory, R_PC);
            pc = machine->memory->get_r(machine->memory, R_PC);
            machine->memory->src = pc + relative;
            break;
    }
}

void setup_general_src_byte_addressing(machine_state_t *machine)
{
    uint16_t pc = 0;
    uint16_t index = 0;
    uint16_t deferred = 0;
    uint8_t mode = (machine->IR & 07000) >> 9;
    uint8_t reg = (machine->IR & 00700) >> 6;
    switch (mode) {
        // Register: Rn
        case 0:
            machine->memory->src = translate_register(reg);
            break;

        // Deferred: (Rn)
        case 1:
            machine->memory->src = machine->memory->get_r(machine->memory, reg);
            break;

        // Autoincrement: (Rn)+
        //
        // NOTE: (incremented by one or two) to address the next word or
        // byte depending upon whether the instruction **operates on byte or word data**
        case 2:
            machine->memory->src = machine->memory->get_r(machine->memory, reg);
            machine->memory->byte_advance_r(machine->memory, reg);
            break;

        // Deferred autoincrement: @(Rn)+
        case 3:
            deferred = machine->memory->get_r(machine->memory, reg);
            index = machine->memory->direct_read_byte(machine->memory, deferred);
            machine->memory->byte_advance(machine->memory, deferred);
            machine->memory->src = machine->memory->direct_read_byte(machine->memory, index);
            break;

        // Autodecrement: -(Rn)
        //
        // NOTE: (decremented by one or two) to address the next word or
        // byte depending upon whether the instruction **operates on byte or word data**
        case 4:
            machine->memory->src = machine->memory->get_r(machine->memory, reg);
            machine->memory->byte_decrease_r(machine->memory, reg);
            break;

        // Deferred autodecrement: @-(Rn)
        case 5:
            deferred = machine->memory->get_r(machine->memory, reg);
            index = machine->memory->direct_read_byte(machine->memory, deferred);
            machine->memory->src = machine->memory->direct_read_byte(machine->memory, index);
            machine->memory->byte_decrease(machine->memory, deferred);
            break;

        // Indexed: X(Rn)
        case 6:
            pc = machine->memory->get_r(machine->memory, R_PC);
            machine->memory->word_advance_r(machine->memory, R_PC);
            index = machine->memory->direct_read_byte(machine->memory, pc);
            machine->memory->src = machine->memory->direct_read_byte(
                machine->memory,
                machine->memory->get_r(machine->memory, reg) + index
            );
            break;

        // Deferred indexed: @X(Rn)
        case 7:
            pc = machine->memory->get_r(machine->memory, R_PC);
            machine->memory->word_advance_r(machine->memory, R_PC);
            index = machine->memory->direct_read_byte(machine->memory, pc);
            deferred = machine->memory->get_r(machine->memory, reg);
            machine->memory->src = machine->memory->direct_read_byte(
                machine->memory,
                machine->memory->direct_read_byte(machine->memory, deferred) + index
            );
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
            setup_sp_src_addressing(machine);
            break;
        default:
            setup_general_src_byte_addressing(machine);
    }
}

void set_zero_flag(machine_state_t *machine, uint16_t value)
{
    uint16_t PS = 0;
    if (value == 0) {
        PS = machine->memory->get_r(machine->memory, R_PS);
        machine->memory->set_r(machine->memory, R_PS, PS | ZEROFLAG);
    } else {
        PS = machine->memory->get_r(machine->memory, R_PS);
        PS = (PS & ZEROFLAG) ? (PS ^ ZEROFLAG) : PS;
        machine->memory->set_r(machine->memory, R_PS, PS);
    }
}

void set_negative_flag_b(machine_state_t *machine, uint8_t value)
{
    uint16_t PS = 0;
    uint8_t mask = 0200;
    if (value & mask) {
        PS = machine->memory->get_r(machine->memory, R_PS);
        machine->memory->set_r(machine->memory, R_PS, PS | NEGATIVEFLAG);
    } else {
        PS = machine->memory->get_r(machine->memory, R_PS);
        PS = (PS & NEGATIVEFLAG) ? (PS ^ NEGATIVEFLAG) : PS;
        machine->memory->set_r(machine->memory, R_PS, PS);
    }
}

void set_negative_flag_w(machine_state_t *machine, uint16_t value)
{
    uint16_t PS = 0;
    uint16_t mask = 0100000;
    if (value & mask) {
        PS = machine->memory->get_r(machine->memory, R_PS);
        machine->memory->set_r(machine->memory, R_PS, PS | NEGATIVEFLAG);
    } else {
        PS = machine->memory->get_r(machine->memory, R_PS);
        PS = (PS & NEGATIVEFLAG) ? (PS ^ NEGATIVEFLAG) : PS;
        machine->memory->set_r(machine->memory, R_PS, PS);
    }
}

int8_t get_branch_offset(machine_state_t *machine)
{
    return (int8_t) (machine->IR & 0377);
}

void NOP(machine_state_t *machine)
{

}

void BR(machine_state_t *machine)
{
    // Branch is relying on PC being advanced to next instruction.


    uint16_t pc = machine->memory->get_r(machine->memory, R_PC);
    int8_t OFFSET = get_branch_offset(machine);
    machine->memory->set_r(machine->memory, R_PC, pc + (2 * OFFSET));
}

void BNE(machine_state_t *machine)
{
    // Branch is relying on PC being advanced to next instruction.


    uint16_t pc = machine->memory->get_r(machine->memory, R_PC);
    uint16_t ps = machine->memory->get_r(machine->memory, R_PS);
    int8_t OFFSET = get_branch_offset(machine);
    int16_t NEW_PC = pc + (int8_t) (2 * OFFSET);
    machine->ALU = ps & ZEROFLAG;
    if (machine->ALU == 0)
        machine->memory->set_r(machine->memory, R_PC, (uint16_t) NEW_PC);
}

void BEQ(machine_state_t *machine)
{
    // Branch is relying on PC being advanced to next instruction.

    uint16_t pc = machine->memory->get_r(machine->memory, R_PC);
    uint16_t ps = machine->memory->get_r(machine->memory, R_PS);
    int8_t OFFSET = get_branch_offset(machine);
    machine->ALU = ps & ZEROFLAG;
    if (machine->ALU == ZEROFLAG)
        machine->memory->set_r(machine->memory, R_PC, pc + (2 * OFFSET));
}

void BGE(machine_state_t *machine)
{
    // Branch is relying on PC being advanced to next instruction.


    uint16_t pc = machine->memory->get_r(machine->memory, R_PC);
    uint16_t ps = machine->memory->get_r(machine->memory, R_PS);
    int8_t OFFSET = get_branch_offset(machine);
    machine->ALU = (ps & NEGATIVEFLAG) >> 3;
    machine->ALU ^= (ps & OVERFLOWFLAG) >> 1;
    if (machine->ALU == 0)
        machine->memory->set_r(machine->memory, R_PC, pc + (2 * OFFSET));
}

void BLT(machine_state_t *machine)
{
    // Branch is relying on PC being advanced to next instruction.


    uint16_t pc = machine->memory->get_r(machine->memory, R_PC);
    uint16_t ps = machine->memory->get_r(machine->memory, R_PS);
    int8_t OFFSET = get_branch_offset(machine);
    machine->ALU = (ps & NEGATIVEFLAG) >> 3;
    machine->ALU ^= (ps & OVERFLOWFLAG) >> 1;
    if (machine->ALU)
        machine->memory->set_r(machine->memory, R_PC, pc + (2 * OFFSET));
}

void BGT(machine_state_t *machine)
{
    // Branch is relying on PC being advanced to next instruction.


    uint16_t pc = machine->memory->get_r(machine->memory, R_PC);
    uint16_t ps = machine->memory->get_r(machine->memory, R_PS);
    int8_t OFFSET = get_branch_offset(machine);
    machine->ALU = (ps & ZEROFLAG) >> 2;
    machine->ALU |= ((ps & NEGATIVEFLAG) >> 3) ^ ((ps & OVERFLOWFLAG) >> 1);
    if (machine->ALU == 0)
        machine->memory->set_r(machine->memory, R_PC, pc + (2 * OFFSET));
}

void BLE(machine_state_t *machine)
{
    // Branch is relying on PC being advanced to next instruction.


    uint16_t pc = machine->memory->get_r(machine->memory, R_PC);
    uint16_t ps = machine->memory->get_r(machine->memory, R_PS);
    int8_t OFFSET = get_branch_offset(machine);
    machine->ALU = (ps & ZEROFLAG) >> 2;
    machine->ALU |= ((ps & NEGATIVEFLAG) >> 3) ^ ((ps & OVERFLOWFLAG) >> 1);
    if (machine->ALU)
        machine->memory->set_r(machine->memory, R_PC, pc + (2 * OFFSET));
}

void BPL(machine_state_t *machine)
{
    // Branch is relying on PC being advanced to next instruction.
    uint16_t pc = machine->memory->get_r(machine->memory, R_PC);
    uint16_t ps = machine->memory->get_r(machine->memory, R_PS);
    int8_t OFFSET = get_branch_offset(machine);
    if ((ps & NEGATIVEFLAG) == 0)
        machine->memory->set_r(machine->memory, R_PC, pc + (2 * OFFSET));
}

void BMI(machine_state_t *machine)
{
    // Branch is relying on PC being advanced to next instruction.


    uint16_t pc = machine->memory->get_r(machine->memory, R_PC);
    uint16_t ps = machine->memory->get_r(machine->memory, R_PS);
    int8_t OFFSET = get_branch_offset(machine);
    if (ps & NEGATIVEFLAG)
        machine->memory->set_r(machine->memory, R_PC, pc + (2 * OFFSET));
}

void BHI(machine_state_t *machine)
{
    // Branch is relying on PC being advanced to next instruction.


    uint16_t pc = machine->memory->get_r(machine->memory, R_PC);
    uint16_t ps = machine->memory->get_r(machine->memory, R_PS);
    int8_t OFFSET = get_branch_offset(machine);
    machine->ALU = ps & CARRYFLAG;
    machine->ALU |= (ps & ZEROFLAG) >> 2;
    if (machine->ALU == 0)
        machine->memory->set_r(machine->memory, R_PC, pc + (2 * OFFSET));
}

void BLOS(machine_state_t *machine)
{
    // Branch is relying on PC being advanced to next instruction.


    uint16_t pc = machine->memory->get_r(machine->memory, R_PC);
    uint16_t ps = machine->memory->get_r(machine->memory, R_PS);
    int8_t OFFSET = get_branch_offset(machine);
    machine->ALU = ps & CARRYFLAG;
    machine->ALU |= (ps & ZEROFLAG) >> 2;
    if (machine->ALU)
        machine->memory->set_r(machine->memory, R_PC, pc + (2 * OFFSET));
}

void BVC(machine_state_t *machine)
{
    // Branch is relying on PC being advanced to next instruction.


    uint16_t pc = machine->memory->get_r(machine->memory, R_PC);
    uint16_t ps = machine->memory->get_r(machine->memory, R_PS);
    int8_t OFFSET = get_branch_offset(machine);
    machine->ALU = ps & OVERFLOWFLAG;
    if (machine->ALU == 0)
        machine->memory->set_r(machine->memory, R_PC, pc + (2 * OFFSET));
}

void BVS(machine_state_t *machine)
{
    // Branch is relying on PC being advanced to next instruction.


    uint16_t pc = machine->memory->get_r(machine->memory, R_PC);
    uint16_t ps = machine->memory->get_r(machine->memory, R_PS);
    int8_t OFFSET = get_branch_offset(machine);
    machine->ALU = ps & OVERFLOWFLAG;
    if (machine->ALU)
        machine->memory->set_r(machine->memory, R_PC, pc + (2 * OFFSET));
}

void BCC(machine_state_t *machine)
{
    // Branch is relying on PC being advanced to next instruction.


    uint16_t pc = machine->memory->get_r(machine->memory, R_PC);
    uint16_t ps = machine->memory->get_r(machine->memory, R_PS);
    int8_t OFFSET = get_branch_offset(machine);
    machine->ALU = ps & CARRYFLAG;
    if (machine->ALU == 0)
        machine->memory->set_r(machine->memory, R_PC, pc + (2 * OFFSET));
}

void BCS(machine_state_t *machine)
{
    // Branch is relying on PC being advanced to next instruction.


    uint16_t pc = machine->memory->get_r(machine->memory, R_PC);
    uint16_t ps = machine->memory->get_r(machine->memory, R_PS);
    int8_t OFFSET = get_branch_offset(machine);
    machine->ALU = ps & CARRYFLAG;
    if (machine->ALU)
        machine->memory->set_r(machine->memory, R_PC, pc + (2 * OFFSET));
}

void MOV(machine_state_t *machine)
{
    setup_src_addressing(machine);
    setup_dest_addressing(machine);
    machine->ALU = machine->memory->read_word(machine->memory);
    machine->memory->write_word(machine->memory, machine->ALU);
    set_zero_flag(machine, machine->ALU);
    set_negative_flag_w(machine, machine->ALU);
}

void MOVB(machine_state_t *machine)
{
    setup_src_byte_addressing(machine);
    setup_dest_byte_addressing(machine);

    machine->ALU = machine->memory->read_byte(machine->memory);
    machine->memory->write_byte(machine->memory, machine->ALU);

    set_zero_flag(machine, machine->ALU);
    set_negative_flag_b(machine, machine->ALU);
}

void CMP(machine_state_t *machine)
{
    setup_src_addressing(machine);
    setup_dest_addressing(machine);

    machine->ALU = machine->memory->direct_read_word(machine->memory, machine->memory->dest);
    machine->ALU -= machine->memory->read_word(machine->memory);

    set_zero_flag(machine, machine->ALU);
}

void CMPB(machine_state_t *machine)
{
    setup_src_byte_addressing(machine);
    setup_dest_byte_addressing(machine);

    machine->ALU = machine->memory->direct_read_byte(machine->memory, machine->memory->dest);
    machine->ALU -= machine->memory->read_byte(machine->memory);

    set_zero_flag(machine, machine->ALU);
}

void BIT(machine_state_t *machine)
{
    setup_src_addressing(machine);
    setup_dest_addressing(machine);

    machine->ALU = machine->memory->read_word(machine->memory);
    machine->ALU &= machine->memory->direct_read_word(machine->memory, machine->memory->dest);
    set_zero_flag(machine, machine->ALU);
}

void BITB(machine_state_t *machine)
{
    NOP(machine);
}

void BIC(machine_state_t *machine)
{
    setup_src_addressing(machine);
    setup_dest_addressing(machine);

    machine->ALU = machine->memory->direct_read_word(machine->memory, machine->memory->dest);
    machine->ALU &= ~(machine->memory->read_word(machine->memory));
    machine->memory->write_word(machine->memory, machine->ALU);

}

void BICB(machine_state_t *machine)
{
    NOP(machine);
}

void BIS(machine_state_t *machine)
{
    setup_src_addressing(machine);
    setup_dest_addressing(machine);

    machine->ALU = machine->memory->direct_read_word(machine->memory, machine->memory->dest);
    machine->ALU |= machine->memory->read_word(machine->memory);
    machine->memory->write_word(machine->memory, machine->ALU);

}

void BISB(machine_state_t *machine)
{
    NOP(machine);
}

void ADD(machine_state_t *machine)
{
    setup_src_addressing(machine);
    setup_dest_addressing(machine);

    machine->ALU = machine->memory->direct_read_word(machine->memory, machine->memory->dest);
    machine->ALU += machine->memory->read_word(machine->memory);
    machine->memory->write_word(machine->memory, machine->ALU);

}

void SUB(machine_state_t *machine)
{
    setup_src_addressing(machine);
    setup_dest_addressing(machine);

    machine->ALU = machine->memory->direct_read_word(machine->memory, machine->memory->dest);
    machine->ALU -= machine->memory->read_word(machine->memory);
    machine->memory->write_word(machine->memory, machine->ALU);

    set_zero_flag(machine, machine->ALU);
}

void JMP(machine_state_t *machine)
{
    setup_dest_addressing(machine);
    machine->memory->set_r(
        machine->memory,
        R_PC,
        machine->memory->direct_read_word(machine->memory, machine->memory->dest)
    );
}

void SWAB(machine_state_t *machine)
{
    setup_dest_addressing(machine);
    uint16_t word = machine->memory->direct_read_word(machine->memory, machine->memory->dest);
    machine->memory->write_word(machine->memory, ((word & 0177400) >> 8) | ((word & 000377) << 8));

}

void CLR(machine_state_t *machine)
{
    setup_dest_addressing(machine);
    machine->memory->write_word(machine->memory, 0);

}

void CLRB(machine_state_t *machine)
{
    setup_dest_addressing(machine);
    machine->memory->write_byte(machine->memory, 0);

}

void COM(machine_state_t *machine)
{
    setup_dest_addressing(machine);
    machine->memory->write_word(
        machine->memory,
        ~(machine->memory->direct_read_word(machine->memory, machine->memory->dest))
    );

}

void COMB(machine_state_t *machine)
{
    setup_dest_addressing(machine);
    machine->memory->write_byte(
        machine->memory,
        ~(machine->memory->direct_read_byte(machine->memory, machine->memory->dest))
    );

}

void INC(machine_state_t *machine)
{
    setup_dest_addressing(machine);
    uint16_t word = machine->memory->direct_read_word(machine->memory, machine->memory->dest);
    ++word;
    machine->memory->write_word(machine->memory, word);

}

void INCB(machine_state_t *machine)
{
    setup_dest_addressing(machine);
    uint8_t byte = machine->memory->direct_read_byte(machine->memory, machine->memory->dest);
    byte++;
    machine->memory->write_byte(machine->memory, byte);

}

void DEC(machine_state_t *machine)
{
    setup_dest_addressing(machine);
    uint16_t word = machine->memory->direct_read_word(machine->memory, machine->memory->dest);
    word--;
    machine->memory->write_word(machine->memory, word);

    set_zero_flag(machine, word);
}

void DECB(machine_state_t *machine)
{
    setup_dest_byte_addressing(machine);
    uint8_t byte = machine->memory->direct_read_byte(machine->memory, machine->memory->dest);
    byte--;
    machine->memory->write_byte(machine->memory, byte);

    set_zero_flag(machine, byte);
}

void NEG(machine_state_t *machine)
{
    setup_dest_addressing(machine);
    machine->ALU = machine->memory->direct_read_word(machine->memory, machine->memory->dest);
    machine->ALU = ~(machine->ALU) + 1;
    machine->memory->write_word(machine->memory, machine->ALU);

    set_zero_flag(machine, machine->ALU);
}

void NEGB(machine_state_t *machine)
{
    setup_dest_byte_addressing(machine);
    uint8_t byte = machine->memory->direct_read_byte(machine->memory, machine->memory->dest);
    byte = ~(byte) + 1;
    machine->memory->write_byte(machine->memory, byte);

    set_zero_flag(machine, byte);
}

void ADC(machine_state_t *machine)
{
    setup_dest_addressing(machine);
    uint16_t word = machine->memory->direct_read_word(machine->memory, machine->memory->dest);
    word += (machine->memory->get_r(machine->memory, R_PS) & CARRYFLAG);
    machine->memory->write_word(machine->memory, word);

    set_zero_flag(machine, word);
}

void ADCB(machine_state_t *machine)
{
    setup_dest_byte_addressing(machine);
    uint8_t byte = machine->memory->direct_read_byte(machine->memory, machine->memory->dest);
    byte += (machine->memory->get_r(machine->memory, R_PS) & CARRYFLAG);
    machine->memory->write_byte(machine->memory, byte);

    set_zero_flag(machine, byte);
}

void SBC(machine_state_t *machine)
{
    setup_dest_addressing(machine);
    uint16_t word = machine->memory->direct_read_word(machine->memory, machine->memory->dest);
    word = word - (machine->memory->get_r(machine->memory, R_PS) & CARRYFLAG);
    machine->memory->write_word(machine->memory, word);

    set_zero_flag(machine, word);
}

void SBCB(machine_state_t *machine)
{
    setup_dest_byte_addressing(machine);
    uint8_t byte = machine->memory->direct_read_byte(machine->memory, machine->memory->dest);
    byte = byte - (machine->memory->get_r(machine->memory, R_PS) & CARRYFLAG);
    machine->memory->write_byte(machine->memory, byte);

    set_zero_flag(machine, byte);
}

void TST(machine_state_t *machine)
{
    setup_dest_addressing(machine);
    machine->ALU = machine->memory->direct_read_word(machine->memory, machine->memory->dest);
    set_zero_flag(machine, machine->ALU);
    set_negative_flag_w(machine, machine->ALU);

}

void TSTB(machine_state_t *machine)
{
    setup_dest_byte_addressing(machine);
    machine->ALU = machine->memory->direct_read_byte(machine->memory, machine->memory->dest);
    set_zero_flag(machine, machine->ALU);
    set_negative_flag_b(machine, machine->ALU);
}

void ROR(machine_state_t *machine)
{
    setup_dest_addressing(machine);
    uint16_t word = machine->memory->direct_read_word(machine->memory, machine->memory->dest);
    word = (word << 1) | (word >> 15);
    machine->memory->write_word(machine->memory, word);

    set_zero_flag(machine, word);
}

void RORB(machine_state_t *machine)
{
    setup_dest_byte_addressing(machine);
    uint8_t byte = machine->memory->direct_read_byte(machine->memory, machine->memory->dest);
    byte = (byte << 1) | (byte >> 7);
    machine->memory->write_byte(machine->memory, byte);

    set_zero_flag(machine, byte);
}

void ROL(machine_state_t *machine)
{
    setup_dest_addressing(machine);
    uint16_t word = machine->memory->direct_read_word(machine->memory, machine->memory->dest);
    word = (word << 15) | (word >> 1);
    machine->memory->write_word(machine->memory, word);

    set_zero_flag(machine, word);
}

void ROLB(machine_state_t *machine)
{
    setup_dest_byte_addressing(machine);
    uint8_t byte = machine->memory->direct_read_byte(machine->memory, machine->memory->dest);
    byte = (byte << 7) | (byte >> 1);
    machine->memory->write_byte(machine->memory, byte);

    set_zero_flag(machine, byte);
}

void ASR(machine_state_t *machine)
{
    setup_dest_addressing(machine);
    machine->ALU = machine->memory->direct_read_word(machine->memory, machine->memory->dest);
    machine->ALU >>= 1;
    machine->memory->write_word(machine->memory, machine->ALU);

    set_zero_flag(machine, machine->ALU);
}

void ASRB(machine_state_t *machine)
{
    setup_dest_byte_addressing(machine);
    machine->ALU = machine->memory->direct_read_byte(machine->memory, machine->memory->dest);
    machine->ALU >>= 1;
    machine->memory->write_byte(machine->memory, machine->ALU);

    set_zero_flag(machine, machine->ALU);
}

void ASL(machine_state_t *machine)
{
    setup_dest_addressing(machine);
    machine->ALU = machine->memory->direct_read_word(machine->memory, machine->memory->dest);
    machine->ALU <<= 1;
    machine->memory->write_word(machine->memory, machine->ALU);

    set_zero_flag(machine, machine->ALU);
}

void ASLB(machine_state_t *machine)
{
    setup_dest_byte_addressing(machine);
    machine->ALU = machine->memory->direct_read_byte(machine->memory, machine->memory->dest);
    machine->ALU <<= 1;
    machine->memory->write_byte(machine->memory, machine->ALU);

    set_zero_flag(machine, machine->ALU);
}

void MTPS(machine_state_t *machine)
{
    setup_dest_addressing(machine);
    machine->memory->set_r(
        machine->memory,
        R_PS,
        machine->memory->direct_read_word(machine->memory, machine->memory->dest)
    );

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
    // setup_dest_addressing(machine);
    // if (machine->PSW & NEGATIVEFLAG) {
    //     *machine->DEST = -1;
    // } else {
    //     *machine->DEST = 0;
    // }
    //
    NOP(machine);
}

void MFPS(machine_state_t *machine)
{
    setup_dest_addressing(machine);
    machine->memory->write_word(
        machine->memory,
        machine->memory->get_r(machine->memory, R_PS)
    );

}

void MUL(machine_state_t *machine)
{
    // uint16_t *REG = &(machine->R[(machine->IR & 0700) >> 6]);
    // setup_dest_addressing(machine);

    // machine->ALU = *machine->DEST;
    // machine->ALU *= *REG;
    // *REG = machine->ALU;
    //
    NOP(machine);
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
    uint8_t reg = (machine->IR & 0700) >> 6;
    setup_dest_addressing(machine);

    machine->ALU = machine->memory->get_r(machine->memory, reg);
    machine->ALU ^= machine->memory->direct_read_word(machine->memory, machine->memory->dest);
    machine->memory->write_word(machine->memory, machine->ALU);

    set_zero_flag(machine, machine->ALU);
}

void JSR(machine_state_t *machine)
{
    uint8_t reg = (machine->IR & 0700) >> 6;
    setup_dest_addressing(machine);


    uint16_t tmp = machine->memory->direct_read_word(machine->memory, machine->memory->dest);
    uint16_t reg_contents = machine->memory->get_r(machine->memory, reg);

    uint16_t sp = machine->memory->get_r(machine->memory, R_SP);
    machine->memory->direct_write_word(machine->memory, sp, reg_contents);
    machine->memory->word_decrease_r(machine->memory, R_SP);


    uint16_t pc = machine->memory->get_r(machine->memory, R_PC);
    machine->memory->set_r(machine->memory, reg, pc);
    machine->memory->set_r(machine->memory, R_PC, tmp);
}

void RTS(machine_state_t *machine)
{
    uint8_t reg = machine->IR & 07;
    uint16_t reg_contents = machine->memory->get_r(machine->memory, reg);
    machine->memory->set_r(machine->memory, R_PC, reg_contents);

    machine->memory->word_advance_r(machine->memory, R_SP);
    uint16_t sp = machine->memory->get_r(machine->memory, R_SP);
    uint16_t top_stack = machine->memory->direct_read_word(machine->memory, sp);
    machine->memory->set_r(machine->memory, reg, top_stack);
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
        // Double OP Register Source and JSR
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
        case JSR_op:
            JSR(machine);
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
                case RTS_op:
                    RTS(machine);
                    break;
                default:
                    HALT(machine);
                    break;
                }
            }
        }
    }
}
