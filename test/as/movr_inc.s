		MOV s000, r0
		MOV $s000, r1
		INC r0
		MOV r0, s000
D:		DEC r0
		MOV r0, (r1)+
		BNE D
		HALT
s000:	.WORD 4
