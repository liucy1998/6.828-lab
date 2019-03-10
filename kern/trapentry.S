/* See COPYRIGHT for copyright information. */

#include <inc/mmu.h>
#include <inc/memlayout.h>
#include <inc/trap.h>

#include <kern/picirq.h>


###################################################################
# exceptions/interrupts
###################################################################

/* TRAPHANDLER defines a globally-visible function for handling a trap.
 * It pushes a trap number onto the stack, then jumps to _alltraps.
 * Use TRAPHANDLER for traps where the CPU automatically pushes an error code.
 *
 * You shouldn't call a TRAPHANDLER function from C, but you may
 * need to _declare_ one in C (for instance, to get a function pointer
 * during IDT setup).  You can declare the function with
 *   void NAME();
 * where NAME is the argument passed to TRAPHANDLER.
 */
#define TRAPHANDLER(name, num, dpl)						\
	.data;  									\
	.long num;								\
	.long name;								\
	.long dpl;								\
	.text;											\
	.globl name;		/* define global symbol for 'name' */	\
	.type name, @function;	/* symbol type is function */		\
	.align 2;		/* align function definition */		\
	name:			/* function starts here */		\
	pushl $(num);							\
	jmp _alltraps

/* Use TRAPHANDLER_NOEC for traps where the CPU doesn't push an error code.
 * It pushes a 0 in place of the error code, so the trap frame has the same
 * format in either case.
 */
#define TRAPHANDLER_NOEC(name, num, dpl)					\
	.data;  									\
	.long num;								\
	.long name;								\
	.long dpl;								\
	.text;									\
	.globl name;							\
	.type name, @function;						\
	.align 2;							\
	name:								\
	pushl $0;							\
	pushl $(num);							\
	jmp _alltraps						


/*
 * Lab 3: Your code here for generating entry points for the different traps.
 */
.data
	.globl		trap_info
	.p2align 		2
trap_info:
	TRAPHANDLER_NOEC(divide_error_handler, T_DIVIDE, 0)	
	TRAPHANDLER_NOEC(debug_exception_handler, T_DEBUG, 3)
	TRAPHANDLER_NOEC(non_maskable_interrupt_handler, T_NMI, 0)
	TRAPHANDLER_NOEC(breakpoint_handler, T_BRKPT, 3)
	TRAPHANDLER_NOEC(overflow_handler, T_OFLOW, 0)
	TRAPHANDLER_NOEC(bounds_check_handler, T_BOUND, 0)
	TRAPHANDLER_NOEC(invalid_opcode_handler, T_ILLOP, 0)
	TRAPHANDLER_NOEC(device_not_available_handler, T_DEVICE, 0)
	TRAPHANDLER(double_fault_handler, T_DBLFLT, 0)
	TRAPHANDLER(invalid_tss_handler, T_TSS, 0)
	TRAPHANDLER(segment_not_present_handler, T_SEGNP, 0)
	TRAPHANDLER(stack_exception_handler, T_STACK, 0)
	TRAPHANDLER(general_protection_fault_handler, T_GPFLT, 0)
	TRAPHANDLER(pagefault_handler, T_PGFLT, 0)
	TRAPHANDLER_NOEC(floating_point_error_handler, T_FPERR, 0)
	TRAPHANDLER(alignment_check_handler, T_ALIGN, 0)
	TRAPHANDLER_NOEC(machine_check_handler, T_MCHK, 0)
	TRAPHANDLER_NOEC(simd_floating_point_error_handler, T_SIMDERR, 0)
	TRAPHANDLER_NOEC(syscall_handler, T_SYSCALL, 3)
/*
 * Lab 3: Your code here for _alltraps
 */

_alltraps:
	// Compelete trapframe
	pushl %ds 
	pushl %es
	pushal
	
	push %ax 
	mov $GD_KD, %ax
	mov %ax, %ds
	mov %ax, %es
	pop %ax

	pushl %esp

	call trap
	