#ifndef MACHINE_H
#define MACHINE_H

#define MEMSIZE 4096

#define IPLBITS         0340
#define TRAPFLAG        0020
#define NEGATIVEFLAG    0010
#define ZEROFLAG        0004
#define OVERFLOWFLAG    0002
#define CARRYFLAG       0001

enum spec_double_op {
    MUL  = 070,
    DIV  = 071,
    ASH  = 072,
    ASHC = 073,
    XOR  = 074
};
typedef enum spec_double_op spec_double_op_t;

enum double_op {
    MOV  = 01,
    MOVB = 011,
    CMP  = 02,
    CMPB = 012,
    BIT  = 03,
    BITB = 013,
    BIC  = 04,
    BICB = 014,
    BIS  = 05,
    BISB = 015,
    ADD  = 06,
    SUB  = 016
};
typedef enum double_op double_op_t;

enum single_op {
    JMP  = 00001,
    SWAB = 00003,
    CLR  = 00050,
    CLRB = 01050,
    COM  = 00051,
    COMB = 01051,
    INC  = 00052,
    INCB = 01052,
    DEC  = 00053,
    DECB = 01053,
    NEG  = 00054,
    NEGB = 01054,
    ADC  = 00055,
    ADCB = 01055,
    SBC  = 00056,
    SBCB = 01056,
    TST  = 00057,
    TSTB = 01057,
    ROR  = 00060,
    RORB = 01060,
    ROL  = 00061,
    ROLB = 01061,
    ASR  = 00062,
    ASRB = 01062,
    ASL  = 00063,
    ASLB = 01063,
    MTPS = 01064,
    MFPI = 00065,
    MFPD = 01065,
    MTPI = 00066,
    MTPD = 01066,
    SXT  = 00067,
    MFPS = 01067
};
typedef enum single_op single_op_t;
#endif