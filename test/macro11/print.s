		MOV #HELLO, R0
		MOV TTYPB, R2
		MOV TTYPS, R3

LOOP:	MOVB (R0)+, (R2)
		BEQ H

R:		TSTB (R3)			; Wait for READY flag.
		BPL R

		BR LOOP

H:		HALT

TTYKS:	.WORD 777560
TTYKB:	.WORD 777562
TTYPS:	.WORD 777564
TTYPB:	.WORD 777566
HELLO: 	.ASCIZ /HELLO GOODBYE/<12>