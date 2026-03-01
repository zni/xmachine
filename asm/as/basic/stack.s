		MOV $0400, sp
		MOV $4, -(sp)
		MOV $5, -(sp)
		JSR pc, $ADDSP
		HALT

ADDSP:	MOV 2(sp), r0
		MOV 4(sp), r1
		ADD r0, r1
		RTS pc
