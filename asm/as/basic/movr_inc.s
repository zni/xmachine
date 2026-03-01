# Test indirection with increment.
.text
		MOV s000, r0
		MOV $s000, r1
		INC r0
		MOV r0, s000
D:		MOV $0322, (r1)+
		DEC r0
		BNE D
		HALT
s000:	.WORD 4
.end
