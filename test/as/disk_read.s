# Execute a disk READ

		MOV RXCS, r0		# Setup pointers.
		MOV RXDB, r1
		MOV RCMD, (r0)		# Issue READ command.

H1:		TSTB (r0)			# Wait for TR flag.
		BPL H1

		MOVB SEC, (r1)		# Write sector.

H2:		TSTB (r0)			# Wait for TR flag.
		BPL H2

		MOVB TRK, (r1)		# Write track.

D:		BIT DONE, (r0)		# Wait for DONE flag.
		BEQ D

		HALT

RXCS:	.WORD 0177170
RXDB:	.WORD 0177172
RCMD:	.WORD 7
WCMD:	.WORD 5
DONE:	.WORD 040
SEC:	.WORD 5
TRK:    .WORD 7
