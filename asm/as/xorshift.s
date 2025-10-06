	MOV $OUT, r3
	MOV $012, r4

	MOV @$LFSR, r0
	MOV @$LFSR, r1
#	NEG r0

L:	ASR r1
	ASR r1
	ASR r1
	XOR r0, r1

	ASL r1
	ASL r1
	ASL r1
	ASL r1
	ASL r1
	XOR r0, r1

	ASR r1
	ASR r1
	ASR r1
	ASR r1
	ASR r1
	ASR r1
	ASR r1
	XOR r0, r1
	MOV r1, (r3)+
	MOV r1, r0
	DEC r4
	BNE L
	HALT


LFSR:   .WORD 067
OUT:    .WORD 0

