; Simple prompt print and echo program.
;
		MOV #700, SP		; Setup stack pointer.
		MOV TTYKS, R0		; Setup input/output pointers.
		MOV TTYKB, R1
		MOV TTYPB, R2
		MOV TTYPS, R3

		JSR PC, DIR

BEGIN: 	MOV #700, SP
		MOV SP, R5
		JSR PC, PRMPT		; Display prompt.

ECHO:	BIT BUSY, (R0)		; Check if the keyboard is busy.
		BNE ECHO

		INC (R0)			; Flag RDRENB.

BP:		TSTB (R0)			; Check if we're done reading.
		BPL BP

		MOVB (R1), -(SP)	; Move input to last character input.
		CMPB (SP), #12		; Is last character a newline?
		BEQ CHKQ		    ; Yes, branch to beginning.
		BR R$

CHKQ:	MOV SP, R5
		CMP (SP)+, #12
		BNE BEGIN
		CMP (SP)+, #'Q
		BNE CHKP
		BEQ H

CHKP:	MOV R5, SP
		CMP (SP)+, #12
		BNE BEGIN
		CMP (SP)+, #'P
		BNE BEGIN
		JSR PC, UNAME
		BR BEGIN


R$:		TSTB (R3)			; Are we ready to print?
		BPL R$

		MOVB (SP), (R2)		; Echo character to screen.
		BR ECHO				; Continue echoing.

H:		HALT



;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; PROMPT - display prompt
PRMPT:	TSTB (R3)			; Wait for READY flag.
		BPL PRMPT

		MOVB #12, (R2)		; Display newline.
		MOVB #45, (R2)		; Display %.
		MOVB #40, (R2)		; Display space.

PRR:	RTS PC				; Return to caller
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; UNAME - display machine name
UNAME:	TSTB (R3)			; Wait for READY flag.
		BPL UNAME

		MOVB #12, (R2)		; Display newline.
		MOV #'x, (R2)
		MOV #'m, (R2)
		MOV #'a, (R2)
		MOV #'c, (R2)
		MOV #'h, (R2)
		MOV #'i, (R2)
		MOV #'n, (R2)
		MOV #'e, (R2)
		MOV #40, (R2)
		MOV #'v, (R2)
		MOV #'0, (R2)
		MOV #'., (R2)
		MOV #'1, (R2)

		RTS PC				; Return to caller
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; DIR - print directions
DIR:	TSTB (R3)
		BPL DIR

		MOV #12, (R2)
		MOV #'E, (R2)
		MOV #'n, (R2)
		MOV #'t, (R2)
		MOV #'e, (R2)
		MOV #'r, (R2)
		MOV #40, (R2)
		MOV #'Q, (R2)
		MOV #40, (R2)
		MOV #'t, (R2)
		MOV #'o, (R2)
		MOV #40, (R2)
		MOV #'H, (R2)
		MOV #'A, (R2)
		MOV #'L, (R2)
		MOV #'T, (R2)

		RTS PC
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

TTYKS:	.WORD 777560
TTYKB:	.WORD 777562
TTYPS:	.WORD 777564
TTYPB:	.WORD 777566
BUSY:	.WORD 4000
