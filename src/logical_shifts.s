	AREA    SHIFTS, CODE, READONLY
	PRESERVE8
	EXPORT lsl
	EXPORT lsr

lsl			;logical shift left
	mov r0, r0, LSL r1
	mov PC, LR
	
lsr			;logical shift right
	mov r0, r0, LSR r1		; realiza el desplazamiento
	mov PC, LR				; retorno
	
	END