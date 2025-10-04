#include "cpu.h"


void init_cpu()
{
    //m_R0 = 0;
    //m_R1 = 0;
    //m_R2 = 0;
    //m_R3 = 0;
    //m_R4 = 0;
    //m_R5 = 0;
    //m_SP = 0;
    //m_PC = 0;
    //m_PSW = 0;
    //m_ALU = 0;
    //m_IR = 0;
    //m_processed = false;
}


//void CPU::send(enum BusMessage t, uint32_t addr, uint16_t data)
//{
//    // printf("CPU::%s(%d, %07o, %07o)\n",
//    //     __FUNCTION__,
//    //     static_cast<std::underlying_type<BusMessage>::type>(t),
//    //     addr,
//    //     data
//    // );
//    m_processed = false;
//    //m_bus_connection->send_bus_message(this, t, addr, data);
//}
//
//void CPU::recv(enum BusMessage t, uint32_t addr, uint16_t data)
//{
//    m_processed = false;
//    process_message(t, addr, data);
//}
//
//uint16_t CPU::bus_id()
//{
//    return 0000001;
//}
//
//void CPU::set_bus(Bus *b)
//{
//    //m_bus_connection = b;
//}

void execute()
{
//    while (!m_bus_connection->halted()) {
//        send(BusMessage::MSYN, 0, 0);
//        send(BusMessage::DATI, m_PC, 0);
//        m_IR = m_recv_data;
//        m_PC += 2;
//        send(BusMessage::CLEAR, 0, 0);
//        exec_instruction();
//        dump();
//    }
}

void dump_cpu()
{
    //wclear(m_window);
    //wprintw(m_window, "ALU: %07o\n", m_ALU);
    //wprintw(m_window, "PSW: %07o\n", m_PSW);
    //wprintw(m_window, "IR : %07o\n", m_IR);
    //wprintw(m_window, "SP : %07o\n", m_SP);
    //wprintw(m_window, "PC : %07o\n", m_PC);
    //wprintw(m_window, "R0 : %07o\n", m_R0);
    //wprintw(m_window, "R1 : %07o\n", m_R1);
    //wprintw(m_window, "R2 : %07o\n", m_R2);
    //wprintw(m_window, "R3 : %07o\n", m_R3);
    //wprintw(m_window, "R4 : %07o\n", m_R4);
    //wprintw(m_window, "R5 : %07o\n", m_R5);
    //wrefresh(m_window);

    //std::cout << "ALU:" << std::oct << m_ALU << std::endl;
    //std::cout << "PSW:" << std::oct << m_PSW << std::endl;
    //std::cout << "IR :" << std::oct << m_IR << std::endl;
    //std::cout << "SP :" << std::oct << m_SP << std::endl;
    //std::cout << "PC :" << std::oct << m_PC << std::endl;
    //std::cout << "R0 :" << std::oct << m_R0 << std::endl;
    //std::cout << "R1 :" << std::oct << m_R1 << std::endl;
    //std::cout << "R2 :" << std::oct << m_R2 << std::endl;
    //std::cout << "R3 :" << std::oct << m_R3 << std::endl;
    //std::cout << "R4 :" << std::oct << m_R4 << std::endl;
    //std::cout << "R5 :" << std::oct << m_R5 << std::endl;
}

//void CPU::set_window(WINDOW *window)
//{
//    m_window = window;
//}

//void CPU::process_message(enum BusMessage t, uint32_t addr, uint16_t data)
//{
//    // printf("CPU::%s(%d, %07o, %07o)\n",
//    //     __FUNCTION__,
//    //     static_cast<std::underlying_type<BusMessage>::type>(t),
//    //     addr,
//    //     data
//    // );
//
//    m_recv_addr = addr;
//    m_recv_data = data;
//    m_processed = true;
//}

void inc_register(uint16_t reg, uint8_t byte)
{
//    uint16_t *r = NULL;
//    switch (reg) {
//        case 0:
//            r = &m_R0;
//            break;
//        case 1:
//            r = &m_R1;
//            break;
//        case 2:
//            r = &m_R2;
//            break;
//        case 3:
//            r = &m_R3;
//            break;
//        case 4:
//            r = &m_R4;
//            break;
//        case 5:
//            r = &m_R5;
//            break;
//        case 6:
//            r = &m_SP;
//            break;
//        case 7:
//            r = &m_PC;
//            break;
//        case 8:
//            r = &m_PSW;
//            break;
//
//        default:
//            return;
//    }
//
//    if (byte) {
//        (*r)++;
//    } else {
//        *r += 2;
//    }
}

void dec_register(uint16_t reg, uint8_t byte)
{
//    uint16_t *r = NULL;
//    switch (reg) {
//        case 0:
//            r = &m_R0;
//            break;
//        case 1:
//            r = &m_R1;
//            break;
//        case 2:
//            r = &m_R2;
//            break;
//        case 3:
//            r = &m_R3;
//            break;
//        case 4:
//            r = &m_R4;
//            break;
//        case 5:
//            r = &m_R5;
//            break;
//        case 6:
//            r = &m_SP;
//            break;
//        case 7:
//            r = &m_PC;
//            break;
//        case 8:
//            r = &m_PSW;
//            break;
//
//        default:
//            return;
//    }
//
//    if (byte) {
//        (*r)--;
//    } else {
//        *r -= 2;
//    }
}

uint16_t fetch_register_contents(uint16_t reg)
{
//    switch (reg) {
//        case 0: return m_R0;
//        case 1: return m_R1;
//        case 2: return m_R2;
//        case 3: return m_R3;
//        case 4: return m_R4;
//        case 5: return m_R5;
//        case 6: return m_SP; // SP
//        case 7: return m_PC; // PC
//        case 8: return m_PSW; // PSW
//
//        default: return 0xffff;
//    }
}

// Convert a high address to a bus address.
uint32_t translate_bus_addr(uint32_t addr)
{
    if (addr & 0177000) {
        addr |= 0700000;
    }

    return addr;
}

uint16_t fetch_data(uint32_t addr)
{
//    if (is_internal_bus_addr(addr)) {
//        return fetch_data_register(addr);
//    }
//
//    addr = translate_bus_addr(addr);
//
//    send(BusMessage::MSYN, 0, 0);
//    send(BusMessage::DATI, addr, 0);
//    send(BusMessage::CLEAR, 0, 0);
//
//    return m_recv_data;
}

uint16_t fetch_data_register(uint32_t addr)
{
//    uint16_t *reg = bus_addr_to_register(addr);
//    return *reg;
}

void store_data(uint32_t addr, uint16_t data)
{
//    if (is_internal_bus_addr(addr)) {
//        store_data_register(addr, data);
//        return;
//    }
//
//    addr = translate_bus_addr(addr);
//
//    send(BusMessage::MSYN, 0, 0);
//    send(BusMessage::DATO, addr, data);
//    send(BusMessage::CLEAR, 0, 0);
}

void store_data_b(uint32_t addr, uint8_t data)
{
//    if (is_internal_bus_addr(addr)) {
//        store_data_register(addr, data);
//        return;
//    }
//
//    addr = translate_bus_addr(addr);
//
//    send(BusMessage::MSYN, 0, 0);
//    send(BusMessage::DATOB, addr, data);
//    send(BusMessage::CLEAR, 0, 0);
}

void store_data_register(uint32_t addr, uint16_t data)
{
//    uint16_t *reg = bus_addr_to_register(addr);
//    *reg = data;
}

void exec_instruction()
{

}
//{
//    switch (m_IR & 0177400) {
//    // Branch OPS
//    case BR_op:
//        BR();
//        break;
//    case BNE_op:
//        BNE();
//        break;
//    case BEQ_op:
//        BEQ();
//        break;
//    case BGE_op:
//        BGE();
//        break;
//    case BLT_op:
//        BLT();
//        break;
//    case BGT_op:
//        BGT();
//        break;
//    case BLE_op:
//        BLE();
//        break;
//    case BPL_op:
//        BPL();
//        break;
//    case BMI_op:
//        BMI();
//        break;
//    case BHI_op:
//        BHI();
//        break;
//    case BLOS_op:
//        BLOS();
//        break;
//    case BVC_op:
//        BVC();
//        break;
//    case BVS_op:
//        BVS();
//        break;
//    case BCC_op:
//        BCC();
//        break;
//    case BCS_op:
//        BCS();
//        break;
//
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
//    }
//}

// XXX Why is this int8_t ?
int8_t get_branch_offset()
{
//    return (int8_t) (m_IR & 0377);
}

// Addressing

uint8_t is_internal_bus_addr(uint32_t addr)
{
//    switch (addr) {
//        case 0777700: return true;
//        case 0777701: return true;
//        case 0777702: return true;
//        case 0777703: return true;
//        case 0777704: return true;
//        case 0777705: return true;
//        case 0777706: return true;
//        case 0777707: return true;
//        case 0777776: return true;
//
//        default: return false;
//    }
}

uint32_t register_to_bus_addr(uint16_t reg)
{
//    switch (reg) {
//        case 0: return 0777700;
//        case 1: return 0777701;
//        case 2: return 0777702;
//        case 3: return 0777703;
//        case 4: return 0777704;
//        case 5: return 0777705;
//        case 6: return 0777706; // SP
//        case 7: return 0777707; // PC
//        case 8: return 0777776; // PSW
//
//        default: return 0xffffffff;
//    }
}

uint16_t* bus_addr_to_register(uint32_t addr)
{
//    switch (addr) {
//        case 0777700: return &m_R0;
//        case 0777701: return &m_R1;
//        case 0777702: return &m_R2;
//        case 0777703: return &m_R3;
//        case 0777704: return &m_R4;
//        case 0777705: return &m_R5;
//        case 0777706: return &m_SP;
//        case 0777707: return &m_PC;
//        case 0777776: return &m_PSW;
//
//        default: return NULL;
//    }
}

void setup_pc_addressing(uint32_t *loc, uint16_t mode, uint16_t reg)
{
//    uint32_t relative = 0;
//    switch (mode) {
//        // From register.
//        case 0:
//            *loc = register_to_bus_addr(reg);
//            break;
//
//        // From immediate.
//        case 2:
//            *loc = m_PC;
//            m_PC += 2;
//            break;
//
//        // From absolute.
//        case 3:
//            *loc = fetch_data(m_PC);
//            m_PC += 2;
//            break;
//
//        // Relative
//        case 6:
//            relative = fetch_data(m_PC);
//            m_PC += 2;
//            *loc = m_PC + relative;
//            break;
//    }
}

void setup_sp_addressing(uint32_t *loc, uint16_t mode, uint16_t reg)
{
//    uint32_t index = 0;
//    uint32_t addr = 0;
//    uint32_t deferred = 0;
//    switch (mode) {
//        // Register: SP
//        case 0:
//            *loc = register_to_bus_addr(reg);
//            break;
//
//        // Deferred: (SP), top of the stack
//        case 1:
//            *loc = m_SP;
//            break;
//
//        // Autoincrement: (Rn)+, get top of stack and then pop it
//        case 2:
//            *loc = m_SP;
//            m_SP += 2;
//            break;
//
//        // Deferred autoincrement: @(SP)+, top of stack is a pointer to a value,
//        // then pop it.
//        case 3:
//            *loc = fetch_data(m_SP);
//            m_SP += 2;
//            break;
//
//        // Autodecrement: -(SP), push value onto stack.
//        case 4:
//            m_SP -= 2;
//            *loc = m_SP;
//            break;
//
//        // Indexed: X(SP), access item X on the stack
//        case 6:
//            index = fetch_data(m_PC);
//            m_PC += 2;
//            *loc = m_SP + index;
//            break;
//
//        // Deferred index: @X(SP), access item pointed to by item X on the stack
//        case 7:
//            index = fetch_data(m_PC);
//            m_PC += 2;
//            *loc = fetch_data(index + m_SP);
//            break;
//    }
}

void setup_general_addressing(uint32_t *loc, uint16_t mode, uint16_t reg)
{
//    uint16_t index = 0;
//    uint32_t deferred = 0;
//    switch (mode) {
//        // Register: Rn
//        case 0:
//            *loc = register_to_bus_addr(reg);
//            break;
//
//        // Deferred: (Rn)
//        case 1:
//            *loc = fetch_register_contents(reg);
//            break;
//
//        // Autoincrement: (Rn)+
//        //
//        // NOTE: (incremented by one or two) to address the next word or
//        // byte depending upon whether the instruction **operates on byte or word data**
//        case 2:
//            *loc = fetch_register_contents(reg);
//            inc_register(reg);
//            break;
//
//        // Deferred autoincrement: @(Rn)+
//        case 3:
//            deferred = fetch_register_contents(reg);
//            index = fetch_data(deferred);
//            inc_register(reg);
//            *loc = fetch_data(index);
//            break;
//
//        // Autodecrement: -(Rn)
//        //
//        // NOTE: (decremented by one or two) to address the next word or
//        // byte depending upon whether the instruction **operates on byte or word data**
//        case 4:
//            *loc = fetch_register_contents(reg);
//            dec_register(reg);
//            break;
//
//        // Deferred autodecrement: @-(Rn)
//        case 5:
//            deferred = fetch_register_contents(reg);
//            index = fetch_data(deferred);
//            *loc = fetch_data(index);
//            index -= 2;
//            store_data(deferred, index);
//            break;
//
//        // Indexed: X(Rn)
//        case 6:
//            index = fetch_data(m_PC);
//            m_PC += 2;
//            deferred = fetch_register_contents(reg);
//            *loc = fetch_data(index + deferred);
//            break;
//
//        // Deferred indexed: @X(Rn)
//        case 7:
//            index = fetch_data(m_PC);
//            m_PC += 2;
//            deferred = fetch_register_contents(reg);
//            deferred = fetch_data(deferred);
//            *loc = fetch_data(deferred + index);
//            break;
//    }
}

void setup_general_byte_addressing(uint32_t *loc, uint16_t mode, uint16_t reg)
{
//    uint16_t index = 0;
//    uint32_t deferred = 0;
//    switch (mode) {
//        // Register: Rn
//        case 0:
//            *loc = register_to_bus_addr(reg);
//            break;
//
//        // Deferred: (Rn)
//        case 1:
//            *loc = fetch_register_contents(reg);
//            break;
//
//        // Autoincrement: (Rn)+
//        //
//        // NOTE: (incremented by one or two) to address the next word or
//        // byte depending upon whether the instruction **operates on byte or word data**
//        case 2:
//            *loc = fetch_register_contents(reg);
//            inc_register(reg, true);
//            break;
//
//        // Deferred autoincrement: @(Rn)+
//        case 3:
//            deferred = fetch_register_contents(reg);
//            index = fetch_data(deferred);
//            inc_register(reg, true);
//            *loc = fetch_data(index);
//            break;
//
//        // Autodecrement: -(Rn)
//        //
//        // NOTE: (decremented by one or two) to address the next word or
//        // byte depending upon whether the instruction **operates on byte or word data**
//        case 4:
//            *loc = fetch_register_contents(reg);
//            dec_register(reg, true);
//            break;
//
//        // Deferred autodecrement: @-(Rn)
//        case 5:
//            deferred = fetch_register_contents(reg);
//            index = fetch_data(deferred);
//            *loc = fetch_data(index);
//            index -= 1;
//            store_data(deferred, index);
//            break;
//
//        // Indexed: X(Rn)
//        case 6:
//            index = fetch_data(m_PC);
//            m_PC += 2;
//            deferred = fetch_register_contents(reg);
//            *loc = fetch_data(index + deferred);
//            break;
//
//        // Deferred indexed: @X(Rn)
//        case 7:
//            index = fetch_data(m_PC);
//            m_PC += 2;
//            deferred = fetch_register_contents(reg);
//            deferred = fetch_data(deferred);
//            *loc = fetch_data(deferred + index);
//            break;
//    }
}

void setup_dest_addressing(uint8_t byte)
{
//    uint16_t reg = (m_IR & 00007);
//    uint16_t mode = (m_IR & 00070) >> 3;
//    switch (reg) {
//        case 7:
//            setup_pc_addressing(&m_dest_address, mode, reg);
//            break;
//        case 6:
//            setup_sp_addressing(&m_dest_address, mode, reg);
//            break;
//        default:
//            if (byte) {
//                setup_general_byte_addressing(&m_dest_address, mode, reg);
//            } else {
//                setup_general_addressing(&m_dest_address, mode, reg);
//            }
//    }
}

void setup_src_addressing(uint8_t byte)
{
//    uint16_t reg = (m_IR & 00700) >> 6;
//    uint16_t mode = (m_IR & 07000) >> 9;
//    switch (reg) {
//        case 7:
//            setup_pc_addressing(&m_src_address, mode, reg);
//            break;
//        case 6:
//            setup_sp_addressing(&m_src_address, mode, reg);
//            break;
//        default:
//            if (byte) {
//                setup_general_byte_addressing(&m_src_address, mode, reg);
//            } else {
//                setup_general_addressing(&m_src_address, mode, reg);
//            }
//    }
}

void set_zero_flag(uint16_t value)
{
//    if (value == 0) {
//        m_PSW |= ZEROFLAG;
//    } else {
//        m_PSW = (m_PSW & ZEROFLAG) ? (m_PSW ^ ZEROFLAG) : m_PSW;
//    }
}

void set_negative_flag_b(uint8_t value)
{
//    uint8_t mask = 0200;
//    if (value & mask) {
//        m_PSW |= NEGATIVEFLAG;
//    } else {
//        m_PSW = (m_PSW & NEGATIVEFLAG) ? (m_PSW ^ NEGATIVEFLAG) : m_PSW;
//    }
}

void set_negative_flag_w(uint16_t value)
{
//    uint16_t mask = 0100000;
//    if (value & mask) {
//        m_PSW |= NEGATIVEFLAG;
//    } else {
//        m_PSW = (m_PSW & NEGATIVEFLAG) ? (m_PSW ^ NEGATIVEFLAG) : m_PSW;
//    }
}

void NOP()
{

}

void BR()
{
//    m_PC = m_PC + (2 * get_branch_offset());
}

void BNE()
{
//    int8_t offset = get_branch_offset();
//    m_ALU = m_PSW & ZEROFLAG;
//    if (m_ALU == 0)
//        m_PC = m_PC + (2 * offset);
}

void BEQ()
{
//    int8_t offset = get_branch_offset();
//    m_ALU = m_PSW & ZEROFLAG;
//    if (m_ALU == ZEROFLAG)
//        m_PC = m_PC + (2 * offset);
}

void BGE()
{
//    int8_t offset = get_branch_offset();
//    m_ALU = (m_PSW & NEGATIVEFLAG) >> 3;
//    m_ALU ^= (m_PSW & OVERFLOWFLAG) >> 1;
//    if (m_ALU == 0)
//        m_PC = m_PC + (2 * offset);
}

void BLT()
{
//    int8_t offset = get_branch_offset();
//    m_ALU = (m_PSW & NEGATIVEFLAG) >> 3;
//    m_ALU ^= (m_PSW & OVERFLOWFLAG) >> 1;
//    if (m_ALU)
//        m_PC = m_PC + (2 * offset);
}

void BGT()
{
//    int8_t offset = get_branch_offset();
//    m_ALU = (m_PSW & ZEROFLAG) >> 2;
//    m_ALU |= ((m_PSW & NEGATIVEFLAG) >> 3) ^ ((m_PSW & OVERFLOWFLAG) >> 1);
//    if (m_ALU == 0)
//        m_PC = m_PC + (2 * offset);
}

void BLE()
{
//    int8_t offset = get_branch_offset();
//    m_ALU = (m_PSW & ZEROFLAG) >> 2;
//    m_ALU |= ((m_PSW & NEGATIVEFLAG) >> 3) ^ ((m_PSW & OVERFLOWFLAG) >> 1);
//    if (m_ALU)
//        m_PC = m_PC + (2 * offset);
}

void BPL()
{
//    int8_t offset = get_branch_offset();
//    m_ALU = m_PSW & NEGATIVEFLAG;
//    if (m_ALU == 0)
//        m_PC = m_PC + (2 * offset);
}

void BMI()
{
//    int8_t offset = get_branch_offset();
//    m_ALU = m_PSW & NEGATIVEFLAG;
//    if (m_ALU)
//        m_PC = m_PC + (2 * offset);
}

void BHI()
{
//    int8_t offset = get_branch_offset();
//    m_ALU = m_PSW & CARRYFLAG;
//    m_ALU |= (m_PSW & ZEROFLAG) >> 2;
//    if (m_ALU == 0)
//        m_PC = m_PC + (2 * offset);
}

void BLOS()
{
//    int8_t offset = get_branch_offset();
//    m_ALU = m_PSW & CARRYFLAG;
//    m_ALU |= (m_PSW & ZEROFLAG) >> 2;
//    if (m_ALU)
//        m_PC = m_PC + (2 * offset);
}

void BVC()
{
//    int8_t offset = get_branch_offset();
//    m_ALU = m_PSW & OVERFLOWFLAG;
//    if (m_ALU == 0)
//        m_PC = m_PC + (2 * offset);
}

void BVS()
{
//    int8_t offset = get_branch_offset();
//    m_ALU = m_PSW & OVERFLOWFLAG;
//    if (m_ALU)
//        m_PC = m_PC + (2 * offset);
}

void BCC()
{
//    int8_t offset = get_branch_offset();
//    m_ALU = m_PSW & CARRYFLAG;
//    if (m_ALU == 0)
//        m_PC = m_PC + (2 * offset);
}

void BCS()
{
//    int8_t offset = get_branch_offset();
//    m_ALU = m_PSW & CARRYFLAG;
//    if (m_ALU)
//        m_PC = m_PC + (2 * offset);
}

void MOV()
{
//    setup_src_addressing();
//    setup_dest_addressing();
//
//    m_ALU = fetch_data(m_src_address);
//    store_data(m_dest_address, m_ALU);
//
//    set_zero_flag(m_ALU);
//    set_negative_flag_w(m_ALU);
}

void MOVB()
{
//    setup_src_addressing(true);
//    setup_dest_addressing(true);
//
//    m_ALU = fetch_data(m_src_address) & 0377;
//    store_data_b(m_dest_address, m_ALU);
//
//    set_zero_flag(m_ALU);
//    set_negative_flag_b(m_ALU);
}

void CMP()
{
//    setup_src_addressing();
//    setup_dest_addressing();
//
//    m_ALU = fetch_data(m_dest_address);
//    m_ALU -= fetch_data(m_src_address);
//
//    set_zero_flag(m_ALU);
//    set_negative_flag_w(m_ALU);
}

void CMPB()
{
//    setup_src_addressing(true);
//    setup_dest_addressing(true);
//
//    m_ALU = fetch_data(m_dest_address) & 0377;
//    m_ALU -= fetch_data(m_src_address) & 0377;
//
//    set_zero_flag(m_ALU);
//    set_negative_flag_b(m_ALU);
}

void BIT()
{
//    setup_src_addressing();
//    setup_dest_addressing();
//
//    m_ALU = fetch_data(m_src_address);
//    m_ALU &= fetch_data(m_dest_address);
//
//    set_zero_flag(m_ALU);
//    set_negative_flag_w(m_ALU);
}

void BITB()
{
//    setup_src_addressing(true);
//    setup_dest_addressing(true);
//
//    m_ALU = fetch_data(m_src_address) & 0377;
//    m_ALU &= fetch_data(m_dest_address) & 0377;
//
//    set_negative_flag_b(m_ALU);
//    set_zero_flag(m_ALU);
}

void BIC()
{
//    setup_src_addressing();
//    setup_dest_addressing();
//
//    m_ALU = fetch_data(m_dest_address);
//    m_ALU &= ~(fetch_data(m_src_address));
//    store_data(m_dest_address, m_ALU);
//
//    set_negative_flag_w(m_ALU);
//    set_zero_flag(m_ALU);
}

void BICB()
{
//    setup_src_addressing(true);
//    setup_dest_addressing(true);
//
//    m_ALU = fetch_data(m_dest_address) & 0377;
//    m_ALU &= ~(fetch_data(m_src_address) & 0377);
//    store_data_b(m_dest_address, m_ALU);
//
//    set_negative_flag_b(m_ALU);
//    set_zero_flag(m_ALU);
}

void BIS()
{
//    setup_src_addressing();
//    setup_dest_addressing();
//
//    m_ALU = fetch_data(m_dest_address);
//    m_ALU |= fetch_data(m_src_address);
//    store_data(m_dest_address, m_ALU);
//
//    set_negative_flag_w(m_ALU);
//    set_zero_flag(m_ALU);
}

void BISB()
{
//    setup_src_addressing(true);
//    setup_dest_addressing(true);
//
//    m_ALU = fetch_data(m_dest_address) & 0377;
//    m_ALU |= fetch_data(m_src_address) & 0377;
//    store_data_b(m_dest_address, m_ALU);
//
//    set_negative_flag_b(m_ALU);
//    set_zero_flag(m_ALU);
}

void ADD()
{
//    setup_src_addressing();
//    setup_dest_addressing();
//
//    m_ALU = fetch_data(m_src_address);
//    m_ALU += fetch_data(m_dest_address);
//    store_data(m_dest_address, m_ALU);
//
//    set_negative_flag_w(m_ALU);
//    set_zero_flag(m_ALU);
}

void SUB()
{
//    setup_src_addressing();
//    setup_dest_addressing();
//
//    m_ALU = fetch_data(m_src_address);
//    m_ALU -= fetch_data(m_dest_address);
//    store_data(m_dest_address, m_ALU);
//
//    set_negative_flag_w(m_ALU);
//    set_zero_flag(m_ALU);
}

void JMP()
{
//    setup_dest_addressing();
//
//    m_PC = fetch_data(m_dest_address);
}

void SWAB()
{
//    setup_dest_addressing();
//
//    m_ALU = fetch_data(m_dest_address);
//    m_ALU = ((m_ALU & 0177400) >> 8) | ((m_ALU & 000377) << 8);
//    store_data(m_dest_address, m_ALU);
//
//    set_negative_flag_w(m_ALU);
//    set_zero_flag(m_ALU);
}

void CLR()
{
//    setup_dest_addressing();
//    m_ALU = 0;
//    store_data(m_dest_address, m_ALU);
//
//    set_negative_flag_w(m_ALU);
//    set_zero_flag(m_ALU);
}

void CLRB()
{
//    setup_dest_addressing(true);
//    m_ALU = 0;
//    store_data_b(m_dest_address, m_ALU);
//
//    set_negative_flag_b(m_ALU);
//    set_zero_flag(m_ALU);
}

void COM()
{
///    setup_dest_addressing();
///
///    m_ALU = ~fetch_data(m_dest_address);
///    store_data(m_dest_address, m_ALU);
///
///    set_negative_flag_w(m_ALU);
///    set_zero_flag(m_ALU);
}

void COMB()
{
//    setup_dest_addressing(true);
//
//    m_ALU = ~(fetch_data(m_dest_address) & 0377);
//    store_data_b(m_dest_address, m_ALU);
//
//    set_negative_flag_b(m_ALU);
//    set_zero_flag(m_ALU);
}

void INC()
{
//    setup_dest_addressing();
//    m_ALU = fetch_data(m_dest_address);
//    m_ALU++;
//    store_data(m_dest_address, m_ALU);
//
//    set_zero_flag(m_ALU);
//    set_negative_flag_w(m_ALU);
}

void INCB()
{
//    setup_dest_addressing(true);
//    m_ALU = fetch_data(m_dest_address) & 0377;
//    m_ALU++;
//    store_data_b(m_dest_address, m_ALU);
//
//    set_zero_flag(m_ALU);
//    set_negative_flag_b(m_ALU);
}

void DEC()
{
//    setup_dest_addressing();
//    m_ALU = fetch_data(m_dest_address);
//    m_ALU--;
//    store_data(m_dest_address, m_ALU);
//
//    set_zero_flag(m_ALU);
//    set_negative_flag_w(m_ALU);
}

void DECB()
{
//    setup_dest_addressing(true);
//
//    m_ALU = fetch_data(m_dest_address) & 0377;
//    m_ALU--;
//    store_data_b(m_dest_address, m_ALU);
//
//    set_zero_flag(m_ALU);
//    set_negative_flag_b(m_ALU);
}

void NEG()
{
//    setup_dest_addressing();
//
//    m_ALU = fetch_data(m_dest_address);
//    m_ALU = ~m_ALU + 1;
//    store_data(m_dest_address, m_ALU);
//
//    set_zero_flag(m_ALU);
//    set_negative_flag_w(m_ALU);
}

void NEGB()
{
//    setup_dest_addressing(true);
//
//    m_ALU = fetch_data(m_dest_address) & 0377;
//    m_ALU = (~m_ALU + 1) & 0377;
//    store_data_b(m_dest_address, m_ALU);
//
//    set_zero_flag(m_ALU);
//    set_negative_flag_b(m_ALU);
}

void ADC()
{
//    setup_dest_addressing();
//
//    m_ALU = fetch_data(m_dest_address);
//    m_ALU += m_PSW & CARRYFLAG;
//    store_data(m_dest_address, m_ALU);
//
//    set_zero_flag(m_ALU);
//    set_negative_flag_w(m_ALU);
}

void ADCB()
{
//    setup_dest_addressing(true);
//
//    m_ALU = fetch_data(m_dest_address) & 0377;
//    m_ALU += m_PSW & CARRYFLAG;
//    store_data_b(m_dest_address, m_ALU);
//
//    set_zero_flag(m_ALU);
//    set_negative_flag_b(m_ALU);
}

void SBC()
{
//    setup_dest_addressing();
//
//    m_ALU = fetch_data(m_dest_address);
//    m_ALU -= (m_PSW & CARRYFLAG);
//    store_data(m_dest_address, m_ALU);
//
//    set_zero_flag(m_ALU);
//    set_negative_flag_w(m_ALU);
}

void SBCB()
{
//    setup_dest_addressing(true);
//
//    m_ALU = fetch_data(m_dest_address) & 0377;
//    m_ALU -= (m_PSW & CARRYFLAG);
//    store_data_b(m_dest_address, m_ALU);
//
//    set_zero_flag(m_ALU);
//    set_negative_flag_b(m_ALU);
}

void TST()
{
//    setup_dest_addressing();
//
//    m_ALU = fetch_data(m_dest_address);
//
//    set_zero_flag(m_ALU);
//    set_negative_flag_w(m_ALU);
}

void TSTB()
{
//    setup_dest_addressing(true);
//
//    m_ALU = fetch_data(m_dest_address) & 0377;
//
//    set_zero_flag(m_ALU);
//    set_negative_flag_b(m_ALU);
}

void ROR()
{
//    setup_dest_addressing();
//
//    m_ALU = fetch_data(m_dest_address);
//    m_ALU = (m_ALU << 1) | (m_ALU >> 15);
//    store_data(m_dest_address, m_ALU);
//
//    set_zero_flag(m_ALU);
//    set_negative_flag_w(m_ALU);
}

void RORB()
{
//    setup_dest_addressing(true);
//
//    m_ALU = fetch_data(m_dest_address) & 0377;
//    m_ALU = (m_ALU << 1) | (m_ALU >> 7);
//    store_data_b(m_dest_address, m_ALU);
//
//    set_zero_flag(m_ALU);
//    set_negative_flag_b(m_ALU);
}

void ROL()
{
//    setup_dest_addressing();
//
//    m_ALU = fetch_data(m_dest_address);
//    m_ALU = (m_ALU << 15) | (m_ALU >> 1);
//    store_data(m_dest_address, m_ALU);
//
//    set_zero_flag(m_ALU);
//    set_negative_flag_w(m_ALU);
}

void ROLB()
{
//    setup_dest_addressing(true);
//
//    m_ALU = fetch_data(m_dest_address) & 0377;
//    m_ALU = (m_ALU << 7) | (m_ALU >> 1);
//    store_data_b(m_dest_address, m_ALU);
//
//    set_zero_flag(m_ALU);
//    set_negative_flag_b(m_ALU);
}

void ASR()
{
//    setup_dest_addressing();
//
//    m_ALU = fetch_data(m_dest_address);
//    m_ALU >>= 1;
//    store_data(m_dest_address, m_ALU);
//
//    set_zero_flag(m_ALU);
//    set_negative_flag_w(m_ALU);
}

void ASRB()
{
//    setup_dest_addressing();
//
//    m_ALU = fetch_data(m_dest_address) & 0377;
//    m_ALU >>= 1;
//    store_data_b(m_dest_address, m_ALU);
//
//    set_zero_flag(m_ALU);
//    set_negative_flag_b(m_ALU);
}

void ASL()
{
//    setup_dest_addressing();
//
//    m_ALU = fetch_data(m_dest_address);
//    m_ALU <<= 1;
//    store_data(m_dest_address, m_ALU);
//
//    set_zero_flag(m_ALU);
//    set_negative_flag_w(m_ALU);
}

void ASLB()
{
//    setup_dest_addressing(true);
//
//    m_ALU = fetch_data(m_dest_address) & 0377;
//    m_ALU <<= 1;
//    store_data(m_dest_address, m_ALU);
//
//    set_zero_flag(m_ALU);
//    set_negative_flag_b(m_ALU);
}

void MTPS()
{
//    setup_dest_addressing();
//    m_ALU = fetch_data(m_dest_address);
//    m_PSW = m_ALU;
}

void MFPI()
{
    NOP();
}

void MFPD()
{
    NOP();
}

void MTPI()
{
    NOP();
}

void MTPD()
{
    NOP();
}

void SXT()
{
    NOP();
}

void MFPS()
{
//    setup_dest_addressing();
//    m_ALU = m_PSW;
//    store_data(m_dest_address, m_ALU);
}

void MUL()
{
    NOP();
}

void DIV()
{
    NOP();
}

void ASH()
{
    NOP();
}

void ASHC()
{
    NOP();
}

void XOR()
{
    NOP();
}

void JSR()
{
//    uint32_t reg = (m_IR & 0700) >> 6;
//    setup_dest_addressing();
//
//
//    uint16_t tmp = m_dest_address;
//    uint16_t reg_contents = fetch_register_contents(reg);
//
//    store_data(m_SP, reg_contents);
//    dec_register(m_SP);
//
//    store_data_register(register_to_bus_addr(reg), m_PC);
//    m_PC = tmp;
}

void RTS()
{
//    uint32_t reg = m_IR & 07;
//    uint16_t reg_contents = fetch_register_contents(reg);
//    m_PC = reg_contents;
//
//    inc_register(m_SP);
//    uint16_t top_stack = fetch_data(m_SP);
//    store_data_register(register_to_bus_addr(reg), top_stack);
}

void HALT()
{
//    m_bus_connection->set_halt(true);
}

int main(int argc, char **argv)
{
    return 0;
}

