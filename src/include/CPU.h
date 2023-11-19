#ifndef CPU_H
#define CPU_H

#include <cstdint>

#include "Bus.h"
#include "BusMessage.h"
#include "IBusElement.h"

enum ProcessorStatusFlags {
    CARRYFLAG    = 0000001,
    OVERFLOWFLAG = 0000002,
    ZEROFLAG     = 0000004,
    NEGATIVEFLAG = 0000010,
    TRAPFLAG     = 0000020,
    IPLBITS      = 0000340,
    PREVMODE     = 0030000,
    CURRMODE     = 0140000
};

enum OPCode {
    // Branch OPs
    BR_op   = 0000400,
    BNE_op  = 0001000,
    BEQ_op  = 0001400,
    BGE_op  = 0002000,
    BLT_op  = 0002400,
    BGT_op  = 0003000,
    BLE_op  = 0003400,
    BPL_op  = 0100000,
    BMI_op  = 0100400,
    BHI_op  = 0101000,
    BLOS_op = 0101400,
    BVC_op  = 0102000,
    BVS_op  = 0102400,
    BCC_op  = 0103000,
    BCS_op  = 0103400,

    // Double OP Register Source
    MUL_op  = 0070000,
    DIV_op  = 0071000,
    ASH_op  = 0072000,
    ASHC_op = 0073000,
    XOR_op  = 0074000,

    // Subroutine
    JSR_op  = 0004000,
    RTS_op  = 0000200,

    // Double OPs
    MOV_op  = 0010000,
    MOVB_op = 0110000,
    CMP_op  = 0020000,
    CMPB_op = 0120000,
    BIT_op  = 0030000,
    BITB_op = 0130000,
    BIC_op  = 0040000,
    BICB_op = 0140000,
    BIS_op  = 0050000,
    BISB_op = 0150000,
    ADD_op  = 0060000,
    SUB_op  = 0160000,

    // Single OPs
    JMP_op  = 0000100,
    SWAB_op = 0000300,
    CLR_op  = 0005000,
    CLRB_op = 0105000,
    COM_op  = 0005100,
    COMB_op = 0105100,
    INC_op  = 0005200,
    INCB_op = 0105200,
    DEC_op  = 0005300,
    DECB_op = 0105300,
    NEG_op  = 0005400,
    NEGB_op = 0105400,
    ADC_op  = 0005500,
    ADCB_op = 0105500,
    SBC_op  = 0005600,
    SBCB_op = 0105600,
    TST_op  = 0005700,
    TSTB_op = 0105700,
    ROR_op  = 0006000,
    RORB_op = 0106000,
    ROL_op  = 0006100,
    ROLB_op = 0106100,
    ASR_op  = 0006200,
    ASRB_op = 0106200,
    ASL_op  = 0006300,
    ASLB_op = 0106300,
    MTPS_op = 0106400,
    MFPI_op = 0006500,
    MFPD_op = 0106500,
    MTPI_op = 0006600,
    MTPD_op = 0106600,
    SXT_op  = 0006700,
    MFPS_op = 0106700
};

class CPU : public IBusElement
{
    public:
        CPU();
        ~CPU();

        void send(enum BusMessage, uint32_t, uint16_t);
        void recv(enum BusMessage, uint32_t, uint16_t);
        uint16_t bus_id();
        void set_bus(Bus *);
        void set_step_mode(bool);

        void execute();

        void dump();

    private:
        void process_message(enum BusMessage, uint32_t, uint16_t);

        void inc_register(uint16_t, bool byte = false);
        void dec_register(uint16_t, bool byte = false);
        uint16_t fetch_register_contents(uint16_t);
        uint16_t fetch_data(uint32_t);
        uint16_t fetch_data_register(uint32_t);
        void store_data(uint32_t, uint16_t);
        void store_data_b(uint32_t, uint8_t);
        void store_data_register(uint32_t, uint16_t);

        void exec_instruction();

        int8_t get_branch_offset();

        bool is_internal_bus_addr(uint32_t);
        uint32_t register_to_bus_addr(uint16_t);
        uint16_t* bus_addr_to_register(uint32_t);
        void setup_pc_addressing(uint32_t *, uint16_t, uint16_t);
        void setup_sp_addressing(uint32_t *, uint16_t, uint16_t);
        void setup_general_addressing(uint32_t*, uint16_t, uint16_t);
        void setup_general_byte_addressing(uint32_t*, uint16_t, uint16_t);

        void setup_dest_addressing(bool byte = false);
        void setup_src_addressing(bool byte = false);

        void set_zero_flag(uint16_t);
        void set_negative_flag_w(uint16_t);
        void set_negative_flag_b(uint8_t);


        void NOP();
        void BR();
        void BNE();
        void BEQ();
        void BGE();
        void BLT();
        void BGT();
        void BLE();
        void BPL();
        void BMI();
        void BHI();
        void BLOS();
        void BVC();
        void BVS();
        void BCC();
        void BCS();

        void MOV();
        void MOVB();
        void CMP();
        void CMPB();
        void BIT();
        void BITB();
        void BIC();
        void BICB();
        void BIS();
        void BISB();
        void ADD();
        void SUB();

        void JMP();
        void SWAB();
        void CLR();
        void CLRB();
        void COM();
        void COMB();
        void INC();
        void INCB();
        void DEC();
        void DECB();
        void NEG();
        void NEGB();
        void ADC();
        void ADCB();
        void SBC();
        void SBCB();
        void TST();
        void TSTB();
        void ROR();
        void RORB();
        void ROL();
        void ROLB();
        void ASR();
        void ASRB();
        void ASL();
        void ASLB();
        void MTPS();
        void MFPI();
        void MFPD();
        void MTPI();
        void MTPD();
        void SXT();
        void MFPS();

        void MUL();
        void DIV();
        void ASH();
        void ASHC();
        void XOR();

        void JSR();
        void RTS();

        void HALT();

        uint16_t m_R0;
        uint16_t m_R1;
        uint16_t m_R2;
        uint16_t m_R3;
        uint16_t m_R4;
        uint16_t m_R5;
        uint16_t m_SP;
        uint16_t m_PC;
        uint16_t m_PSW;
        uint16_t m_ALU;
        uint16_t m_IR;

        uint32_t m_src_address;
        uint32_t m_dest_address;

        uint32_t m_recv_addr;
        uint16_t m_recv_data;

        bool m_single_step;

        bool m_processed;

        Bus *m_bus_connection;
};

#endif