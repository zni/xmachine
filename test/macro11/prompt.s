; Simple prompt print and echo program.
;
		MOV #400, SP		; Setup stack pointer.
		MOV TTYKS, R0		; Setup input/output pointers.
		MOV TTYKB, R1
		MOV TTYPB, R2
		MOV TTYPS, R3

BEGIN:	MOVB #12, INP		; Clear last character input.
		JSR PC, #PRMPT		; Display prompt.

ECHO:	BIT BUSY, (R0)		; Check if the keyboard is busy.
		BNE ECHO

		INC (R0)			; Flag RDRENB.

BP:		BIT BUSY, (R0)		; Check if we're busy.
		BNE BP

		MOVB (R1), INP		; Move input to last character input.
		CMPB INP, #12		; Is last character a newline?
		BEQ BEGIN			; Yes, branch to beginning.

R$:		TSTB (R3)			; Are we ready to print?
		BPL R$

		MOVB INP, (R2)		; Echo character to screen.
		BR ECHO				; Continue echoing.
		HALT



;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; PROMPT - display prompt
PRMPT:	MOV #PSTR, R4		; Setup prompt string pointer.

PR:		TSTB (R3)			; Wait for READY flag.
		BPL PR

		MOVB #12, (R2)		; Display newline.

PR1:	TSTB (R3)
		BPL PR1

L1:		MOVB (R4)+, (R2)	; Incrementally display prompt.
		BNE PRR				; Was last character \0?

PR2:	TSTB (R3)
		BPL PR2

		BR L1
PRR:	RTS PC				; Return to caller
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

TTYKS:	.WORD 777560
TTYKB:	.WORD 777562
TTYPS:	.WORD 777564
TTYPB:	.WORD 777566
INP:	.WORD 0
BUSY:	.WORD 4000
PSTR:	.ASCIZ /% /
