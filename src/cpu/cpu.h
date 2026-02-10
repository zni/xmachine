#ifndef CPU_H
#define CPU_H

#include <stdint.h>

#include "../common/include/types.h"

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

typedef struct cpu {
    uint16_t r0;
    uint16_t r1;
    uint16_t r2;
    uint16_t r3;
    uint16_t r4;
    uint16_t r5;
    uint16_t sp;
    uint16_t pc;
    uint16_t psw;
    uint16_t alu;
    uint16_t ir;

    uint32_t src_address;
    uint32_t dest_address;

    bool_t halted;
} cpu_t;

void exec_instruction(cpu_t*);

int8_t get_branch_offset(cpu_t*);
void set_negative_flag_w(cpu_t *, uint16_t);

void ADCB(cpu_t *cpu);
void ADC(cpu_t *cpu);
void ADD(cpu_t *cpu);
void ASHC(cpu_t *cpu);
void ASH(cpu_t *cpu);
void ASLB(cpu_t *cpu);
void ASL(cpu_t *cpu);
void ASRB(cpu_t *cpu);
void ASR(cpu_t *cpu);
void BCC(cpu_t *cpu);
void BCS(cpu_t *cpu);
void BEQ(cpu_t *cpu);
void BGE(cpu_t *cpu);
void BGT(cpu_t *cpu);
void BHI(cpu_t *cpu);
void BICB(cpu_t *cpu);
void BIC(cpu_t *cpu);
void BISB(cpu_t *cpu);
void BIS(cpu_t *cpu);
void BITB(cpu_t *cpu);
void BIT(cpu_t *cpu);
void BLE(cpu_t *cpu);
void BLOS(cpu_t *cpu);
void BLT(cpu_t *cpu);
void BMI(cpu_t *cpu);
void BNE(cpu_t *cpu);
void BPL(cpu_t *cpu);
void BR(cpu_t *cpu);
void BVC(cpu_t *cpu);
void BVS(cpu_t *cpu);
void CLRB(cpu_t *cpu);
void CLR(cpu_t *cpu);
void CMPB(cpu_t *cpu);
void CMP(cpu_t *cpu);
void COMB(cpu_t *cpu);
void COM(cpu_t *cpu);
void DECB(cpu_t *cpu);
void DEC(cpu_t *cpu);
void DIV(cpu_t *cpu);
void HALT(cpu_t *cpu);
void INCB(cpu_t *cpu);
void INC(cpu_t *cpu);
void JMP(cpu_t *cpu);
void JSR(cpu_t *cpu);
void MFPD(cpu_t *cpu);
void MFPI(cpu_t *cpu);
void MFPS(cpu_t *cpu);
void MOVB(cpu_t *cpu);
void MOV(cpu_t *cpu);
void MTPD(cpu_t *cpu);
void MTPI(cpu_t *cpu);
void MTPS(cpu_t *cpu);
void MUL(cpu_t *cpu);
void NEGB(cpu_t *cpu);
void NEG(cpu_t *cpu);
void NOP();
void ROLB(cpu_t *cpu);
void ROL(cpu_t *cpu);
void RORB(cpu_t *cpu);
void ROR(cpu_t *cpu);
void RTS(cpu_t *cpu);
void SBCB(cpu_t *cpu);
void SBC(cpu_t *cpu);
void SUB(cpu_t *cpu);
void SWAB(cpu_t *cpu);
void SXT(cpu_t *cpu);
void TSTB(cpu_t *cpu);
void TST(cpu_t *cpu);
void XOR(cpu_t *cpu);

#endif

