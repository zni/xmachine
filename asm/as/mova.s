# Test out absolute addressing.
.text
	MOV $012,	r0
	MOV r0,		r1
	MOV r1,		r2
	MOV r2,		040
.end
