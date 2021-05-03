	.intel_syntax noprefix
	.text
	.globl	__switch
	.type	__switch, @function
__switch:
.LFB0:
	.cfi_startproc
	endbr64
	push	rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	mov	rbp, rsp
	.cfi_def_cfa_register 6
	sub	rsp, 16
	
	mov	QWORD PTR -8[rbp], rdi    # storeSpHere
	mov	QWORD PTR -16[rbp], rsi   # useThisSp

	#
	# PUSH ALL STATES
	#
	push rax
	push rbx
	push rcx
	push rdx
	push rbp
	# rsp - register stack pointer (current location in stack, growing downwards)
	push rsi
	push rdi
	push r8
	push r9
	push r10
	push r11
	push r12
	push r13
	push r14
	push r15

	mov	QWORD PTR [rdi], rsp      # Write the current stack pointer to *storeSpHere

	mov	rsp, rsi   # Load the new SP


	#
	# POP ALL STATES
	#

	pop r15
	pop r14
	pop r13
	pop r12
	pop r11
	pop r10
	pop r9
	pop r8
	pop rdi
	pop rsi
	# rsp
	pop rbp
	pop rdx
	pop rcx
	pop rbx
	pop rax


	# leave
	mov rsp, rbp 
	pop rbp
	
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE0:
	.size	__switch, .-__switch
	.ident	"GCC: (Ubuntu 9.3.0-17ubuntu1~20.04) 9.3.0"
	.section	.note.GNU-stack,"",@progbits
	.section	.note.gnu.property,"a"
	.align 8
	.long	 1f - 0f
	.long	 4f - 1f
	.long	 5
0:
	.string	 "GNU"
1:
	.align 8
	.long	 0xc0000002
	.long	 3f - 2f
2:
	.long	 0x3
3:
	.align 8
4:
