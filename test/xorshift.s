.TITLE XORSHIFT

	MOV #OUT, R3
	MOV #12, R4

	MOV @#LFSR, R0
	MOV @#LFSR, R1
	;NEG R0

L:	ASR R1
	ASR R1
	ASR R1
	XOR R0, R1

	ASL R1
	ASL R1
	ASL R1
	ASL R1
	ASL R1
	XOR R0, R1

	ASR R1
	ASR R1
	ASR R1
	ASR R1
	ASR R1
	ASR R1
	ASR R1
	XOR R0, R1
	MOV R1, (R3)+
	MOV R1, R0
	DEC R4
	BNE L
	HALT


LFSR:   .WORD 067
OUT:    .WORD 0