.text
		MOV TTYKS, r0
		MOV TTYKB, r1
		MOV TTYPB, r2
		MOV TTYPS, r3
ECHO:	INC (r0)			# Set RDRENB flag

B:		TSTB (r0)			# Wait for DONE flag to raise.
		BPL B

R:		TSTB (r3)			# Wait for READY flag to raise.
		BPL R

		MOVB (r1), (r2)		# Move key buffer contents to print buffer.
		BR ECHO
		HALT
TTYKS:	.WORD 0777560
TTYKB:	.WORD 0777562
TTYPS:	.WORD 0777564
TTYPB:	.WORD 0777566
BUSY:	.WORD 04000
.end
