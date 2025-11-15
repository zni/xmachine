		MOV RXCS, r0		# Move address of disk status register into R0
		MOV RXDB, r1		# Move address of disk buffer register into R1
		MOV $OUT, r2		# Move the address of OUT into R2
		MOV CMD, (r0)		# Set the GO bit with command of empty buffer (001) to RXCS
TR:		BIT DONE, (r0)		# Test if the DONE flag is set.
		BNE H				# Branch to HALT if it is.
		TSTB (r0)			# Test if the Transfer Ready bit is set.
		BPL TR				# Loop around again if it's not.
		MOVB (r1), (r2)+	# Move the contents of the disk buffer
							# to address pointed to by R2.
		BIT DONE, (r0)		# Test if the DONE flag is set.
		BEQ TR				# Loop back around if it's not.
H:		HALT
RXCS:	.WORD 0177170
RXDB:	.WORD 0177172
CMD:	.WORD 3
DONE:	.WORD 040
OUT:	.WORD 0
