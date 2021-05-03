	.intel_syntax noprefix
	.text
	.globl	call_on_new_stack
	.type	call_on_new_stack, @function



#
# Paremeter passing order for registers:
#  rdi,rsi,rdx, rcx, r8 and r9. Only the 7th argument and onwards are passed on the stack.
#

# void* call_on_new_stack(void* (*fun_ptr)(void*), int64 * stack_bottom, void* param)

call_on_new_stack:
.LFB0:

	.cfi_startproc
	endbr64
	push	rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	mov	rbp, rsp
	.cfi_def_cfa_register 6
	sub	rsp, 48
	# Keeping this here for educational purpose.
	# Store the variables passed as registers to 
	# their location on the stack
	mov	QWORD PTR -24[rbp], rdi # func_ptr
	mov	QWORD PTR -32[rbp], rsi # stack_bottom
	mov	QWORD PTR -40[rbp], rdx # param
	

	# Store current rsp on new stack
	mov	rax, QWORD PTR -32[rbp] # stack_bottom
	mov	QWORD PTR [rax], rsp    # *stack_bottom = RSP

	# Put the parameters to pass to the function
	# in the correct registers
	mov	rax, QWORD PTR -40[rbp] # param
	mov	rdx, QWORD PTR -24[rbp] # func_ptr
	mov	rdi, rax                # param

	# Set stack pointer to new stack
	mov	rsp, QWORD PTR -32[rbp] 

	call	rdx

	# By now the rsp should be pointing at the bottom of our custom stack
	# lets pop rsp 
	pop rsp # Should return the rsp to what it was prior to "Set stack pointer to new stack

	mov	QWORD PTR -8[rbp], rax # return-value from function


	mov	eax, 0 # Return value from this function

	# LEAVE is same as:
    #
    #   mov rsp, rbp
    #   pop rbp

	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE0:
	.size	call_on_new_stack, .-call_on_new_stack
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
