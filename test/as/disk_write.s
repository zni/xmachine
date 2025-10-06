# Execute a disk FILL BUFFER and then WRITE SECTOR.

		MOV RXCS, r0		# Setup pointers.
		MOV RXDB, r1

		MOV FCMD, (r0)		# Issue FILL command.

P:		MOV $DAT, r2		# Setup pointer to data.
H3:		BIT DONE, (r0)
		BNE W
		TSTB (r0)			# Wait for TR flag.
		BPL H3

		MOVB (r2)+, (r1)	# Move data to RXDB, reset pointer if we move the null byte.
		BEQ P

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
RCMD:	.WORD 7				# Read command
WCMD:	.WORD 5				# Write command
FCMD:	.WORD 1				# Fill command
DONE:	.WORD 040			# DONE flag
SEC:	.WORD 5				# Sector to write to.
TRK:    .WORD 7				# Track to write to.
DAT:	.ASCIZ /Yellow Hello/

