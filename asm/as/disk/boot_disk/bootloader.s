# Load data from sector 0 track 0 on disk into memory.
# Then jump to it after loading.

		MOV RXCS, r0		# Setup pointers.
		MOV RXDB, r1
		MOV TTYPB, r3
		MOV TTYPS, r4
		MOV RCMD, (r0)		# Issue READ command.

H1:		TSTB (r0)			# Wait for TR flag.
		BPL H1

		MOVB SEC, (r1)		# Write sector.

H2:		TSTB (r0)			# Wait for TR flag.
		BPL H2

		MOVB TRK, (r1)		# Write track.

D:		BIT DONE, (r0)		# Wait for DONE flag.
		BEQ D

		MOV LDOFF, r2
		MOV ECMD, (r0)

H3:		TSTB (r0)			# Wait for TR flag
		BPL H3

		MOVB (r1), (r2)+
		MOV DOT, (r3)
PR:		TSTB (r4)			# Wait for READY flag.
		BPL PR

D2:		BIT DONE, (r0)
		BEQ H3

		MOV $DNE, r0
PR2:	MOVB (r0)+, (r3)
		BEQ J
		BR PR2
J:		JMP LDOFF

		HALT
RXCS:	.WORD 0177170
RXDB:	.WORD 0177172
TTYPS:	.WORD 0777564
TTYPB:	.WORD 0777566
RCMD:	.WORD 7
WCMD:	.WORD 5
ECMD:	.WORD 3
DONE:	.WORD 040
SEC:	.WORD 0
TRK:    .WORD 0
DOT:	.WORD 052
LDOFF:	.WORD 0600
DNE:	.ASCIZ /DONE/<12>
