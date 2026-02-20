# Not sure what the point of this program is.
# It modifies the instructions to be executed until
# it starts running them.
.text
.globl s000
		MOV s000, r0
		MOV s000, r1
		INC r0
		MOV r0, s000
D:		DEC r0
		MOV $0322, (r1)+
		BNE D
		HALT
s000:	.WORD 4
.end
