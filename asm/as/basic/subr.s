# Test out subroutines.

		MOV $0400, sp		# Setup SP
		MOV $4, r0			# Initialize R0
		JSR pc, $ADD4
		JSR pc, $ADD6
		JSR pc, $SUB3
		HALT

ADD4:	ADD $4, r0
		JSR pc, $ADD6
		RTS pc

ADD6:	ADD $6, r0
		JSR pc, $SUB3
		RTS pc

SUB3:	MOV $3, r1
		SUB r1, r0
		RTS pc
