; Load data from sector 0 track 0 on disk into memory.
; Then jump to it after loading.

		MOV RXCS, R0		; Setup pointers.
		MOV RXDB, R1
		MOV TTYPB, R3
		MOV TTYPS, R4
		MOV RCMD, (R0)		; Issue READ command.

H1:		TSTB (R0)			; Wait for TR flag.
		BPL H1

		MOVB SEC, (R1)		; Write sector.

H2:		TSTB (R0)			; Wait for TR flag.
		BPL H2

		MOVB TRK, (R1)		; Write track.

D:		BIT DONE, (R0)		; Wait for DONE flag.
		BEQ D

		MOV LDOFF, R2
		MOV ECMD, (R0)

H3:		TSTB (R0)			; Wait for TR flag
		BPL H3

		MOVB (R1), (R2)+
		MOV DOT, (R3)
PR:		TSTB (R4)			; Wait for READY flag.
		BPL PR

D2:		BIT DONE, (R0)
		BEQ H3

		MOV #DNE, R0
PR2:	MOVB (R0)+, (R3)
		BEQ J
		BR PR2
J:		JMP LDOFF

		HALT
RXCS:	.WORD 177170
RXDB:	.WORD 177172
TTYPS:	.WORD 777564
TTYPB:	.WORD 777566
RCMD:	.WORD 7
WCMD:	.WORD 5
ECMD:	.WORD 3
DONE:	.WORD 40
SEC:	.WORD 0
TRK:    .WORD 0
DOT:	.WORD 52
LDOFF:	.WORD 600
DNE:	.ASCIZ /DONE/<12>
