# Memory value to register.
# Increase.
# Register to memory location.
	MOV s000, r0
	INC r0
	MOV r0, s000
	HALT

	s000: .WORD 04
.end
