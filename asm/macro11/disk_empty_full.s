		MOV RXCS, R0		; Move address of disk status register into R0
		MOV RXDB, R1		; Move address of disk buffer register into R1
		MOV #OUT, R2		; Move the address of OUT into R2
		MOV ECMD, (R0)		; Set the GO bit with command of empty buffer (001) to RXCS
TR:		BIT DONE, (R0)		; Test if the DONE flag is set.
		BNE H				; Branch to HALT if it is.
		TSTB (R0)			; Test if the Transfer Ready bit is set.
		BPL TR				; Loop around again if it's not.
		MOVB (R1), (R2)+	; Move the contents of the disk buffer
							; to address pointed to by R2.
		BIT DONE, (R0)		; Test if the DONE flag is set.
		BEQ TR				; Loop back around if it's not.

H:		MOV #OUT, R2

		MOV FCMD, (R0)

HOLD:	TSTB (R0)
		BPL HOLD

TRS:	BIT DONE, (R0)
		BNE D
		TSTB (R0)
		BPL TRS
		MOVB (R2)+, (R1)
		BIT DONE, (R0)
		BEQ TRS
D:		HALT
RXCS:	.WORD 177170
RXDB:	.WORD 177172
ECMD:	.WORD 3
FCMD:	.WORD 1
DONE:	.WORD 40
OUT:	.WORD 0