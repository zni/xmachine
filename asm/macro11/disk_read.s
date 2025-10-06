; Execute a disk READ

		MOV RXCS, R0		; Setup pointers.
		MOV RXDB, R1
		MOV RCMD, (R0)		; Issue READ command.

H1:		TSTB (R0)			; Wait for TR flag.
		BPL H1

		MOVB SEC, (R1)		; Write sector.

H2:		TSTB (R0)			; Wait for TR flag.
		BPL H2

		MOVB TRK, (R1)		; Write track.

D:		BIT DONE, (R0)		; Wait for DONE flag.
		BEQ D

		HALT
RXCS:	.WORD 177170
RXDB:	.WORD 177172
RCMD:	.WORD 7
WCMD:	.WORD 5
DONE:	.WORD 40
SEC:	.WORD 5
TRK:    .WORD 7