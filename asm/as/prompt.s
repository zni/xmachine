# Simple prompt print and echo program.
#
		MOV $0700, sp		# Setup stack pointer.
		MOV TTYKS, r0		# Setup input/output pointers.
		MOV TTYKB, r1
		MOV TTYPB, r2
		MOV TTYPS, r3

		JSR pc, DIR

BEGIN: 	MOV $0700, sp
		MOV sp, r5
		JSR pc, PRMPT		# Display prompt.

ECHO:	BIT BUSY, (r0)		# Check if the keyboard is busy.
		BNE ECHO

		INC (r0)			# Flag RDRENB.

BP:		TSTB (r0)			# Check if we're done reading.
		BPL BP

		MOVB (r1), -(sp)	# Move input to last character input.
		CMPB (sp), $012		# Is last character a newline?
		BEQ CHKQ		    # Yes, branch to beginning.
		BR RP

CHKQ:	MOV sp, r5
		CMP (sp)+, $012
		BNE BEGIN
		CMP (sp)+, $'Q
		BNE CHKP
		BEQ H

CHKP:	MOV r5, sp
		CMP (sp)+, $012
		BNE BEGIN
		CMP (sp)+, $'P
		BNE BEGIN
		JSR pc, UNAME
		BR BEGIN


RP:		TSTB (r3)			# Are we ready to print?
		BPL RP

		MOVB (sp), (r2)		# Echo character to screen.
		BR ECHO				# Continue echoing.

H:		HALT



#;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
# PROMPT - display prompt
PRMPT:	TSTB (r3)			# Wait for READY flag.
		BPL PRMPT

		MOVB $012, (r2)		# Display newline.
		MOVB $045, (r2)		# Display %.
		MOVB $040, (r2)		# Display space.

PRR:	RTS pc				# Return to caller
#;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

#;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
# UNAME - display machine name
UNAME:	TSTB (r3)			# Wait for READY flag.
		BPL UNAME

		MOVB $012, (r2)		# Display newline.
		MOV $'x, (r2)
		MOV $'m, (r2)
		MOV $'a, (r2)
		MOV $'c, (r2)
		MOV $'h, (r2)
		MOV $'i, (r2)
		MOV $'n, (r2)
		MOV $'e, (r2)
		MOV $40, (r2)
		MOV $'v, (r2)
		MOV $'0, (r2)
		MOV $'., (r2)
		MOV $'1, (r2)

		RTS pc				# Return to caller
#;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

#;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
# DIR - print directions
DIR:	TSTB (r3)
		BPL DIR

		MOV $12, (r2)
		MOV $'E, (r2)
		MOV $'n, (r2)
		MOV $'t, (r2)
		MOV $'e, (r2)
		MOV $'r, (r2)
		MOV $40, (r2)
		MOV $'Q, (r2)
		MOV $40, (r2)
		MOV $'t, (r2)
		MOV $'o, (r2)
		MOV $40, (r2)
		MOV $'H, (r2)
		MOV $'A, (r2)
		MOV $'L, (r2)
		MOV $'T, (r2)

		RTS pc
#;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

TTYKS:	.WORD 0777560
TTYKB:	.WORD 0777562
TTYPS:	.WORD 0777564
TTYPB:	.WORD 0777566
BUSY:	.WORD 04000
