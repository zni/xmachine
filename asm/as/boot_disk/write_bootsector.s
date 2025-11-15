# Write data located at label DAT to sector 0 track 0 of the disk.

		MOV RXCS, r0		# Setup pointers.
		MOV RXDB, r1
		MOV TTYPB, r4

		MOV FCMD, (r0)		# Issue FILL command.

		MOV $DAT, r2		# Setup pointer to data.
		MOV $0, r3
H3:		BIT DONE, (r0)
		BNE W
		TSTB (r0)			# Wait for TR flag.
		BPL H3

		MOVB (r2)+, (r1)	# Move data to RXDB.
		INC r3
		CMP $0200, r3		# Check if we've written 128 bytes.
		BEQ W				# If we have, write it to disk.

		MOV DOT, (r4)

		BR H3				# Else start all over again.

W:		MOV WCMD, (r0)		# Issue WRITE command.

H1:		TSTB (r0)			# Wait for TR flag.
		BPL H1

		MOVB SEC, (r1)		# Write sector.

H2:		TSTB (r0)			# Wait for TR flag.
		BPL H2

		MOVB TRK, (r1)		# Write track.

D:		BIT DONE, (r0)		# Wait for DONE flag.
		BEQ D

		HALT
RXCS:	.WORD 0177170		# RXCS address
RXDB:	.WORD 0177172		# RXDB address
WCMD:	.WORD 5				# Write command
FCMD:	.WORD 1				# Fill command
DONE:	.WORD 040			# DONE flag
SEC:	.WORD 0				# Sector to write to.
TRK:    .WORD 0				# Track to write to.
DOT:	.WORD 052


# Begin data to write to the boot sector of the disk.
DAT: 	MOV TTYKS, r0
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
END:	.WORD 0

