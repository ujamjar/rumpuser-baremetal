#include <bmk/kernel.h>

.equ NO_IRQ, 0x80                 // mask to disable IRQ 
.equ NO_FIQ, 0x40                 // mask to disable FIQ 
.equ NO_INT, (NO_IRQ | NO_FIQ)    // mask to disable IRQ and FIQ 
.equ FIQ_MODE, 0x11
.equ IRQ_MODE, 0x12
.equ SYS_MODE, 0x1F 
.equ SUPER_MODE, 0x13

// XXX
// This is the bootstack from i386 used for multiboot.
// I'm not sure we want this - the stack should probably be
// at the end of RAM instead.  For now it will do.
.section bootstack
.set stack_size, 65536
.globl _bootstack
.comm _bootstack, stack_size

ENTRY(_start)
loop:
  msr cpsr_c,#(SYS_MODE | NO_INT)   // straight into system mode
  ldr sp,=(_bootstack + stack_size) // set stack
  blx bmk_cpu_vector_table          // init the vector table
  msr cpsr_c,#(SYS_MODE)            // turn on interrupts 
  blx bmk_init                      // nothing at the moment - could do interrupt init instead?
  blx bmk_main
	b loop
END(_start)

ENTRY(bmk_cpu_isr)
  mov r13,r0                      // save r0 in r13_IRQ 
  sub r0,lr,#4                    // put return address in r0_SYS 
  mov lr,r1                       // save r1 in r14_IRQ (lr) 
  mrs r1,spsr                     // put the SPSR in r1_SYS 
  msr cpsr_c,#(SYS_MODE | NO_IRQ) // SYSTEM mode, no IRQ/FIQ enabled! 
  stmfd sp!,{r0,r1}               // save SPSR and PC on SYS stack 
  stmfd sp!,{r2-r3,r12,lr}        // save AAPCS-clobbered regs on SYS stack 
  mov r0,sp                       // make the sp_SYS visible to IRQ mode 
  sub sp,sp,#(2*4)                // make room for stacking (r0_SYS, r1_SYS)  
  msr cpsr_c,#(IRQ_MODE | NO_IRQ) // IRQ mode, IRQ/FIQ disabled 
  stmfd r0!,{r13,r14}             // finish saving the context (r0_SYS,r1_SYS)
  msr cpsr_c,#(SYS_MODE | NO_IRQ) // SYSTEM mode, IRQ disabled 

  // NOTE: bmk_isr might re-enable IRQ interrupts (the FIQ is enabled
  // already), if IRQs are prioritized by an interrupt controller.

  ldr r12,=bmk_isr
  mov lr,pc                       // copy the return address to link register 
  bx r12                          // call the C IRQ-handler (ARM/THUMB) 

  msr cpsr_c,#(SYS_MODE | NO_INT) // SYSTEM mode, IRQ/FIQ disabled 
  mov r0,sp                       // make sp_SYS visible to IRQ mode 
  add sp,sp,#(8*4)                // fake unstacking 8 registers from sp_SYS 
  msr cpsr_c,#(IRQ_MODE | NO_INT) // IRQ mode, both IRQ/FIQ disabled 
  mov sp,r0                       // copy sp_SYS to sp_IRQ 
  ldr r0,[sp,#(7*4)]              // load the saved SPSR from the stack 
  msr spsr_cxsf,r0                // copy it into spsr_IRQ 
  ldmfd sp,{r0-r3,r12,lr}^        // unstack all saved USER/SYSTEM registers 
  nop                             // can't access banked reg immediately 
  ldr lr,[sp,#(6*4)]              // load return address from the SYS stack 
  movs pc,lr                      // return restoring CPSR from SPSR 
END(bmk_cpu_isr)

