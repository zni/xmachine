		MOV TTYKS, R0
		MOV TTYKB, R1
		MOV TTYPB, R2
		MOV TTYPS, R3
ECHO:	INC (R0)			; Set RDRENB flag

B:		BIT BUSY, (R0)		; Wait for BUSY flag to drop.
		BNE B

R:		TSTB (R3)			; Wait for READY flag to raise.
		BPL R

		MOVB (R1), (R2)		; Move key buffer contents to print buffer.
		BR ECHO
		HALT

TTYKS:	.WORD 777560
TTYKB:	.WORD 777562
TTYPS:	.WORD 777564
TTYPB:	.WORD 777566
BUSY:	.WORD 4000