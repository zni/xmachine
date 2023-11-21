; Write data located at label DAT to sector 0 track 0 of the disk.

		MOV RXCS, R0		; Setup pointers.
		MOV RXDB, R1
		MOV TTYPB, R4

		MOV FCMD, (R0)		; Issue FILL command.

		MOV #DAT, R2		; Setup pointer to data.
		MOV #0, R3
H3:		BIT DONE, (R0)
		BNE W
		TSTB (R0)			; Wait for TR flag.
		BPL H3

		MOVB (R2)+, (R1)	; Move data to RXDB.
		INC R3
		CMP #200, R3		; Check if we've written 128 bytes.
		BEQ W				; If we have, write it to disk.

		MOV DOT, (R4)

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
WCMD:	.WORD 5				; Write command
FCMD:	.WORD 1				; Fill command
DONE:	.WORD 40			; DONE flag
SEC:	.WORD 0				; Sector to write to.
TRK:    .WORD 0				; Track to write to.
DOT:	.WORD 52


; Begin data to write to the boot sector of the disk.
DAT: 	MOV TTYKS, R0
		MOV TTYKB, R1
		MOV TTYPB, R2
		MOV TTYPS, R3
ECHO:	INC (R0)			; Set RDRENB flag

B:		TSTB (R0)			; Wait for DONE flag to raise.
		BPL B

R:		TSTB (R3)			; Wait for READY flag to raise.
		BPL R

		MOVB (R1), (R2)		; Move key buffer contents to print buffer.
		BR ECHO
		HALT

TTYKS:	.WORD 777560
TTYKB:	.WORD 777562
TTYPS:	.WORD 777564
TTYPB:	.WORD 777566
BUSY:	.WORD 4000
END:	.WORD 0