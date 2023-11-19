; Simple prompt print and echo program.
;
		MOV #400, SP		; Setup stack pointer.
		MOV TTYKS, R0		; Setup input/output pointers.
		MOV TTYKB, R1
		MOV TTYPB, R2
BEGIN:	MOVB #0, INP		; Clear last character input.
		JSR PC, #PRMPT		; Display prompt.
ECHO:	INC (R0)			; Signal KB to receive input.
		MOVB (R1), INP		; Move input to last character input.
		CMPB INP, #12		; Is last character a newline?
		BEQ BEGIN			; Yes, branch to beginning.
		MOVB INP, (R2)		; No, echo character to screen.
		BR ECHO				; Continue echoing.
		HALT

PRMPT:	MOV #PSTR, R4		; Setup prompt string pointer.
		MOVB #12, (R2)		; Display newline.
L1:		MOVB (R4)+, (R2)	; Incrementally display prompt.
		BNE L1				; Was last character \0?
		RTS PC				; Return to caller


TTYKS:	.WORD 777560
TTYKB:	.WORD 777562
TTYPS:	.WORD 777564
TTYPB:	.WORD 777566
PSTR:	.ASCIZ /% /
.EVEN
INP:	.WORD 0
KNL:	.WORD 0