#ifndef HANDLERS_H
#define HANDLERS_H

#include <stddef.h>
#include "machine_state.h"

void BR(machine_state_t*);
void BNE(machine_state_t*);
void BEQ(machine_state_t*);
void BGE(machine_state_t*);
void BLT(machine_state_t*);
void BGT(machine_state_t*);
void BLE(machine_state_t*);
void BPL(machine_state_t*);
void BMI(machine_state_t*);
void BHI(machine_state_t*);
void BLOS(machine_state_t*);
void BVC(machine_state_t*);
void BVS(machine_state_t*);
void BCC(machine_state_t*);
void BCS(machine_state_t*);

void MOV(machine_state_t*);
void MOVB(machine_state_t*);
void CMP(machine_state_t*);
void CMPB(machine_state_t*);
void BIT(machine_state_t*);
void BITB(machine_state_t*);
void BIC(machine_state_t*);
void BICB(machine_state_t*);
void BIS(machine_state_t*);
void BISB(machine_state_t*);
void ADD(machine_state_t*);
void SUB(machine_state_t*);

void JMP(machine_state_t*);
void SWAB(machine_state_t*);
void CLR(machine_state_t*);
void CLRB(machine_state_t*);
void COM(machine_state_t*);
void COMB(machine_state_t*);
void INC(machine_state_t*);
void INCB(machine_state_t*);
void DEC(machine_state_t*);
void DECB(machine_state_t*);
void NEG(machine_state_t*);
void NEGB(machine_state_t*);
void ADC(machine_state_t*);
void ADCB(machine_state_t*);
void SBC(machine_state_t*);
void SBCB(machine_state_t*);
void TST(machine_state_t*);
void TSTB(machine_state_t*);
void ROR(machine_state_t*);
void RORB(machine_state_t*);
void ROL(machine_state_t*);
void ROLB(machine_state_t*);
void ASR(machine_state_t*);
void ASRB(machine_state_t*);
void ASL(machine_state_t*);
void ASLB(machine_state_t*);
void MTPS(machine_state_t*);
void MFPI(machine_state_t*);
void MFPD(machine_state_t*);
void MTPI(machine_state_t*);
void MTPD(machine_state_t*);
void SXT(machine_state_t*);
void MFPS(machine_state_t*);

void MUL(machine_state_t*);
void DIV(machine_state_t*);
void ASH(machine_state_t*);
void ASHC(machine_state_t*);
void XOR(machine_state_t*);

void HALT(machine_state_t*);

void exec_instruction(machine_state_t*);

#endif