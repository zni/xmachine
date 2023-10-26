#include <stdio.h>

#include "include/machine_state.h"
#include "include/opcodes.h"

void setup_dest_addressing(machine_state_t *machine)
{
    // From immediate.
    if ((machine->IR & 077) == 027) {
        machine->PC += 2;
        machine->MBR = machine->PC;
        machine->DEST = &(machine->MEMORY[machine->MBR]);

    // From absolute.
    } else if ((machine->IR & 077) == 037) {
        machine->PC += 2;
        machine->MBR = machine->PC;
        machine->MAR = machine->MEMORY[machine->MBR];
        machine->DEST = &(machine->MEMORY[machine->MAR]);

    // From register.
    } else if ((machine->IR & 070) == 00) {
        machine->DEST = &(machine->R[(machine->IR & 007)]);
    }
}

void setup_dest_byte_addressing(machine_state_t *machine)
{
    if ((machine->IR & 077) == 027) {
        machine->PC += 2;
        machine->MBR = machine->PC;
        machine->DESTB = &(machine->MEMORY[machine->MBR]);

    } else if ((machine->IR & 077) == 037) {
        machine->PC += 2;
        machine->MBR = machine->PC;
        machine->MAR = machine->MEMORY[machine->MBR];
        machine->DESTB = &(machine->MEMORY[machine->MAR]);

    // From register.
    } else if ((machine->IR & 070) == 00) {
        machine->DESTB = &(machine->R[(machine->IR & 007)]);
    }
}

void setup_src_addressing(machine_state_t *machine)
{
    // From immediate address.
    if ((machine->IR & 07700) == 02700) {
        machine->PC += 2;
        machine->MBR = machine->PC;
        machine->SRC = &(machine->MEMORY[machine->MBR]);

    // From absolute address.
    } else if ((machine->IR & 03700) == 03700) {
        machine->PC += 2;
        machine->MBR = machine->PC;
        machine->MAR = machine->MEMORY[machine->MBR];
        machine->SRC = &(machine->MEMORY[machine->MAR]);

    // From register.
    } else if ((machine->IR & 07000) == 0) {
        machine->SRC = &(machine->R[(machine->IR & 00700) >> 6]);
    }
}

void setup_src_byte_addressing(machine_state_t *machine)
{
    // From immediate address.
    if ((machine->IR & 07700) == 02700) {
        machine->PC += 2;
        machine->MBR = machine->PC;
        machine->SRCB = &(machine->MEMORY[machine->MBR]);

    // From absolute address.
    } else if ((machine->IR & 03700) == 03700) {
        machine->PC += 2;
        machine->MBR = machine->PC;
        machine->MAR = machine->MEMORY[machine->MBR];
        machine->SRCB = &(machine->MEMORY[machine->MAR]);

    // From register.
    } else if ((machine->IR & 07000) == 0) {
        machine->SRCB = &(machine->R[(machine->IR & 00700) >> 6]);
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
    machine->PC += 2;
}

void MOVB(machine_state_t *machine)
{
    setup_src_byte_addressing(machine);
    setup_dest_byte_addressing(machine);

    *machine->DESTB = *machine->SRCB;
    machine->PC += 2;
}

void CMP(machine_state_t *machine)
{
    setup_src_addressing(machine);
    setup_dest_addressing(machine);

    machine->ALU = *machine->DEST;
    machine->ALU -= *machine->SRC;
    machine->PC += 2;
    set_zero_flag(machine);
}

void CMPB(machine_state_t *machine)
{
    setup_src_byte_addressing(machine);
    setup_dest_byte_addressing(machine);

    machine->ALU = *machine->DESTB;
    machine->ALU -= *machine->SRCB;
    machine->PC += 2;
    set_zero_flag(machine);
}

void BIT(machine_state_t *machine)
{
    setup_src_addressing(machine);
    setup_dest_addressing(machine);

    machine->ALU = *machine->DEST;
    machine->ALU &= *machine->SRC;
    *machine->DEST = machine->ALU;
    set_zero_flag(machine);
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
    set_zero_flag(machine);
}

void DECB(machine_state_t *machine)
{
    setup_dest_addressing(machine);
    machine->MBR = machine->MAR & 077;
    *machine->DEST = ((*machine->DEST & 000377) - 1) | (*machine->DEST & 0177400);
    machine->PC += 2;
}

void NEG(machine_state_t *machine)
{
    setup_dest_addressing(machine);
    machine->MBR = machine->MAR & 077;
    *machine->DEST = ~(*machine->DEST) + 1;
    machine->PC += 2;
}

void NEGB(machine_state_t *machine)
{
    setup_dest_addressing(machine);
    machine->MBR = machine->MAR & 077;
    *machine->DEST = ((~(*machine->DEST & 000377) + 1) & 000377) | (*machine->DEST & 0177400);
    machine->PC += 2;
}

void ADC(machine_state_t *machine)
{
    setup_dest_addressing(machine);
    machine->MBR = machine->MAR & 077;
    *machine->DEST = *machine->DEST + (machine->PSW & CARRYFLAG);
    machine->PC += 2;
}

void ADCB(machine_state_t *machine)
{
    setup_dest_addressing(machine);
    machine->MBR = machine->MAR & 077;
    *machine->DEST = ((*machine->DEST & 000377) + (machine->PSW & CARRYFLAG)) | (*machine->DEST & 0177400);
    machine->PC += 2;
}

void SBC(machine_state_t *machine)
{
    setup_dest_addressing(machine);
    machine->MBR = machine->MAR & 077;
    *machine->DEST = *machine->DEST - (machine->PSW & CARRYFLAG);
    machine->PC += 2;
}

void SBCB(machine_state_t *machine)
{
    setup_dest_addressing(machine);
    machine->MBR = machine->MAR & 077;
    *machine->DEST = ((*machine->DEST & 000377) - (machine->PSW & CARRYFLAG)) | (*machine->DEST & 0177400);
    machine->PC += 2;
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
    machine->PC += 2;
}

void RORB(machine_state_t *machine)
{
    setup_dest_addressing(machine);
    machine->MBR = machine->MAR & 077;
    *machine->DEST = (((*machine->DEST & 000377) << 1) | ((*machine->DEST & 000377) >> 7)) | (*machine->DEST & 0177400);
    machine->PC += 2;
}

void ROL(machine_state_t *machine)
{
    setup_dest_addressing(machine);
    machine->MBR = machine->MAR & 077;
    *machine->DEST = (*machine->DEST << 15) | (*machine->DEST >> 1);
    machine->PC += 2;
}

void ROLB(machine_state_t *machine)
{
    setup_dest_addressing(machine);
    machine->MBR = machine->MAR & 077;
    *machine->DEST = (((*machine->DEST & 000377) << 7) | ((*machine->DEST & 000377) >> 1)) | (*machine->DEST & 0177400);
    machine->PC += 2;
}

void ASR(machine_state_t *machine)
{
    setup_dest_addressing(machine);
    machine->MBR = machine->MAR & 077;
    machine->ALU = *machine->DEST;
    machine->ALU >>= 1;
    *machine->DEST = machine->ALU;
    machine->PC += 2;
}

void ASRB(machine_state_t *machine)
{
    setup_dest_addressing(machine);
    machine->MBR = machine->MAR & 077;
    *machine->DEST = ((*machine->DEST & 000377) >> 1) | (*machine->DEST & 0177400);
    machine->PC += 2;
}

void ASL(machine_state_t *machine)
{
    setup_dest_addressing(machine);
    machine->MBR = machine->MAR & 077;
    machine->ALU = *machine->DEST;
    machine->ALU <<= 1;
    *machine->DEST = machine->ALU;
    machine->PC += 2;
}

void ASLB(machine_state_t *machine)
{
    setup_dest_addressing(machine);
    machine->MBR = machine->MAR & 077;
    *machine->DEST = ((*machine->DEST & 000377) << 1) | (*machine->DEST & 0177400);
    machine->PC += 2;
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
