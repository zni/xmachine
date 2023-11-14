; Execute a disk FILL BUFFER and then WRITE SECTOR.

		MOV RXCS, R0		; Setup pointers.
		MOV RXDB, R1

		MOV FCMD, (R0)		; Issue FILL command.

P:		MOV #DAT, R2		; Setup pointer to data.
H3:		TSTB (R0)			; Wait for TR flag.
		BPL H3

		MOVB (R2)+, (R1)	; Move data to RXDB, reset pointer if we move the null byte.
		BEQ P

		BIT DONE, (R0)		; Are we done?
		BNE W				; Then branch to sending WRITE.
		BR H3				; Else start all over again.

W:		MOV WCMD, (R0)		; Issue WRITE command.

H1:		TSTB (R0)			; Wait for TR flag.
		BPL H1

		MOVB SEC, (R1)		; Write sector.

H2:		TSTB (R0)			; Wait for TR flag.
		BPL H2

		MOVB TRK, (R1)		; Write track.

D:		BIT DONE, (R0)		; Wait for DONE flag.
		BEQ D

		HALT
RXCS:	.WORD 177170		; RXCS address
RXDB:	.WORD 177172		; RXDB address
RCMD:	.WORD 7				; Read command
WCMD:	.WORD 5				; Write command
FCMD:	.WORD 1				; Fill command
DONE:	.WORD 40			; DONE flag
SEC:	.WORD 5				; Sector to write to.
TRK:    .WORD 7				; Track to write to.
DAT:	.ASCIZ /Yellow Hello/