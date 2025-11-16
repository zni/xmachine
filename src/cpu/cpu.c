#include "cpu.h"

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "../libunibus/device_bus_mgr.h"

cpu_t *CPU_STATE = NULL;
bus_state_t *BUS_STATE = NULL;

bool_t is_internal_bus_addr(uint32_t);
uint32_t translate_bus_addr(uint32_t);
uint16_t* bus_addr_to_register(cpu_t*, uint32_t);
uint16_t fetch_data(cpu_t*, uint32_t);
uint16_t fetch_data_register(cpu_t*, uint32_t);
void store_data_register(cpu_t*, uint32_t, uint16_t);
void store_data(cpu_t*, uint32_t, uint16_t);
void store_data_b(cpu_t*, uint32_t, uint8_t);

void
usage()
{
    fprintf(stderr, "cpu -p <offset>\n");
}

cpu_t*
init_cpu()
{
    cpu_t *cpu = malloc(sizeof(cpu_t));
    if (cpu == NULL) {
        perror("cpu");
        return NULL;
    }

    cpu->r0 = 0;
    cpu->r1 = 0;
    cpu->r2 = 0;
    cpu->r3 = 0;
    cpu->r4 = 0;
    cpu->r5 = 0;
    cpu->sp = 0;
    cpu->pc = 0;
    cpu->psw = 0;
    cpu->alu = 0;
    cpu->ir = 0;

    cpu->src_address = 0;
    cpu->dest_address = 0;

    cpu->halted = FALSE;

    return cpu;
}

void
execute(cpu_t *cpu)
{
    while (!cpu->halted) {
        cpu->ir = fetch_data(cpu, cpu->pc);
        cpu->pc += 2; // advance by word.
        exec_instruction(cpu);
    }
}

void
dump_cpu(cpu_t *cpu)
{
    printf("ALU: %07o\n", cpu->alu);
    printf("PSW: %07o\n", cpu->psw);
    printf("IR : %07o\n", cpu->ir);
    printf("SP : %07o\n", cpu->sp);
    printf("PC : %07o\n", cpu->pc);
    printf("R0 : %07o\n", cpu->r0);
    printf("R1 : %07o\n", cpu->r1);
    printf("R2 : %07o\n", cpu->r2);
    printf("R3 : %07o\n", cpu->r3);
    printf("R4 : %07o\n", cpu->r4);
    printf("R5 : %07o\n", cpu->r5);
}

void
process_message(/*enum BusMessage t,*/ cpu_t *cpu,  uint32_t addr, uint16_t data)
{
//    // printf("CPU::%s(%d, %07o, %07o)\n",
//    //     __FUNCTION__,
//    //     static_cast<std::underlying_type<BusMessage>::type>(t),
//    //     addr,
//    //     data
//    // );
//
    //m_recv_addr = addr;
    //m_recv_data = data;
    //m_processed = TRUE;
}

void
inc_register(cpu_t *cpu, uint16_t reg, uint8_t is_byte_addr)
{
    uint16_t *r = NULL;
    switch (reg) {
        case 0:
            r = &(cpu->r0);
            break;
        case 1:
            r = &(cpu->r1);
            break;
        case 2:
            r = &(cpu->r2);
            break;
        case 3:
            r = &(cpu->r3);
            break;
        case 4:
            r = &(cpu->r4);
            break;
        case 5:
            r = &(cpu->r5);
            break;
        case 6:
            r = &(cpu->sp);
            break;
        case 7:
            r = &(cpu->pc);
            break;
        case 8:
            r = &(cpu->psw);
            break;

        default:
            return;
    }

    if (is_byte_addr) {
        (*r)++;
    } else {
        *r += 2;
    }
}

void
dec_register(cpu_t *cpu, uint16_t reg, uint8_t is_byte_addr)
{
    uint16_t *r = NULL;
    switch (reg) {
        case 0:
            r = &(cpu->r0);
            break;
        case 1:
            r = &(cpu->r1);
            break;
        case 2:
            r = &(cpu->r2);
            break;
        case 3:
            r = &(cpu->r3);
            break;
        case 4:
            r = &(cpu->r4);
            break;
        case 5:
            r = &(cpu->r5);
            break;
        case 6:
            r = &(cpu->sp);
            break;
        case 7:
            r = &(cpu->pc);
            break;
        case 8:
            r = &(cpu->psw);
            break;

        default:
            return;
    }

    if (is_byte_addr) {
        (*r)--;
    } else {
        *r -= 2;
    }
}

uint16_t
fetch_register_contents(cpu_t *cpu, uint16_t reg)
{
    switch (reg) {
        case 0: return cpu->r0;
        case 1: return cpu->r1;
        case 2: return cpu->r2;
        case 3: return cpu->r3;
        case 4: return cpu->r4;
        case 5: return cpu->r5;
        case 6: return cpu->sp;
        case 7: return cpu->pc;
        case 8: return cpu->psw;

        default: return 0xffff;
    }
}

// Convert a high address to a bus address.
uint32_t
translate_bus_addr(uint32_t addr)
{
    if (addr & 0177000) {
        addr |= 0700000;
    }

    return addr;
}

uint16_t
fetch_data(cpu_t *cpu, uint32_t addr)
{
    uint16_t data;
    if (is_internal_bus_addr(addr)) {
        data = fetch_data_register(cpu, addr);
        return data;
    }

    addr = translate_bus_addr(addr);

    // Request bus master, if necessary.
    req_bus_master(BUS_STATE);

    // Block for read.
    data = read_data_in(BUS_STATE, addr);

    release_bus_master(BUS_STATE);

    return data;
}

uint16_t
fetch_data_register(cpu_t *cpu, uint32_t addr)
{
    uint16_t *reg = bus_addr_to_register(cpu, addr);
    return *reg;
}

void
store_data(cpu_t *cpu, uint32_t addr, uint16_t data)
{
    if (is_internal_bus_addr(addr)) {
        store_data_register(cpu, addr, data);
        return;
    }

    addr = translate_bus_addr(addr);

    // Request bus master, if necessary.
    req_bus_master(BUS_STATE);

    // Block for write.
    write_data_out(BUS_STATE, addr, data);

    release_bus_master(BUS_STATE);

// XXX REMOVE
//    send(BusMessage::MSYN, 0, 0);
//    send(BusMessage::DATO, addr, data);
//    send(BusMessage::CLEAR, 0, 0);
}

void
store_data_b(cpu_t *cpu, uint32_t addr, uint8_t data)
{
    if (is_internal_bus_addr(addr)) {
        store_data_register(cpu, addr, data);
        return;
    }

    addr = translate_bus_addr(addr);

// FIXME
//    send(BusMessage::MSYN, 0, 0);
//    send(BusMessage::DATOB, addr, data);
//    send(BusMessage::CLEAR, 0, 0);
}

void store_data_register(cpu_t *cpu, uint32_t addr, uint16_t data)
{
    uint16_t *reg = bus_addr_to_register(cpu, addr);
    *reg = data;
}

void exec_instruction(cpu_t *cpu)
{
    switch (cpu->ir & 0177400) {
    // Branch OPS
    case BR_op:
        BR(cpu);
        break;
    case BNE_op:
        BNE(cpu);
        break;
    case BEQ_op:
        BEQ(cpu);
        break;
    case BGE_op:
        BGE(cpu);
        break;
    case BLT_op:
        BLT(cpu);
        break;
    case BGT_op:
        BGT(cpu);
        break;
    case BLE_op:
        BLE(cpu);
        break;
    case BPL_op:
        BPL(cpu);
        break;
    case BMI_op:
        BMI(cpu);
        break;
    case BHI_op:
        BHI(cpu);
        break;
    case BLOS_op:
        BLOS(cpu);
        break;
    case BVC_op:
        BVC(cpu);
        break;
    case BVS_op:
        BVS(cpu);
        break;
    case BCC_op:
        BCC(cpu);
        break;
    case BCS_op:
        BCS(cpu);
        break;

//    default:
//        // Double OP Register Source and JSR
//        switch (m_IR & 0177000) {
//        case MUL_op:
//            MUL();
//            break;
//        case DIV_op:
//            DIV();
//            break;
//        case ASH_op:
//            ASH();
//            break;
//        case ASHC_op:
//            ASHC();
//            break;
//        case XOR_op:
//            XOR();
//            break;
//        case JSR_op:
//            JSR();
//            break;
//        default:
//            switch (m_IR & 0170000) {
//            // Double OPs
//            case MOV_op:
//                MOV();
//                break;
//            case MOVB_op:
//                MOVB();
//                break;
//            case CMP_op:
//                CMP();
//                break;
//            case CMPB_op:
//                CMPB();
//                break;
//            case BIT_op:
//                BIT();
//                break;
//            case BITB_op:
//                BITB();
//                break;
//            case BIC_op:
//                BIC();
//                break;
//            case BICB_op:
//                BICB();
//                break;
//            case BIS_op:
//                BIS();
//                break;
//            case BISB_op:
//                BISB();
//                break;
//            case ADD_op:
//                ADD();
//                break;
//            case SUB_op:
//                SUB();
//                break;
//            default:
//                // Single OPs
//                switch (m_IR & 0177700) {
//                case JMP_op:
//                    JMP();
//                    break;
//                case SWAB_op:
//                    SWAB();
//                    break;
//                case CLR_op:
//                    CLR();
//                    break;
//                case CLRB_op:
//                    CLRB();
//                    break;
//                case COM_op:
//                    COM();
//                    break;
//                case COMB_op:
//                    COMB();
//                    break;
//                case INC_op:
//                    INC();
//                    break;
//                case INCB_op:
//                    INCB();
//                    break;
//                case DEC_op:
//                    DEC();
//                    break;
//                case DECB_op:
//                    DECB();
//                    break;
//                case NEG_op:
//                    NEG();
//                    break;
//                case NEGB_op:
//                    NEGB();
//                    break;
//                case ADC_op:
//                    ADC();
//                    break;
//                case ADCB_op:
//                    ADCB();
//                    break;
//                case SBC_op:
//                    SBC();
//                    break;
//                case SBCB_op:
//                    SBCB();
//                    break;
//                case TST_op:
//                    TST();
//                    break;
//                case TSTB_op:
//                    TSTB();
//                    break;
//                case ROR_op:
//                    ROR();
//                    break;
//                case RORB_op:
//                    RORB();
//                    break;
//                case ROL_op:
//                    ROL();
//                    break;
//                case ROLB_op:
//                    ROLB();
//                    break;
//                case ASR_op:
//                    ASR();
//                    break;
//                case ASRB_op:
//                    ASRB();
//                    break;
//                case ASL_op:
//                    ASL();
//                    break;
//                case ASLB_op:
//                    ASLB();
//                    break;
//                case MTPS_op:
//                    MTPS();
//                    break;
//                case MFPI_op:
//                    MFPI();
//                    break;
//                case MFPD_op:
//                    MFPD();
//                    break;
//                case MTPI_op:
//                    MTPI();
//                    break;
//                case MTPD_op:
//                    MTPD();
//                    break;
//                case SXT_op:
//                    SXT();
//                    break;
//                case MFPS_op:
//                    MFPS();
//                    break;
//                case RTS_op:
//                    RTS();
//                    break;
//                default:
//                    HALT();
//                    break;
//                }
//            }
//        }
    }
}

int8_t
get_branch_offset(cpu_t *cpu)
{
    return (int8_t) (cpu->ir & 0377);
}

/*******************************
 * Addressing
 *******************************/

bool_t
is_internal_bus_addr(uint32_t addr)
{
    switch (addr) {
        case 0777700: return TRUE;
        case 0777701: return TRUE;
        case 0777702: return TRUE;
        case 0777703: return TRUE;
        case 0777704: return TRUE;
        case 0777705: return TRUE;
        case 0777706: return TRUE;
        case 0777707: return TRUE;
        case 0777776: return TRUE;

        default: return FALSE;
    }
}

uint32_t
register_to_bus_addr(uint16_t reg)
{
    switch (reg) {
        case 0: return 0777700;
        case 1: return 0777701;
        case 2: return 0777702;
        case 3: return 0777703;
        case 4: return 0777704;
        case 5: return 0777705;
        case 6: return 0777706; // SP
        case 7: return 0777707; // PC
        case 8: return 0777776; // PSW

        default: return 0xffffffff;
    }
}

uint16_t*
bus_addr_to_register(cpu_t *cpu, uint32_t addr)
{
    switch (addr) {
        case 0777700: return &cpu->r0;
        case 0777701: return &cpu->r1;
        case 0777702: return &cpu->r2;
        case 0777703: return &cpu->r3;
        case 0777704: return &cpu->r4;
        case 0777705: return &cpu->r5;
        case 0777706: return &cpu->sp;
        case 0777707: return &cpu->pc;
        case 0777776: return &cpu->psw;

        default: return NULL;
    }
}

void
setup_pc_addressing(cpu_t *cpu, uint32_t *loc, uint16_t mode, uint16_t reg)
{
    uint32_t relative = 0;
    switch (mode) {
        // From register.
        case 0:
            *loc = register_to_bus_addr(reg);
            break;

        // From immediate.
        case 2:
            *loc = cpu->pc;
            cpu->pc += 2;
            break;

        // From absolute.
        case 3:
            *loc = fetch_data(cpu, cpu->pc);
            cpu->pc += 2;
            break;

        // Relative
        case 6:
            relative = fetch_data(cpu, cpu->pc);
            cpu->pc += 2;
            *loc = cpu->pc + relative;
            break;
    }
}

void
setup_sp_addressing(cpu_t *cpu, uint32_t *loc, uint16_t mode, uint16_t reg)
{
    uint32_t index = 0;
    uint32_t addr = 0;
    uint32_t deferred = 0;
    switch (mode) {
        // Register: SP
        case 0:
            *loc = register_to_bus_addr(reg);
            break;

        // Deferred: (SP), top of the stack
        case 1:
            *loc = cpu->sp;
            break;

        // Autoincrement: (Rn)+, get top of stack and then pop it
        case 2:
            *loc = cpu->sp;
            cpu->sp += 2;
            break;

        // Deferred autoincrement: @(SP)+, top of stack is a pointer to a value,
        // then pop it.
        case 3:
            *loc = fetch_data(cpu, cpu->sp);
            cpu->sp += 2;
            break;

        // Autodecrement: -(SP), push value onto stack.
        case 4:
            cpu->sp -= 2;
            *loc = cpu->sp;
            break;

        // Indexed: X(SP), access item X on the stack
        case 6:
            index = fetch_data(cpu, cpu->pc);
            cpu->pc += 2;
            *loc = cpu->sp + index;
            break;

        // Deferred index: @X(SP), access item pointed to by item X on the stack
        case 7:
            index = fetch_data(cpu, cpu->pc);
            cpu->pc += 2;
            *loc = fetch_data(cpu, index + cpu->sp);
            break;
    }
}

void
setup_general_addressing(cpu_t *cpu, uint32_t *loc, uint16_t mode, uint16_t reg)
{
    uint16_t index = 0;
    uint32_t deferred = 0;
    switch (mode) {
        // Register: Rn
        case 0:
            *loc = register_to_bus_addr(reg);
            break;

        // Deferred: (Rn)
        case 1:
            *loc = fetch_register_contents(cpu, reg);
            break;

        // Autoincrement: (Rn)+
        //
        // NOTE: (incremented by one or two) to address the next word or
        // byte depending upon whether the instruction **operates on byte or word data**
        case 2:
            *loc = fetch_register_contents(cpu, reg);
            inc_register(cpu, reg, FALSE);
            break;

        // Deferred autoincrement: @(Rn)+
        case 3:
            deferred = fetch_register_contents(cpu, reg);
            index = fetch_data(cpu, deferred);
            inc_register(cpu, reg, FALSE);
            *loc = fetch_data(cpu, index);
            break;

        // Autodecrement: -(Rn)
        //
        // NOTE: (decremented by one or two) to address the next word or
        // byte depending upon whether the instruction **operates on byte or word data**
        case 4:
            *loc = fetch_register_contents(cpu, reg);
            dec_register(cpu, reg, FALSE);
            break;

        // Deferred autodecrement: @-(Rn)
        case 5:
            deferred = fetch_register_contents(cpu, reg);
            index = fetch_data(cpu, deferred);
            *loc = fetch_data(cpu, index);
            index -= 2;
            store_data(cpu, deferred, index);
            break;

        // Indexed: X(Rn)
        case 6:
            index = fetch_data(cpu, cpu->pc);
            cpu->pc += 2;
            deferred = fetch_register_contents(cpu, reg);
            *loc = fetch_data(cpu, index + deferred);
            break;

        // Deferred indexed: @X(Rn)
        case 7:
            index = fetch_data(cpu, cpu->pc);
            cpu->pc += 2;
            deferred = fetch_register_contents(cpu, reg);
            deferred = fetch_data(cpu, deferred);
            *loc = fetch_data(cpu, deferred + index);
            break;
    }
}

void
setup_general_byte_addressing(cpu_t *cpu, uint32_t *loc, uint16_t mode, uint16_t reg)
{
    uint16_t index = 0;
    uint32_t deferred = 0;
    switch (mode) {
        // Register: Rn
        case 0:
            *loc = register_to_bus_addr(reg);
            break;

        // Deferred: (Rn)
        case 1:
            *loc = fetch_register_contents(cpu, reg);
            break;

        // Autoincrement: (Rn)+
        //
        // NOTE: (incremented by one or two) to address the next word or
        // byte depending upon whether the instruction **operates on byte or word data**
        case 2:
            *loc = fetch_register_contents(cpu, reg);
            inc_register(cpu, reg, TRUE);
            break;

        // Deferred autoincrement: @(Rn)+
        case 3:
            deferred = fetch_register_contents(cpu, reg);
            index = fetch_data(cpu, deferred);
            inc_register(cpu, reg, TRUE);
            *loc = fetch_data(cpu, index);
            break;

        // Autodecrement: -(Rn)
        //
        // NOTE: (decremented by one or two) to address the next word or
        // byte depending upon whether the instruction **operates on byte or word data**
        case 4:
            *loc = fetch_register_contents(cpu, reg);
            dec_register(cpu, reg, TRUE);
            break;

        // Deferred autodecrement: @-(Rn)
        case 5:
            deferred = fetch_register_contents(cpu, reg);
            index = fetch_data(cpu, deferred);
            *loc = fetch_data(cpu, index);
            index -= 1;
            store_data(cpu, deferred, index);
            break;

        // Indexed: X(Rn)
        case 6:
            index = fetch_data(cpu, cpu->pc);
            cpu->pc += 2;
            deferred = fetch_register_contents(cpu, reg);
            *loc = fetch_data(cpu, index + deferred);
            break;

        // Deferred indexed: @X(Rn)
        case 7:
            index = fetch_data(cpu, cpu->pc);
            cpu->pc += 2;
            deferred = fetch_register_contents(cpu, reg);
            deferred = fetch_data(cpu, deferred);
            *loc = fetch_data(cpu, deferred + index);
            break;
    }
}

void
setup_dest_addressing(cpu_t *cpu, uint8_t use_byte_addr)
{
    uint16_t reg = (cpu->ir & 00007);
    uint16_t mode = (cpu->ir & 00070) >> 3;
    switch (reg) {
        case 7:
            setup_pc_addressing(cpu, &(cpu->dest_address), mode, reg);
            break;
        case 6:
            setup_sp_addressing(cpu, &(cpu->dest_address), mode, reg);
            break;
        default:
            if (use_byte_addr) {
                setup_general_byte_addressing(cpu, &(cpu->dest_address), mode, reg);
            } else {
                setup_general_addressing(cpu, &(cpu->dest_address), mode, reg);
            }
    }
}

void
setup_src_addressing(cpu_t *cpu, uint8_t use_byte_addr)
{
    uint16_t reg = (cpu->ir & 00700) >> 6;
    uint16_t mode = (cpu->ir & 07000) >> 9;
    switch (reg) {
        case 7:
            setup_pc_addressing(cpu, &(cpu->src_address), mode, reg);
            break;
        case 6:
            setup_sp_addressing(cpu, &(cpu->src_address), mode, reg);
            break;
        default:
            if (use_byte_addr) {
                setup_general_byte_addressing(cpu, &(cpu->src_address), mode, reg);
            } else {
                setup_general_addressing(cpu, &(cpu->src_address), mode, reg);
            }
    }
}

void
set_zero_flag(cpu_t *cpu, uint16_t value)
{
    if (value == 0) {
        cpu->psw |= ZEROFLAG;
    } else {
        cpu->psw = (cpu->psw & ZEROFLAG) ? (cpu->psw ^ ZEROFLAG) : cpu->psw;
    }
}

void
set_negative_flag_b(cpu_t *cpu, uint8_t value)
{
    uint8_t mask = 0200;
    if (value & mask) {
        cpu->psw |= NEGATIVEFLAG;
    } else {
        cpu->psw = (cpu->psw & NEGATIVEFLAG) ? (cpu->psw ^ NEGATIVEFLAG) : cpu->psw;
    }
}

void
set_negative_flag_w(cpu_t *cpu, uint16_t value)
{
    uint16_t mask = 0100000;
    if (value & mask) {
        cpu->psw |= NEGATIVEFLAG;
    } else {
        cpu->psw = (cpu->psw & NEGATIVEFLAG) ? (cpu->psw ^ NEGATIVEFLAG) : cpu->psw;
    }
}

void
NOP()
{
    return;
}

void
BR(cpu_t *cpu)
{
    cpu->pc = cpu->pc + (2 * get_branch_offset(cpu));
}

void
BNE(cpu_t *cpu)
{
    int8_t offset = get_branch_offset(cpu);
    cpu->alu = cpu->psw & ZEROFLAG;
    if (cpu->alu == 0)
        cpu->pc = cpu->pc + (2 * offset);
}

void
BEQ(cpu_t *cpu)
{
    int8_t offset = get_branch_offset(cpu);
    cpu->alu = cpu->psw & ZEROFLAG;
    if (cpu->alu == ZEROFLAG)
        cpu->pc = cpu->pc + (2 * offset);
}

void
BGE(cpu_t *cpu)
{
    int8_t offset = get_branch_offset(cpu);
    cpu->alu = (cpu->psw & NEGATIVEFLAG) >> 3;
    cpu->alu ^= (cpu->psw & OVERFLOWFLAG) >> 1;
    if (cpu->alu == 0)
        cpu->pc = cpu->pc + (2 * offset);
}

void
BLT(cpu_t *cpu)
{
    int8_t offset = get_branch_offset(cpu);
    cpu->alu = (cpu->psw & NEGATIVEFLAG) >> 3;
    cpu->alu ^= (cpu->psw & OVERFLOWFLAG) >> 1;
    if (cpu->alu)
        cpu->pc = cpu->pc + (2 * offset);
}

void
BGT(cpu_t *cpu)
{
    int8_t offset = get_branch_offset(cpu);
    cpu->alu = (cpu->psw & ZEROFLAG) >> 2;
    cpu->alu |= ((cpu->psw & NEGATIVEFLAG) >> 3) ^ ((cpu->psw & OVERFLOWFLAG) >> 1);
    if (cpu->alu == 0)
        cpu->pc = cpu->pc + (2 * offset);
}

void
BLE(cpu_t *cpu)
{
    int8_t offset = get_branch_offset(cpu);
    cpu->alu = (cpu->psw & ZEROFLAG) >> 2;
    cpu->alu |= ((cpu->psw & NEGATIVEFLAG) >> 3) ^ ((cpu->psw & OVERFLOWFLAG) >> 1);
    if (cpu->alu)
        cpu->pc = cpu->pc + (2 * offset);
}

void
BPL(cpu_t *cpu)
{
    int8_t offset = get_branch_offset(cpu);
    cpu->alu = cpu->psw & NEGATIVEFLAG;
    if (cpu->alu == 0)
        cpu->pc = cpu->pc + (2 * offset);
}

void
BMI(cpu_t *cpu)
{
    int8_t offset = get_branch_offset(cpu);
    cpu->alu = cpu->psw & NEGATIVEFLAG;
    if (cpu->alu)
        cpu->pc = cpu->pc + (2 * offset);
}

void
BHI(cpu_t *cpu)
{
    int8_t offset = get_branch_offset(cpu);
    cpu->alu = cpu->psw & CARRYFLAG;
    cpu->alu |= (cpu->psw & ZEROFLAG) >> 2;
    if (cpu->alu == 0)
        cpu->pc = cpu->pc + (2 * offset);
}

void
BLOS(cpu_t *cpu)
{
    int8_t offset = get_branch_offset(cpu);
    cpu->alu = cpu->psw & CARRYFLAG;
    cpu->alu |= (cpu->psw & ZEROFLAG) >> 2;
    if (cpu->alu)
        cpu->pc = cpu->pc + (2 * offset);
}

void BVC(cpu_t *cpu)
{
    int8_t offset = get_branch_offset(cpu);
    cpu->alu = cpu->psw & OVERFLOWFLAG;
    if (cpu->alu == 0)
        cpu->pc = cpu->pc + (2 * offset);
}

void BVS(cpu_t *cpu)
{
    int8_t offset = get_branch_offset(cpu);
    cpu->alu = cpu->psw & OVERFLOWFLAG;
    if (cpu->alu)
        cpu->pc = cpu->pc + (2 * offset);
}

void BCC(cpu_t *cpu)
{
    int8_t offset = get_branch_offset(cpu);
    cpu->alu = cpu->psw & CARRYFLAG;
    if (cpu->alu == 0)
        cpu->pc = cpu->pc + (2 * offset);
}

void BCS(cpu_t *cpu)
{
    int8_t offset = get_branch_offset(cpu);
    cpu->alu = cpu->psw & CARRYFLAG;
    if (cpu->alu)
        cpu->pc = cpu->pc + (2 * offset);
}

void
MOV(cpu_t *cpu)
{
    setup_src_addressing(cpu, FALSE);
    setup_dest_addressing(cpu, FALSE);

    cpu->alu = fetch_data(cpu, cpu->src_address);
    store_data(cpu, cpu->dest_address, cpu->alu);

    set_zero_flag(cpu, cpu->alu);
    set_negative_flag_w(cpu, cpu->alu);
}

void
MOVB(cpu_t *cpu)
{
    setup_src_addressing(cpu, TRUE);
    setup_dest_addressing(cpu, TRUE);

    cpu->alu = fetch_data(cpu, cpu->src_address) & 0377;
    store_data_b(cpu, cpu->dest_address, cpu->alu);

    set_zero_flag(cpu, cpu->alu);
    set_negative_flag_b(cpu, cpu->alu);
}

void
CMP(cpu_t *cpu)
{
    setup_src_addressing(cpu, FALSE);
    setup_dest_addressing(cpu, FALSE);

    cpu->alu = fetch_data(cpu, cpu->dest_address);
    cpu->alu -= fetch_data(cpu, cpu->src_address);

    set_zero_flag(cpu, cpu->alu);
    set_negative_flag_w(cpu, cpu->alu);
}

void
CMPB(cpu_t *cpu)
{
    setup_src_addressing(cpu, TRUE);
    setup_dest_addressing(cpu, TRUE);

    cpu->alu = fetch_data(cpu, cpu->dest_address) & 0377;
    cpu->alu -= fetch_data(cpu, cpu->src_address) & 0377;

    set_zero_flag(cpu, cpu->alu);
    set_negative_flag_b(cpu, cpu->alu);
}

void
BIT(cpu_t *cpu)
{
    setup_src_addressing(cpu, FALSE);
    setup_dest_addressing(cpu, FALSE);

    cpu->alu = fetch_data(cpu, cpu->src_address);
    cpu->alu &= fetch_data(cpu, cpu->dest_address);

    set_zero_flag(cpu, cpu->alu);
    set_negative_flag_w(cpu, cpu->alu);
}

void
BITB(cpu_t *cpu)
{
    setup_src_addressing(cpu, TRUE);
    setup_dest_addressing(cpu, TRUE);

    cpu->alu = fetch_data(cpu, cpu->src_address) & 0377;
    cpu->alu &= fetch_data(cpu, cpu->dest_address) & 0377;

    set_negative_flag_b(cpu, cpu->alu);
    set_zero_flag(cpu, cpu->alu);
}

void
BIC(cpu_t *cpu)
{
    setup_src_addressing(cpu, FALSE);
    setup_dest_addressing(cpu, FALSE);

    cpu->alu = fetch_data(cpu, cpu->dest_address);
    cpu->alu &= ~(fetch_data(cpu, cpu->src_address));
    store_data(cpu, cpu->dest_address, cpu->alu);

    set_negative_flag_w(cpu, cpu->alu);
    set_zero_flag(cpu, cpu->alu);
}

void
BICB(cpu_t *cpu)
{
    setup_src_addressing(cpu, TRUE);
    setup_dest_addressing(cpu, TRUE);

    cpu->alu = fetch_data(cpu, cpu->dest_address) & 0377;
    cpu->alu &= ~(fetch_data(cpu, cpu->src_address) & 0377);
    store_data_b(cpu, cpu->dest_address, cpu->alu);

    set_negative_flag_b(cpu, cpu->alu);
    set_zero_flag(cpu, cpu->alu);
}

void
BIS(cpu_t *cpu)
{
    setup_src_addressing(cpu, FALSE);
    setup_dest_addressing(cpu, FALSE);

    cpu->alu = fetch_data(cpu, cpu->dest_address);
    cpu->alu |= fetch_data(cpu, cpu->src_address);
    store_data(cpu, cpu->dest_address, cpu->alu);

    set_negative_flag_w(cpu, cpu->alu);
    set_zero_flag(cpu, cpu->alu);
}

void
BISB(cpu_t *cpu)
{
    setup_src_addressing(cpu, TRUE);
    setup_dest_addressing(cpu, TRUE);

    cpu->alu = fetch_data(cpu, cpu->dest_address) & 0377;
    cpu->alu |= fetch_data(cpu, cpu->src_address) & 0377;
    store_data_b(cpu, cpu->dest_address, cpu->alu);

    set_negative_flag_b(cpu, cpu->alu);
    set_zero_flag(cpu, cpu->alu);
}

void
ADD(cpu_t *cpu)
{
    setup_src_addressing(cpu, FALSE);
    setup_dest_addressing(cpu, FALSE);

    cpu->alu = fetch_data(cpu, cpu->src_address);
    cpu->alu += fetch_data(cpu, cpu->dest_address);
    store_data(cpu, cpu->dest_address, cpu->alu);

    set_negative_flag_w(cpu, cpu->alu);
    set_zero_flag(cpu, cpu->alu);
}

void
SUB(cpu_t *cpu)
{
    setup_src_addressing(cpu, FALSE);
    setup_dest_addressing(cpu, FALSE);

    cpu->alu = fetch_data(cpu, cpu->src_address);
    cpu->alu -= fetch_data(cpu, cpu->dest_address);
    store_data(cpu, cpu->dest_address, cpu->alu);

    set_negative_flag_w(cpu, cpu->alu);
    set_zero_flag(cpu, cpu->alu);
}

void
JMP(cpu_t *cpu)
{
    setup_dest_addressing(cpu, FALSE);
    cpu->pc = fetch_data(cpu, cpu->dest_address);
}

void
SWAB(cpu_t *cpu)
{
    setup_dest_addressing(cpu, FALSE);

    cpu->alu = fetch_data(cpu, cpu->dest_address);
    cpu->alu = ((cpu->alu & 0177400) >> 8) | ((cpu->alu & 000377) << 8);
    store_data(cpu, cpu->dest_address, cpu->alu);

    set_negative_flag_w(cpu, cpu->alu);
    set_zero_flag(cpu, cpu->alu);
}

void
CLR(cpu_t *cpu)
{
    setup_dest_addressing(cpu, FALSE);
    cpu->alu = 0;
    store_data(cpu, cpu->dest_address, cpu->alu);

    set_negative_flag_w(cpu, cpu->alu);
    set_zero_flag(cpu, cpu->alu);
}

void
CLRB(cpu_t *cpu)
{
    setup_dest_addressing(cpu, TRUE);
    cpu->alu = 0;
    store_data_b(cpu, cpu->dest_address, cpu->alu);

    set_negative_flag_b(cpu, cpu->alu);
    set_zero_flag(cpu, cpu->alu);
}

void
COM(cpu_t *cpu)
{
    setup_dest_addressing(cpu, FALSE);

    cpu->alu = ~fetch_data(cpu, cpu->dest_address);
    store_data(cpu, cpu->dest_address, cpu->alu);

    set_negative_flag_w(cpu, cpu->alu);
    set_zero_flag(cpu, cpu->alu);
}

void
COMB(cpu_t *cpu)
{
    setup_dest_addressing(cpu, FALSE);

    cpu->alu = ~(fetch_data(cpu, cpu->dest_address) & 0377);
    store_data_b(cpu, cpu->dest_address, cpu->alu);

    set_negative_flag_b(cpu, cpu->alu);
    set_zero_flag(cpu, cpu->alu);
}

void
INC(cpu_t *cpu)
{
    setup_dest_addressing(cpu, FALSE);
    cpu->alu = fetch_data(cpu, cpu->dest_address);
    cpu->alu++;
    store_data(cpu, cpu->dest_address, cpu->alu);

    set_zero_flag(cpu, cpu->alu);
    set_negative_flag_w(cpu, cpu->alu);
}

void
INCB(cpu_t *cpu)
{
    setup_dest_addressing(cpu, TRUE);
    cpu->alu = fetch_data(cpu, cpu->dest_address) & 0377;
    cpu->alu++;
    store_data_b(cpu, cpu->dest_address, cpu->alu);

    set_zero_flag(cpu, cpu->alu);
    set_negative_flag_b(cpu, cpu->alu);
}

void
DEC(cpu_t *cpu)
{
    setup_dest_addressing(cpu, FALSE);
    cpu->alu = fetch_data(cpu, cpu->dest_address);
    cpu->alu--;
    store_data(cpu, cpu->dest_address, cpu->alu);

    set_zero_flag(cpu, cpu->alu);
    set_negative_flag_w(cpu, cpu->alu);
}

void
DECB(cpu_t *cpu)
{
    setup_dest_addressing(cpu, TRUE);

    cpu->alu = fetch_data(cpu, cpu->dest_address) & 0377;
    cpu->alu--;
    store_data_b(cpu, cpu->dest_address, cpu->alu);

    set_zero_flag(cpu, cpu->alu);
    set_negative_flag_b(cpu, cpu->alu);
}

void
NEG(cpu_t *cpu)
{
    setup_dest_addressing(cpu, FALSE);

    cpu->alu = fetch_data(cpu, cpu->dest_address);
    cpu->alu = ~cpu->alu + 1;
    store_data(cpu, cpu->dest_address, cpu->alu);

    set_zero_flag(cpu, cpu->alu);
    set_negative_flag_w(cpu, cpu->alu);
}

void
NEGB(cpu_t *cpu)
{
    setup_dest_addressing(cpu, TRUE);

    cpu->alu = fetch_data(cpu, cpu->dest_address) & 0377;
    cpu->alu = (~cpu->alu + 1) & 0377;
    store_data_b(cpu, cpu->dest_address, cpu->alu);

    set_zero_flag(cpu, cpu->alu);
    set_negative_flag_b(cpu, cpu->alu);
}

void
ADC(cpu_t *cpu)
{
    setup_dest_addressing(cpu, FALSE);

    cpu->alu = fetch_data(cpu, cpu->dest_address);
    cpu->alu += cpu->psw & CARRYFLAG;
    store_data(cpu, cpu->dest_address, cpu->alu);

    set_zero_flag(cpu, cpu->alu);
    set_negative_flag_w(cpu, cpu->alu);
}

void
ADCB(cpu_t *cpu)
{
    setup_dest_addressing(cpu, TRUE);

    cpu->alu = fetch_data(cpu, cpu->dest_address) & 0377;
    cpu->alu += cpu->psw & CARRYFLAG;
    store_data_b(cpu, cpu->dest_address, cpu->alu);

    set_zero_flag(cpu, cpu->alu);
    set_negative_flag_b(cpu, cpu->alu);
}

void
SBC(cpu_t *cpu)
{
    setup_dest_addressing(cpu, FALSE);

    cpu->alu = fetch_data(cpu, cpu->dest_address);
    cpu->alu -= (cpu->psw & CARRYFLAG);
    store_data(cpu, cpu->dest_address, cpu->alu);

    set_zero_flag(cpu, cpu->alu);
    set_negative_flag_w(cpu, cpu->alu);
}

void
SBCB(cpu_t *cpu)
{
    setup_dest_addressing(cpu, TRUE);

    cpu->alu = fetch_data(cpu, cpu->dest_address) & 0377;
    cpu->alu -= (cpu->psw & CARRYFLAG);
    store_data_b(cpu, cpu->dest_address, cpu->alu);

    set_zero_flag(cpu, cpu->alu);
    set_negative_flag_b(cpu, cpu->alu);
}

void
TST(cpu_t *cpu)
{
    setup_dest_addressing(cpu, FALSE);

    cpu->alu = fetch_data(cpu, cpu->dest_address);

    set_zero_flag(cpu, cpu->alu);
    set_negative_flag_w(cpu, cpu->alu);
}

void
TSTB(cpu_t *cpu)
{
    setup_dest_addressing(cpu, TRUE);

    cpu->alu = fetch_data(cpu, cpu->dest_address) & 0377;

    set_zero_flag(cpu, cpu->alu);
    set_negative_flag_b(cpu, cpu->alu);
}

void
ROR(cpu_t *cpu)
{
    setup_dest_addressing(cpu, FALSE);

    cpu->alu = fetch_data(cpu, cpu->dest_address);
    cpu->alu = (cpu->alu << 1) | (cpu->alu >> 15);
    store_data(cpu, cpu->dest_address, cpu->alu);

    set_zero_flag(cpu, cpu->alu);
    set_negative_flag_w(cpu, cpu->alu);
}

void
RORB(cpu_t *cpu)
{
    setup_dest_addressing(cpu, TRUE);

    cpu->alu = fetch_data(cpu, cpu->dest_address) & 0377;
    cpu->alu = (cpu->alu << 1) | (cpu->alu >> 7);
    store_data_b(cpu, cpu->dest_address, cpu->alu);

    set_zero_flag(cpu, cpu->alu);
    set_negative_flag_b(cpu, cpu->alu);
}

void
ROL(cpu_t *cpu)
{
    setup_dest_addressing(cpu, FALSE);

    cpu->alu = fetch_data(cpu, cpu->dest_address);
    cpu->alu = (cpu->alu << 15) | (cpu->alu >> 1);
    store_data(cpu, cpu->dest_address, cpu->alu);

    set_zero_flag(cpu, cpu->alu);
    set_negative_flag_w(cpu, cpu->alu);
}

void
ROLB(cpu_t *cpu)
{
    setup_dest_addressing(cpu, TRUE);

    cpu->alu = fetch_data(cpu, cpu->dest_address) & 0377;
    cpu->alu = (cpu->alu << 7) | (cpu->alu >> 1);
    store_data_b(cpu, cpu->dest_address, cpu->alu);

    set_zero_flag(cpu, cpu->alu);
    set_negative_flag_b(cpu, cpu->alu);
}

void
ASR(cpu_t *cpu)
{
    setup_dest_addressing(cpu, FALSE);

    cpu->alu = fetch_data(cpu, cpu->dest_address);
    cpu->alu >>= 1;
    store_data(cpu, cpu->dest_address, cpu->alu);

    set_zero_flag(cpu, cpu->alu);
    set_negative_flag_w(cpu, cpu->alu);
}

void
ASRB(cpu_t *cpu)
{
    setup_dest_addressing(cpu, TRUE);

    // FIXME? fetch_data_b?
    cpu->alu = fetch_data(cpu, cpu->dest_address) & 0377;
    cpu->alu >>= 1;
    store_data_b(cpu, cpu->dest_address, cpu->alu);

    set_zero_flag(cpu, cpu->alu);
    set_negative_flag_b(cpu, cpu->alu);
}

void
ASL(cpu_t *cpu)
{
    setup_dest_addressing(cpu, FALSE);

    cpu->alu = fetch_data(cpu, cpu->dest_address);
    cpu->alu <<= 1;
    store_data(cpu, cpu->dest_address, cpu->alu);

    set_zero_flag(cpu, cpu->alu);
    set_negative_flag_w(cpu, cpu->alu);
}

void
ASLB(cpu_t *cpu)
{
    setup_dest_addressing(cpu, TRUE);

    cpu->alu = fetch_data(cpu, cpu->dest_address) & 0377;
    cpu->alu <<= 1;
    store_data(cpu, cpu->dest_address, cpu->alu);

    set_zero_flag(cpu, cpu->alu);
    set_negative_flag_b(cpu, cpu->alu);
}

void
MTPS(cpu_t *cpu)
{
    setup_dest_addressing(cpu, FALSE);
    cpu->alu = fetch_data(cpu, cpu->dest_address);
    cpu->psw = cpu->alu;
}

void
MFPI(cpu_t *cpu)
{
    NOP();
}

void
MFPD(cpu_t *cpu)
{
    NOP();
}

void
MTPI(cpu_t *cpu)
{
    NOP();
}

void
MTPD(cpu_t *cpu)
{
    NOP();
}

void
SXT(cpu_t *cpu)
{
    NOP();
}

void
MFPS(cpu_t *cpu)
{
    setup_dest_addressing(cpu, FALSE);
    cpu->alu = cpu->psw;
    store_data(cpu, cpu->dest_address, cpu->alu);
}

void
MUL(cpu_t *cpu)
{
    NOP();
}

void
DIV(cpu_t *cpu)
{
    NOP();
}

void
ASH(cpu_t *cpu)
{
    NOP();
}

void
ASHC(cpu_t *cpu)
{
    NOP();
}

void
XOR(cpu_t *cpu)
{
    NOP();
}

void
JSR(cpu_t *cpu)
{
    uint32_t reg = (cpu->ir & 0700) >> 6;
     setup_dest_addressing(cpu, FALSE);

    uint16_t tmp = cpu->dest_address;
    uint16_t reg_contents = fetch_register_contents(cpu, reg);

    store_data(cpu, cpu->sp, reg_contents);
    dec_register(cpu, cpu->sp, FALSE);

    store_data_register(cpu, register_to_bus_addr(reg), cpu->pc);
    cpu->pc = tmp;
}

void
RTS(cpu_t *cpu)
{
    uint32_t reg = cpu->ir & 07;
    uint16_t reg_contents = fetch_register_contents(cpu, reg);
    cpu->pc = reg_contents;

    inc_register(cpu, cpu->sp, FALSE);
    uint16_t top_stack = fetch_data(cpu, cpu->sp);
    store_data_register(cpu, register_to_bus_addr(reg), top_stack);
}

void
HALT(cpu_t *cpu)
{
    cpu->halted = TRUE;
}

void
handler(int signo, siginfo_t *info, void *context)
{
    if (CPU_STATE != NULL) {
        free(CPU_STATE);
    }

    /* Signal and join bus threads. */
    cleanup_bus(BUS_STATE);

    exit(EXIT_SUCCESS);
}

int main(int argc, char **argv)
{
    bus_state_t *bus;
    cpu_t *cpu;
    int ret, opt;
    struct sigaction act = { 0 };
    char sock_l[] = "ba";
    char sock_name[] = "cpu";
    char sock_r[] = "mem";

    /* Setup signal handler. */
    act.sa_flags = SA_SIGINFO;
    act.sa_sigaction = &handler;
    if (sigaction(SIGHUP, &act, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGINT, &act, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    pid_t pid = getpid();
    fprintf(stderr, "cpu is starting [%d]\n", pid);

    /* Initialize CPU. */
    cpu = init_cpu();
    if (cpu == NULL) {
        fprintf(stderr, "failed to initialize CPU\n");
        exit(EXIT_FAILURE);
    }
    CPU_STATE = cpu;

    while ((opt = getopt(argc, argv, "p:")) != -1) {
        switch (opt) {
        case 'p':
            cpu->pc = strtoul(optarg, NULL, 10);
            break;
        default:
            usage();
            free(cpu);
            exit(EXIT_FAILURE);
        }
    }

    /* Initialize bus connections. */
    bus = init_bus(sock_l, sock_name, sock_r);
    if (bus == NULL) {
        free(cpu);
        fprintf(stderr, "failed to initialize bus\n");
        exit(EXIT_FAILURE);
    }
    BUS_STATE = bus;

    /* Attempt to connect to bus. */
    ret = connect_bus(bus);
    if (ret != 0) {
        free(cpu);
        free(bus);
        fprintf(stderr, "failed to initialize bus connections\n");
        exit(EXIT_FAILURE);
    }

    execute(cpu);

    return 0;
}

