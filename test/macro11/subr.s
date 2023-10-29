; Test out subroutines.

		MOV #400, SP		; Setup SP
		MOV #4, R0			; Initialize R0
		JSR PC, #ADD4
		JSR PC, #ADD6
		JSR PC, #SUB3
		HALT


ADD4:	ADD #4, R0
		RTS PC

ADD6:	ADD #6, R0
		RTS PC

SUB3:	MOV #3, R1
		SUB R1, R0
		RTS PC