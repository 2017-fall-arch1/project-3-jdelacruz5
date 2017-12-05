	.arch msp430g2553
	.p2align 1,0
	.text
	
	.data
num:	.word 0 		; state

	.text
buzz:	.word one		; case 1
        .word two 		; case 2
        .word three     ; case 3



	.global Bstate
Bstate:
	mov r12, &num		; passed state param in r12
	cmp #4, &num
	jnc default		

	;; move to correct state
	mov &num, r12
	add r12, r12 		
	mov jt(r12), r0		; jump buzz, 1,2, or 3

	;; switch cases
one:
	mov #1000, r12		; param is 200
	call #buzzer_set_period
	jmp end
two:
	mov #5000, r12		; param is 12000
	call #buzzer_set_period
	jmp end
three:
	mov #4000, r12		; param is 12000
	call #buzzer_set_period
	jmp end
default:
end:
	pop r0			; return 
