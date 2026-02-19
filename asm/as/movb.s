# Test out MOVB.
.text
.globl s000
	MOV $07777, r0
	MOVB r0, r1
	MOVB r1, $s000
	MOV s000, r2
	HALT

s000: .word 0
.end
