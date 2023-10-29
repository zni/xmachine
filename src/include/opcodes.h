#ifndef OPCODES_H
#define OPCODES_H


enum op_codes {
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
#endif